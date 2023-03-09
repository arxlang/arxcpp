#include "codegen/ast-to-object.h"      // for ASTToObjectVisitor, compile_o...
#include <glog/logging.h>               // for COMPACT_GOOGLE_LOG_INFO, LOG
#include <llvm/ADT/APFloat.h>           // for APFloat
#include <llvm/ADT/iterator_range.h>    // for iterator_range
#include <llvm/ADT/Optional.h>          // for Optional
#include <llvm/ADT/StringRef.h>         // for StringRef
#include <llvm/ADT/Twine.h>             // for Twine
#include <llvm/IR/Argument.h>           // for Argument
#include <llvm/IR/BasicBlock.h>         // for BasicBlock
#include <llvm/IR/Constant.h>           // for Constant
#include <llvm/IR/Constants.h>          // for ConstantFP
#include <llvm/IR/DerivedTypes.h>       // for FunctionType
#include <llvm/IR/Function.h>           // for Function
#include <llvm/IR/Instructions.h>       // for AllocaInst, CallInst, PHINode
#include <llvm/IR/IRBuilder.h>          // for IRBuilder
#include <llvm/IR/LegacyPassManager.h>  // for PassManager
#include <llvm/IR/LLVMContext.h>        // for LLVMContext
#include <llvm/IR/Module.h>             // for Module
#include <llvm/IR/Type.h>               // for Type
#include <llvm/IR/Verifier.h>           // for verifyFunction
#include <llvm/MC/TargetRegistry.h>     // for Target, TargetRegistry
#include <llvm/Support/CodeGen.h>       // for CodeGenFileType, Model
#include <llvm/Support/FileSystem.h>    // for OpenFlags
#include <llvm/Support/Host.h>          // for getDefaultTargetTriple
#include <llvm/Support/raw_ostream.h>   // for errs, raw_fd_ostream, raw_ost...
#include <llvm/Support/TargetSelect.h>  // for InitializeAllAsmParsers, Init...
#include <llvm/Target/TargetMachine.h>  // for TargetMachine
#include <llvm/Target/TargetOptions.h>  // for TargetOptions
#include <cassert>                      // for assert
#include <cstdio>                       // for fprintf, stderr, fputc
#include <cstdlib>                      // for exit
#include <fstream>                      // for operator<<
#include <map>                          // for map, operator==, _Rb_tree_ite...
#include <memory>                       // for unique_ptr, allocator, make_u...
#include <string>                       // for string, operator<=>, operator+
#include <system_error>                 // for error_code
#include <utility>                      // for pair, move
#include <vector>                       // for vector
#include "error.h"                      // for LogErrorV
#include "lexer.h"                      // for Lexer
#include "parser.h"                     // for PrototypeAST, ExprAST, ForExp...

namespace llvm {
  class Value;
}

extern std::string INPUT_FILE;
extern std::string OUTPUT_FILE;
extern std::string ARX_VERSION;

/**
 * @brief Put the function defined by the given name to result_func.
 * @param Name Function name
 *
 * First, see if the function has already been added to the current
 * module. If not, check whether we can codegen the declaration from some
 * existing prototype. If no existing prototype exists, return null.
 */
auto ASTToObjectVisitor::getFunction(std::string Name) -> void {
  if (auto* F = this->TheModule->getFunction(Name)) {
    this->result_func = F;
    return;
  }

  auto FI = FunctionProtos.find(Name);
  if (FI != FunctionProtos.end()) {
    FI->second->accept(this);
  };
}

/**
 * @brief Create the Entry Block Allocation.
 * @param TheFunction The llvm function
 * @param VarName The variable name
 * @return An llvm allocation instance.
 *
 * CreateEntryBlockAlloca - Create an alloca instruction in the entry
 * block of the function.  This is used for mutable variables etc.
 */
auto ASTToObjectVisitor::CreateEntryBlockAlloca(
  llvm::Function* TheFunction, llvm::StringRef VarName) -> llvm::AllocaInst* {
  llvm::IRBuilder<> TmpB(
    &TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(
    llvm::Type::getDoubleTy(*this->TheContext), nullptr, VarName);
}

/**
 * @brief Set to nullptr result_val and result_func in order to avoid trash.
 *
 */
auto ASTToObjectVisitor::clean() -> void {
  this->result_val = nullptr;
  this->result_func = nullptr;
}

/**
 * @brief Code generation for FloatExprAST.
 *
 */
auto ASTToObjectVisitor::visit(FloatExprAST* expr) -> void {
  this->result_val =
    llvm::ConstantFP::get(*this->TheContext, llvm::APFloat(expr->Val));
}

/**
 * @brief Code generation for VariableExprAST.
 *
 */
auto ASTToObjectVisitor::visit(VariableExprAST* expr) -> void {
  llvm::Value* V = this->NamedValues[expr->Name];

  if (!V) {
    auto msg = std::string("Unknown variable name: ") + expr->Name;
    this->result_val = LogErrorV(msg.c_str());
    return;
  }

  this->result_val = this->Builder->CreateLoad(
    llvm::Type::getDoubleTy(*this->TheContext), V, expr->Name.c_str());
}

/**
 * @brief Code generation for UnaryExprAST.
 *
 */
auto ASTToObjectVisitor::visit(UnaryExprAST* expr) -> void {
  expr->Operand.get()->accept(this);
  llvm::Value* OperandV = this->result_val;

  if (!OperandV) {
    this->result_val = nullptr;
    return;
  }

  this->getFunction(std::string("unary") + expr->Opcode);
  llvm::Function* F = this->result_func;
  if (!F) {
    this->result_val = LogErrorV("Unknown unary operator");
    return;
  }

  this->result_val = this->Builder->CreateCall(F, OperandV, "unop");
}

/**
 * @brief Code generation for BinaryExprAST.
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
      this->result_val = LogErrorV("destination of '=' must be a variable");
      return;
    }

    // Codegen the RHS.//
    expr->RHS.get()->accept(this);
    llvm::Value* Val = this->result_val;

    if (!Val) {
      this->result_val = nullptr;
      return;
    };

    // Look up the name.//
    llvm::Value* Variable = this->NamedValues[LHSE->getName()];
    if (!Variable) {
      this->result_val = LogErrorV("Unknown variable name");
      return;
    }

    this->Builder->CreateStore(Val, Variable);
    this->result_val = Val;
  }

  expr->LHS.get()->accept(this);
  llvm::Value* L = this->result_val;
  expr->RHS.get()->accept(this);
  llvm::Value* R = this->result_val;

  if (!L || !R) {
    this->result_val = nullptr;
    return;
  }

  switch (expr->Op) {
    case '+':
      this->result_val = this->Builder->CreateFAdd(L, R, "addtmp");
      return;
    case '-':
      this->result_val = this->Builder->CreateFSub(L, R, "subtmp");
      return;
    case '*':
      this->result_val = this->Builder->CreateFMul(L, R, "multmp");
      return;
    case '<':
      L = this->Builder->CreateFCmpULT(L, R, "cmptmp");
      // Convert bool 0/1 to double 0.0 or 1.0 //
      this->result_val = this->Builder->CreateUIToFP(
        L, llvm::Type::getDoubleTy(*this->TheContext), "booltmp");
      return;
  }

  // If it wasn't a builtin binary operator, it must be a user defined
  // one. Emit a call to it.
  this->getFunction(std::string("binary") + expr->Op);
  llvm::Function* F = this->result_func;
  assert(F && "binary operator not found!");

  llvm::Value* Ops[] = {L, R};
  this->result_val = this->Builder->CreateCall(F, Ops, "binop");
}

/**
 * @brief Code generation for CallExprAST.
 *
 */
auto ASTToObjectVisitor::visit(CallExprAST* expr) -> void {
  this->getFunction(expr->Callee);
  llvm::Function* CalleeF = this->result_func;
  if (!CalleeF) {
    this->result_val = LogErrorV("Unknown function referenced");
    return;
  }

  if (CalleeF->arg_size() != expr->Args.size()) {
    this->result_val = LogErrorV("Incorrect # arguments passed");
    return;
  }

  std::vector<llvm::Value*> ArgsV;
  for (unsigned i = 0, e = expr->Args.size(); i != e; ++i) {
    expr->Args[i].get()->accept(this);
    llvm::Value* ArgsV_item = this->result_val;
    ArgsV.push_back(ArgsV_item);
    if (!ArgsV.back()) {
      this->result_val = nullptr;
      return;
    }
  }

  this->result_val = this->Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

/**
 * @brief Code generation for IfExprAST.
 */
auto ASTToObjectVisitor::visit(IfExprAST* expr) -> void {
  expr->Cond.get()->accept(this);
  llvm::Value* CondV = this->result_val;

  if (!CondV) {
    this->result_val = nullptr;
    return;
  }

  // Convert condition to a bool by comparing non-equal to 0.0.
  CondV = this->Builder->CreateFCmpONE(
    CondV,
    llvm::ConstantFP::get(*this->TheContext, llvm::APFloat(0.0)),
    "ifcond");

  llvm::Function* TheFunction = this->Builder->GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block
  // at the end of the function.
  llvm::BasicBlock* ThenBB =
    llvm::BasicBlock::Create(*this->TheContext, "then", TheFunction);
  llvm::BasicBlock* ElseBB =
    llvm::BasicBlock::Create(*this->TheContext, "else");
  llvm::BasicBlock* MergeBB =
    llvm::BasicBlock::Create(*this->TheContext, "ifcont");

  this->Builder->CreateCondBr(CondV, ThenBB, ElseBB);

  // Emit then value.
  this->Builder->SetInsertPoint(ThenBB);

  expr->Then.get()->accept(this);
  llvm::Value* ThenV = this->result_val;
  if (!ThenV) {
    this->result_val = nullptr;
    return;
  }

  this->Builder->CreateBr(MergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for
  // the PHI.
  ThenBB = this->Builder->GetInsertBlock();

  // Emit else block.
  TheFunction->getBasicBlockList().push_back(ElseBB);
  this->Builder->SetInsertPoint(ElseBB);

  expr->Else.get()->accept(this);
  llvm::Value* ElseV = this->result_val;
  if (!ElseV) {
    this->result_val = nullptr;
    return;
  }

  this->Builder->CreateBr(MergeBB);
  // Codegen of 'Else' can change the current block, update ElseBB for
  // the PHI.
  ElseBB = this->Builder->GetInsertBlock();

  // Emit merge block.
  TheFunction->getBasicBlockList().push_back(MergeBB);
  this->Builder->SetInsertPoint(MergeBB);
  llvm::PHINode* PN = this->Builder->CreatePHI(
    llvm::Type::getDoubleTy(*this->TheContext), 2, "iftmp");

  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);

  this->result_val = PN;
  return;
}

/**
 * @brief Code generation for ForExprAST.
 *
 * @param expr A `for` expression.
 */
auto ASTToObjectVisitor::visit(ForExprAST* expr) -> void {
  llvm::Function* TheFunction = this->Builder->GetInsertBlock()->getParent();

  // Create an alloca for the variable in the entry block.
  llvm::AllocaInst* Alloca =
    CreateEntryBlockAlloca(TheFunction, expr->VarName);

  // Emit the start code first, without 'variable' in scope.
  expr->Start.get()->accept(this);
  llvm::Value* StartVal = this->result_val;
  if (!StartVal) {
    this->result_val = nullptr;
    return;
  }

  // Store the value into the alloca.
  this->Builder->CreateStore(StartVal, Alloca);

  // Make the new basic block for the loop header, inserting after
  // current block.
  llvm::BasicBlock* LoopBB =
    llvm::BasicBlock::Create(*this->TheContext, "loop", TheFunction);

  // Insert an explicit fall through from the current block to the
  // LoopBB.
  this->Builder->CreateBr(LoopBB);

  // Start insertion in LoopBB.
  this->Builder->SetInsertPoint(LoopBB);

  // Within the loop, the variable is defined equal to the PHI node.  If
  // it shadows an existing variable, we have to restore it, so save it
  // now.
  llvm::AllocaInst* OldVal = this->NamedValues[expr->VarName];
  this->NamedValues[expr->VarName] = Alloca;

  // Emit the body of the loop.  This, like any other expr, can change
  // the current BB.  Note that we ignore the value computed by the body,
  // but don't allow an error.
  expr->Body.get()->accept(this);
  llvm::Value* BodyVal = this->result_val;

  if (!BodyVal) {
    this->result_val = nullptr;
    return;
  }

  // Emit the step value.
  llvm::Value* StepVal = nullptr;
  if (expr->Step) {
    expr->Step.get()->accept(this);
    StepVal = this->result_val;
    if (!StepVal) {
      this->result_val = nullptr;
      return;
    }
  } else {
    // If not specified, use 1.0.
    StepVal = llvm::ConstantFP::get(*this->TheContext, llvm::APFloat(1.0));
  }

  // Compute the end condition.
  expr->End.get()->accept(this);
  llvm::Value* EndCond = this->result_val;
  if (!EndCond) {
    this->result_val = nullptr;
    return;
  }

  // Reload, increment, and restore the alloca.  This handles the case
  // where the body of the loop mutates the variable.
  llvm::Value* CurVar = this->Builder->CreateLoad(
    llvm::Type::getDoubleTy(*this->TheContext), Alloca, expr->VarName.c_str());
  llvm::Value* NextVar = this->Builder->CreateFAdd(CurVar, StepVal, "nextvar");
  this->Builder->CreateStore(NextVar, Alloca);

  // Convert condition to a bool by comparing non-equal to 0.0.
  EndCond = this->Builder->CreateFCmpONE(
    EndCond,
    llvm::ConstantFP::get(*this->TheContext, llvm::APFloat(0.0)),
    "loopcond");

  // Create the "after loop" block and insert it.
  llvm::BasicBlock* AfterBB =
    llvm::BasicBlock::Create(*this->TheContext, "afterloop", TheFunction);

  // Insert the conditional branch into the end of LoopEndBB.
  this->Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

  // Any new code will be inserted in AfterBB.
  this->Builder->SetInsertPoint(AfterBB);

  // Restore the unshadowed variable.
  if (OldVal) {
    this->NamedValues[expr->VarName] = OldVal;
  } else {
    this->NamedValues.erase(expr->VarName);
  }

  // for expr always returns 0.0.
  this->result_val =
    llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*this->TheContext));
}

/**
 * @brief Code generation for VarExprAST.
 *
 */
auto ASTToObjectVisitor::visit(VarExprAST* expr) -> void {
  std::vector<llvm::AllocaInst*> OldBindings;

  llvm::Function* TheFunction = this->Builder->GetInsertBlock()->getParent();

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
      Init->accept(this);
      InitVal = this->result_val;
      if (!InitVal) {
        this->result_val = nullptr;
        return;
      }
    } else {  // If not specified, use 0.0.
      InitVal = llvm::ConstantFP::get(*this->TheContext, llvm::APFloat(0.0));
    }

    llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
    this->Builder->CreateStore(InitVal, Alloca);

    // Remember the old variable binding so that we can restore the
    // binding when we unrecurse.
    OldBindings.push_back(this->NamedValues[VarName]);

    // Remember this binding.
    this->NamedValues[VarName] = Alloca;
  }

  // Codegen the body, now that all vars are in scope.
  expr->Body.get()->accept(this);
  llvm::Value* BodyVal = this->result_val;
  if (!BodyVal) {
    this->result_val = nullptr;
    return;
  }

  // Pop all our variables from scope.
  for (unsigned i = 0, e = expr->VarNames.size(); i != e; ++i) {
    this->NamedValues[expr->VarNames[i].first] = OldBindings[i];
  }

  // Return the body computation.
  this->result_val = BodyVal;
}

/**
 * @brief Code generation for PrototypeExprAST.
 *
 */
auto ASTToObjectVisitor::visit(PrototypeAST* expr) -> void {
  // Make the function type:  double(double,double) etc.
  std::vector<llvm::Type*> Doubles(
    expr->Args.size(), llvm::Type::getDoubleTy(*this->TheContext));
  llvm::FunctionType* FT = llvm::FunctionType::get(
    llvm::Type::getDoubleTy(*this->TheContext), Doubles, false);

  llvm::Function* F = llvm::Function::Create(
    FT, llvm::Function::ExternalLinkage, expr->Name, this->TheModule.get());

  // Set names for all arguments.
  unsigned Idx = 0;
  for (auto& Arg : F->args()) {
    Arg.setName(expr->Args[Idx++]->Name);
  }

  this->result_func = F;
}

/**
 * @brief Code generation for FunctionExprAST.
 *
 * Transfer ownership of the prototype to the FunctionProtos map, but
 * keep a reference to it for use below.
 */
auto ASTToObjectVisitor::visit(FunctionAST* expr) -> void {
  auto& P = *(expr->Proto);
  FunctionProtos[expr->Proto->getName()] = std::move(expr->Proto);
  this->getFunction(P.getName());
  llvm::Function* TheFunction = this->result_func;

  if (!TheFunction) {
    this->result_func = nullptr;
    return;
  }

  // Create a new basic block to start insertion into.
  // std::cout << "Create a new basic block to start insertion into";
  llvm::BasicBlock* BB =
    llvm::BasicBlock::Create(*this->TheContext, "entry", TheFunction);
  this->Builder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  // std::cout << "Record the function arguments in the NamedValues map.";
  this->NamedValues.clear();

  for (auto& Arg : TheFunction->args()) {
    // Create an alloca for this variable.
    llvm::AllocaInst* Alloca =
      CreateEntryBlockAlloca(TheFunction, Arg.getName());

    // Store the initial value into the alloca.
    this->Builder->CreateStore(&Arg, Alloca);

    // Add arguments to variable symbol table.
    this->NamedValues[std::string(Arg.getName())] = Alloca;
  }

  expr->Body->accept(this);
  llvm::Value* RetVal = this->result_val;

  if (RetVal) {
    // Finish off the function.
    this->Builder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyFunction(*TheFunction);

    this->result_func = TheFunction;
    return;
  }

  // Error reading body, remove function.
  TheFunction->eraseFromParent();

  this->result_func = nullptr;
}

/**
 * @brief Initialize LLVM Module And PassManager.
 *
 */
auto ASTToObjectVisitor::Initialize() -> void {
  this->TheContext = std::make_unique<llvm::LLVMContext>();
  this->TheModule =
    std::make_unique<llvm::Module>("arx jit", *this->TheContext);

  /** Create a new builder for the module. */
  this->Builder = std::make_unique<llvm::IRBuilder<>>(*this->TheContext);
}

/**
 * @brief The main loop that walks the AST.
 * top ::= definition | external | expression | ';'
 */
auto ASTToObjectVisitor::MainLoop(TreeAST* ast) -> void {
  for (auto node = ast->nodes.begin(); node != ast->nodes.end(); ++node) {
    node->get()->accept(this);
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
  fputc(static_cast<char>(X), stderr);
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
 * @brief Compile an AST to object file.
 *
 * @param tree_ast The AST tree object.
 */
auto compile_object(TreeAST* tree_ast) -> void {
  auto codegen = new ASTToObjectVisitor();

  Lexer::getNextToken();

  codegen->Initialize();

  // Run the main "interpreter loop" now.
  LOG(INFO) << "Starting MainLoop";

  codegen->MainLoop(tree_ast);

  LOG(INFO) << "Initialize Target";

  // Initialize the target registry etc.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  LOG(INFO) << "TargetTriple";

  auto TargetTriple = llvm::sys::getDefaultTargetTriple();
  codegen->TheModule->setTargetTriple(TargetTriple);

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

  codegen->TheModule->setDataLayout(TheTargetMachine->createDataLayout());

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

  pass.run(*codegen->TheModule);
  dest.flush();
}

/**
 * @brief Open the Arx shell.
 *
 */
auto open_shell_object() -> void {
  // Prime the first token.
  fprintf(stderr, "Arx %s \n", ARX_VERSION.c_str());
  fprintf(stderr, ">>> ");

  compile_object(new TreeAST());

  exit(0);
}
