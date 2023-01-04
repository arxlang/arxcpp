
// note: arrow will not be used yet
// #include <arrow/api.h>
// #include <arrow/csv/api.h>
// #include <arrow/io/api.h>
// #include <arrow/ipc/api.h>
// #include <arrow/pretty_print.h>
// #include <arrow/result.h>
// #include <arrow/status.h>
// #include <arrow/table.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <glog/logging.h>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Analysis/BasicAliasAnalysis.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/Scalar.h>

#include "codegen/ast-to-object.h"
#include "error.h"
#include "io.h"
#include "jit.h"
#include "lexer.h"
#include "parser.h"

extern std::string INPUT_FILE;
extern std::string OUTPUT_FILE;
extern std::string ARX_VERSION;

class ASTToObjectVisitor : public Visitor {
 public:
  llvm::Value* result_val;
  llvm::Function* result_func;

  ~ASTToObjectVisitor() {
    this->result_val = nullptr;
    this->result_func = nullptr;
  }

  virtual void visit(NumberExprAST*) override;
  virtual void visit(VariableExprAST*) override;
  virtual void visit(UnaryExprAST*) override;
  virtual void visit(BinaryExprAST*) override;
  virtual void visit(CallExprAST*) override;
  virtual void visit(IfExprAST*) override;
  virtual void visit(ForExprAST*) override;
  virtual void visit(VarExprAST*) override;
  virtual void visit(PrototypeAST*) override;
  virtual void visit(FunctionAST*) override;
  virtual void clean() override;
};

/**
 * ===----------------------------------------------------------------------===
 *  Code Generation Globals
 * ===----------------------------------------------------------------------===
 */

std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<llvm::IRBuilder<>> Builder;
llvm::ExitOnError ExitOnErr;

std::map<std::string, llvm::AllocaInst*> NamedValues;
std::unique_ptr<llvm::orc::ArxJIT> TheJIT;
std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

auto codegen = new ASTToObjectVisitor();

/**
 * @brief
 * @param Name
 * @return
 *
 * First, see if the function has already been added to the current
 * module. If not, check whether we can codegen the declaration from some
 * existing prototype. If no existing prototype exists, return null.
 */
static auto getFunction(std::string Name) -> void {
  if (auto* F = TheModule->getFunction(Name)) {
    codegen->result_func = F;
    return;
  }

  auto FI = FunctionProtos.find(Name);
  if (FI != FunctionProtos.end()) {
    FI->second->accept(codegen);
  };
}

/**
 * @brief
 * @param TheFunction
 * @param VarName
 * @return
 *
 * CreateEntryBlockAlloca - Create an alloca instruction in the entry
 * block of the function.  This is used for mutable variables etc.
 */
static auto CreateEntryBlockAlloca(
  llvm::Function* TheFunction, llvm::StringRef VarName) -> llvm::AllocaInst* {
  llvm::IRBuilder<> TmpB(
    &TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(
    llvm::Type::getDoubleTy(*TheContext), nullptr, VarName);
}

auto ASTToObjectVisitor::clean() -> void {
  this->result_val = nullptr;
  this->result_func = nullptr;
}

/**
 * @brief
 * @return
 *
 */
auto ASTToObjectVisitor::visit(NumberExprAST* expr) -> void {
  codegen->result_val =
    llvm::ConstantFP::get(*TheContext, llvm::APFloat(expr->Val));
}

/**
 * @brief Stat a variable in the function.
 * @return The variable loaded into the llvm.
 *
 */
auto ASTToObjectVisitor::visit(VariableExprAST* expr) -> void {
  llvm::Value* V = NamedValues[expr->Name];

  if (!V) {
    codegen->result_val = LogErrorV("Unknown variable name");
    return;
  }

  codegen->result_val = Builder->CreateLoad(
    llvm::Type::getDoubleTy(*TheContext), V, expr->Name.c_str());
}

/**
 * @brief
 * @return
 *
 */
auto ASTToObjectVisitor::visit(UnaryExprAST* expr) -> void {
  expr->Operand.get()->accept(codegen);
  llvm::Value* OperandV = codegen->result_val;

  if (!OperandV) {
    codegen->result_val = nullptr;
    return;
  }

  getFunction(std::string("unary") + expr->Opcode);
  llvm::Function* F = codegen->result_func;
  if (!F) {
    codegen->result_val = LogErrorV("Unknown unary operator");
    return;
  }

  codegen->result_val = Builder->CreateCall(F, OperandV, "unop");
}

/**
 * @brief
 * @return
 *
 */
auto ASTToObjectVisitor::visit(BinaryExprAST* expr) -> void {
  //  Special case '=' because we don't want to emit the LHS as an
  // expression.*/
  if (expr->Op == '=') {
    // Assignment requires the LHS to be an identifier.
    // This assume we're building without RTTI because LLVM builds that
    // way by default.  If you build LLVM with RTTI this can be changed
    // to a dynamic_cast for automatic error checking.
    VariableExprAST* LHSE = static_cast<VariableExprAST*>(expr->LHS.get());
    if (!LHSE) {
      codegen->result_val = LogErrorV("destination of '=' must be a variable");
      return;
    }

    // Codegen the RHS.//
    expr->RHS.get()->accept(codegen);
    llvm::Value* Val = codegen->result_val;

    if (!Val) {
      codegen->result_val = nullptr;
      return;
    };

    // Look up the name.//
    llvm::Value* Variable = NamedValues[LHSE->getName()];
    if (!Variable) {
      codegen->result_val = LogErrorV("Unknown variable name");
      return;
    }

    Builder->CreateStore(Val, Variable);
    codegen->result_val = Val;
  }

  expr->LHS.get()->accept(codegen);
  llvm::Value* L = codegen->result_val;
  expr->RHS.get()->accept(codegen);
  llvm::Value* R = codegen->result_val;

  if (!L || !R) {
    codegen->result_val = nullptr;
    return;
  }

  switch (expr->Op) {
    case '+':
      codegen->result_val = Builder->CreateFAdd(L, R, "addtmp");
      return;
    case '-':
      codegen->result_val = Builder->CreateFSub(L, R, "subtmp");
      return;
    case '*':
      codegen->result_val = Builder->CreateFMul(L, R, "multmp");
      return;
    case '<':
      L = Builder->CreateFCmpULT(L, R, "cmptmp");
      // Convert bool 0/1 to double 0.0 or 1.0 //
      codegen->result_val = Builder->CreateUIToFP(
        L, llvm::Type::getDoubleTy(*TheContext), "booltmp");
      return;
  }

  // If it wasn't a builtin binary operator, it must be a user defined
  // one. Emit a call to it.
  getFunction(std::string("binary") + expr->Op);
  llvm::Function* F = codegen->result_func;
  assert(F && "binary operator not found!");

  llvm::Value* Ops[] = {L, R};
  codegen->result_val = Builder->CreateCall(F, Ops, "binop");
}

/**
 * @brief Look up the name in the global module table.
 * @return
 *
 */
auto ASTToObjectVisitor::visit(CallExprAST* expr) -> void {
  getFunction(expr->Callee);
  llvm::Function* CalleeF = codegen->result_func;
  if (!CalleeF) {
    codegen->result_val = LogErrorV("Unknown function referenced");
    return;
  }

  if (CalleeF->arg_size() != expr->Args.size()) {
    codegen->result_val = LogErrorV("Incorrect # arguments passed");
    return;
  }

  std::vector<llvm::Value*> ArgsV;
  for (unsigned i = 0, e = expr->Args.size(); i != e; ++i) {
    expr->Args[i].get()->accept(codegen);
    llvm::Value* ArgsV_item = codegen->result_val;
    ArgsV.push_back(ArgsV_item);
    if (!ArgsV.back()) {
      codegen->result_val = nullptr;
      return;
    }
  }

  codegen->result_val = Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

/**
 * @brief
 */
auto ASTToObjectVisitor::visit(IfExprAST* expr) -> void {
  expr->Cond.get()->accept(codegen);
  llvm::Value* CondV = codegen->result_val;

  if (!CondV) {
    codegen->result_val = nullptr;
    return;
  }

  // Convert condition to a bool by comparing non-equal to 0.0.
  CondV = Builder->CreateFCmpONE(
    CondV, llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0)), "ifcond");

  llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block
  // at the end of the function.
  llvm::BasicBlock* ThenBB =
    llvm::BasicBlock::Create(*TheContext, "then", TheFunction);
  llvm::BasicBlock* ElseBB = llvm::BasicBlock::Create(*TheContext, "else");
  llvm::BasicBlock* MergeBB = llvm::BasicBlock::Create(*TheContext, "ifcont");

  Builder->CreateCondBr(CondV, ThenBB, ElseBB);

  // Emit then value.
  Builder->SetInsertPoint(ThenBB);

  expr->Then.get()->accept(codegen);
  llvm::Value* ThenV = codegen->result_val;
  if (!ThenV) {
    codegen->result_val = nullptr;
    return;
  }

  Builder->CreateBr(MergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for
  // the PHI.
  ThenBB = Builder->GetInsertBlock();

  // Emit else block.
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder->SetInsertPoint(ElseBB);

  expr->Else.get()->accept(codegen);
  llvm::Value* ElseV = codegen->result_val;
  if (!ElseV) {
    codegen->result_val = nullptr;
    return;
  }

  Builder->CreateBr(MergeBB);
  // Codegen of 'Else' can change the current block, update ElseBB for
  // the PHI.
  ElseBB = Builder->GetInsertBlock();

  // Emit merge block.
  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder->SetInsertPoint(MergeBB);
  llvm::PHINode* PN =
    Builder->CreatePHI(llvm::Type::getDoubleTy(*TheContext), 2, "iftmp");

  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);

  codegen->result_val = PN;
  return;
}

auto ASTToObjectVisitor::visit(ForExprAST* expr) -> void {
  llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();

  // Create an alloca for the variable in the entry block.
  llvm::AllocaInst* Alloca =
    CreateEntryBlockAlloca(TheFunction, expr->VarName);

  // Emit the start code first, without 'variable' in scope.
  expr->Start.get()->accept(codegen);
  llvm::Value* StartVal = codegen->result_val;
  if (!StartVal) {
    codegen->result_val = nullptr;
    return;
  }

  // Store the value into the alloca.
  Builder->CreateStore(StartVal, Alloca);

  // Make the new basic block for the loop header, inserting after
  // current block.
  llvm::BasicBlock* LoopBB =
    llvm::BasicBlock::Create(*TheContext, "loop", TheFunction);

  // Insert an explicit fall through from the current block to the
  // LoopBB.
  Builder->CreateBr(LoopBB);

  // Start insertion in LoopBB.
  Builder->SetInsertPoint(LoopBB);

  // Within the loop, the variable is defined equal to the PHI node.  If
  // it shadows an existing variable, we have to restore it, so save it
  // now.
  llvm::AllocaInst* OldVal = NamedValues[expr->VarName];
  NamedValues[expr->VarName] = Alloca;

  // Emit the body of the loop.  This, like any other expr, can change
  // the current BB.  Note that we ignore the value computed by the body,
  // but don't allow an error.
  expr->Body.get()->accept(codegen);
  llvm::Value* BodyVal = codegen->result_val;

  if (!BodyVal) {
    codegen->result_val = nullptr;
    return;
  }

  // Emit the step value.
  llvm::Value* StepVal = nullptr;
  if (expr->Step) {
    expr->Step.get()->accept(codegen);
    StepVal = codegen->result_val;
    if (!StepVal) {
      codegen->result_val = nullptr;
      return;
    }
  } else {
    // If not specified, use 1.0.
    StepVal = llvm::ConstantFP::get(*TheContext, llvm::APFloat(1.0));
  }

  // Compute the end condition.
  expr->End.get()->accept(codegen);
  llvm::Value* EndCond = codegen->result_val;
  if (!EndCond) {
    codegen->result_val = nullptr;
    return;
  }

  // Reload, increment, and restore the alloca.  This handles the case
  // where the body of the loop mutates the variable.
  llvm::Value* CurVar = Builder->CreateLoad(
    llvm::Type::getDoubleTy(*TheContext), Alloca, expr->VarName.c_str());
  llvm::Value* NextVar = Builder->CreateFAdd(CurVar, StepVal, "nextvar");
  Builder->CreateStore(NextVar, Alloca);

  // Convert condition to a bool by comparing non-equal to 0.0.
  EndCond = Builder->CreateFCmpONE(
    EndCond,
    llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0)),
    "loopcond");

  // Create the "after loop" block and insert it.
  llvm::BasicBlock* AfterBB =
    llvm::BasicBlock::Create(*TheContext, "afterloop", TheFunction);

  // Insert the conditional branch into the end of LoopEndBB.
  Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

  // Any new code will be inserted in AfterBB.
  Builder->SetInsertPoint(AfterBB);

  // Restore the unshadowed variable.
  if (OldVal) {
    NamedValues[expr->VarName] = OldVal;
  } else {
    NamedValues.erase(expr->VarName);
  }

  // for expr always returns 0.0.
  codegen->result_val =
    llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*TheContext));
}

/**
 * @brief
 * @return Return the body computation.
 *
 */
auto ASTToObjectVisitor::visit(VarExprAST* expr) -> void {
  std::vector<llvm::AllocaInst*> OldBindings;

  llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();

  // Register all variables and emit their initializer.
  for (auto& i : expr->VarNames) {
    const std::string& VarName = i.first;
    ExprAST* Init = i.second.get();

    // Emit the initializer before adding the variable to scope, this
    // prevents the initializer from referencing the variable itself, and
    // permits stuff like this:
    //  var a = 1 in
    //    var a = a in ...   # refers to outer 'a'.

    llvm::Value* InitVal = nullptr;
    if (Init) {
      Init->accept(codegen);
      InitVal = codegen->result_val;
      if (!InitVal) {
        codegen->result_val = nullptr;
        return;
      }
    } else {  // If not specified, use 0.0.
      InitVal = llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0));
    }

    llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
    Builder->CreateStore(InitVal, Alloca);

    // Remember the old variable binding so that we can restore the
    // binding when we unrecurse.
    OldBindings.push_back(NamedValues[VarName]);

    // Remember this binding.
    NamedValues[VarName] = Alloca;
  }

  // Codegen the body, now that all vars are in scope.
  expr->Body.get()->accept(codegen);
  llvm::Value* BodyVal = codegen->result_val;
  if (!BodyVal) {
    codegen->result_val = nullptr;
    return;
  }

  // Pop all our variables from scope.
  for (unsigned i = 0, e = expr->VarNames.size(); i != e; ++i) {
    NamedValues[expr->VarNames[i].first] = OldBindings[i];
  }

  // Return the body computation.
  codegen->result_val = BodyVal;
}

/**
 * @brief
 * @return
 *
 */
auto ASTToObjectVisitor::visit(PrototypeAST* expr) -> void {
  // Make the function type:  double(double,double) etc.
  std::vector<llvm::Type*> Doubles(
    expr->Args.size(), llvm::Type::getDoubleTy(*TheContext));
  llvm::FunctionType* FT = llvm::FunctionType::get(
    llvm::Type::getDoubleTy(*TheContext), Doubles, false);

  llvm::Function* F = llvm::Function::Create(
    FT, llvm::Function::ExternalLinkage, expr->Name, TheModule.get());

  // Set names for all arguments.
  unsigned Idx = 0;
  for (auto& Arg : F->args()) {
    Arg.setName(expr->Args[Idx++]);
  }

  codegen->result_func = F;
}

/**
 * @brief
 * @return
 *
 * Transfer ownership of the prototype to the FunctionProtos map, but
 * keep a reference to it for use below.
 */
auto ASTToObjectVisitor::visit(FunctionAST* expr) -> void {
  auto& P = *(expr->Proto);
  FunctionProtos[expr->Proto->getName()] = std::move(expr->Proto);
  getFunction(P.getName());
  llvm::Function* TheFunction = codegen->result_func;

  if (!TheFunction) {
    codegen->result_func = nullptr;
    return;
  }

  // If this is an operator, install it.
  std::cout << "If this is an operator, install it";
  if (P.isBinaryOp()) {
    Parser::BinopPrecedence[P.getOperatorName()] = P.getBinaryPrecedence();
  }

  // Create a new basic block to start insertion into.
  std::cout << "Create a new basic block to start insertion into";
  llvm::BasicBlock* BB =
    llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
  Builder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  std::cout << "Record the function arguments in the NamedValues map.";
  NamedValues.clear();

  for (auto& Arg : TheFunction->args()) {
    // Create an alloca for this variable.
    llvm::AllocaInst* Alloca =
      CreateEntryBlockAlloca(TheFunction, Arg.getName());

    // Store the initial value into the alloca.
    Builder->CreateStore(&Arg, Alloca);

    // Add arguments to variable symbol table.
    NamedValues[std::string(Arg.getName())] = Alloca;
  }

  expr->Body->accept(codegen);
  llvm::Value* RetVal = codegen->result_val;

  if (RetVal) {
    // Finish off the function.
    Builder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyFunction(*TheFunction);

    codegen->result_func = TheFunction;
    return;
  }

  // Error reading body, remove function.
  TheFunction->eraseFromParent();

  if (P.isBinaryOp()) {
    Parser::BinopPrecedence.erase(expr->Proto->getOperatorName());
  }

  codegen->result_func = nullptr;
}

/**
 * ===----------------------------------------------------------------------===
 *  Top-Level parsing and JIT Driver
 * ===----------------------------------------------------------------------===
 */

/**
 * @brief Open a new module.
 *
 */
static auto InitializeModuleAndPassManager() -> void {
  TheContext = std::make_unique<llvm::LLVMContext>();
  TheModule = std::make_unique<llvm::Module>("arx jit", *TheContext);

  /** Create a new builder for the module. */
  Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

/**
 * @brief
 *
 */
static auto HandleDefinition() -> void {
  if (auto FnAST = Parser::ParseDefinition()) {
    // note: not sure if it would work properly
    FnAST.get()->accept(codegen);
    llvm::Function* FnIR = codegen->result_func;

    if (FnIR) {
      fprintf(stderr, "Read function definition:");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");
    }
  } else {
    //  Skip token for error recovery. //
    Lexer::getNextToken();
  }
}

/**
 * @brief
 *
 */
static auto HandleExtern() -> void {
  if (auto ProtoAST = Parser::ParseExtern()) {
    ProtoAST.get()->accept(codegen);
    llvm::Function* FnIR = codegen->result_func;

    if (FnIR) {
      fprintf(stderr, "Read extern: ");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");
      FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
    }
  } else {
    //  Skip token for error recovery. //
    Lexer::getNextToken();
  }
}

/**
 * @brief Evaluate a top-level expression into an anonymous function.
 *
 */
static auto HandleTopLevelExpression() -> void {
  if (auto FnAST = Parser::ParseTopLevelExpr()) {
    FnAST.get()->accept(codegen);
  } else {
    //   Skip token for error recovery. //
    Lexer::getNextToken();
  }
}

/**
 * @brief
 * top ::= definition | external | expression | ';'
 */
static auto MainLoop() -> void {
  while (true) {
    switch (Lexer::CurTok) {
      case tok_eof:
        return;
      case ';':  // ignore top-level semicolons.
        Lexer::getNextToken();
        break;
      case tok_function:
        HandleDefinition();
        break;
      case tok_extern:
        HandleExtern();
        break;
      default:
        HandleTopLevelExpression();
        break;
    }
  }
}

//===----------------------------------------------------------------------===
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/**
 * @brief putchar that takes a double and returns 0.
 *
 */
extern "C" DLLEXPORT auto putchard(double X) -> double {
  fputc((char) X, stderr);
  return 0;
}

/**
 * @brief printf that takes a double prints it as "%f\n", returning 0.
 *
 */
extern "C" DLLEXPORT auto printd(double X) -> double {
  fprintf(stderr, "%f\n", X);
  return 0;
}

/**
 * @brief
 *
 */
auto compile(TreeAST* tree_ast) -> void {
  Lexer::getNextToken();

  InitializeModuleAndPassManager();

  // Run the main "interpreter loop" now.
  LOG(INFO) << "Starting MainLoop";

  MainLoop();

  LOG(INFO) << "Initialize Target";

  // Initialize the target registry etc.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  LOG(INFO) << "TargetTriple";

  auto TargetTriple = llvm::sys::getDefaultTargetTriple();
  TheModule->setTargetTriple(TargetTriple);

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (!Target) {
    llvm::errs() << Error;
    exit(1);
  }

  auto CPU = "generic";
  auto Features = "";

  LOG(INFO) << "Target Options";

  llvm::TargetOptions opt;
  auto RM = llvm::Optional<llvm::Reloc::Model>();

  LOG(INFO) << "Target Matchine";
  auto TheTargetMachine =
    Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

  LOG(INFO) << "Set Data Layout";

  TheModule->setDataLayout(TheTargetMachine->createDataLayout());

  LOG(INFO) << "dest output";
  std::error_code EC;

  if (OUTPUT_FILE == "") {
    OUTPUT_FILE = "./output.o";
  }

  llvm::raw_fd_ostream dest(OUTPUT_FILE, EC, llvm::sys::fs::OF_None);

  if (EC) {
    llvm::errs() << "Could not open file: " << EC.message();
    exit(1);
  }

  llvm::legacy::PassManager pass;
  auto FileType = llvm::CGFT_ObjectFile;

  if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    llvm::errs() << "TheTargetMachine can't emit a file of this type";
    exit(1);
  }

  pass.run(*TheModule);
  dest.flush();
}

/**
 * @brief
 *
 */
auto open_shell() -> void {
  // Prime the first token.
  fprintf(stderr, "Arx %s \n", ARX_VERSION.c_str());
  fprintf(stderr, ">>> ");

  compile(nullptr);

  exit(0);
}
