
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

class ASTToLLVMIRVisitor : public Visitor {
 public:
  llvm::Value* result_val;
  llvm::Function* result_func;

  std::map<std::string, llvm::AllocaInst*> NamedValues;

  std::unique_ptr<llvm::LLVMContext> TheContext;
  std::unique_ptr<llvm::Module> TheModule;
  std::unique_ptr<llvm::IRBuilder<>> Builder;
  std::unique_ptr<llvm::DIBuilder> DBuilder;

  llvm::ExitOnError ExitOnErr;

  std::unique_ptr<llvm::orc::ArxJIT> TheJIT;
  std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

  // DebugInfo
  llvm::DICompileUnit* TheCU;
  llvm::DIType* DblTy;
  std::vector<llvm::DIScope*> LexicalBlocks;

  ~ASTToLLVMIRVisitor() {
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

  auto getFunction(std::string Name) -> void;
  auto CreateEntryBlockAlloca(
    llvm::Function* TheFunction, llvm::StringRef VarName) -> llvm::AllocaInst*;
  auto MainLoop(TreeAST* ast) -> void;
  auto InitializeModuleAndPassManager() -> void;

  auto CreateFunctionType(unsigned NumArgs) -> llvm::DISubroutineType* {
    llvm::SmallVector<llvm::Metadata*, 8> EltTys;
    llvm::DIType* DblTy = this->getDoubleTy();

    // Add the result type.
    EltTys.push_back(DblTy);

    for (unsigned i = 0, e = NumArgs; i != e; ++i) {
      EltTys.push_back(DblTy);
    }

    return this->DBuilder->createSubroutineType(
      this->DBuilder->getOrCreateTypeArray(EltTys));
  }

  // DebugInfo
  void emitLocation(ExprAST* AST);
  llvm::DIType* getDoubleTy();
};

/**
 * @brief Put the function defined by the given name to result_func.
 * @param Name Function name
 *
 * First, see if the function has already been added to the current
 * module. If not, check whether we can codegen the declaration from some
 * existing prototype. If no existing prototype exists, return null.
 */
auto ASTToLLVMIRVisitor::getFunction(std::string Name) -> void {
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
auto ASTToLLVMIRVisitor::CreateEntryBlockAlloca(
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
auto ASTToLLVMIRVisitor::clean() -> void {
  this->result_val = nullptr;
  this->result_func = nullptr;
}

// DebugInfo

auto ASTToLLVMIRVisitor::getDoubleTy() -> llvm::DIType* {
  if (this->DblTy) {
    return this->DblTy;
  }

  DblTy =
    this->DBuilder->createBasicType("double", 64, llvm::dwarf::DW_ATE_float);
  return DblTy;
}

auto ASTToLLVMIRVisitor::emitLocation(ExprAST* AST) -> void {
  if (!AST) {
    return this->Builder->SetCurrentDebugLocation(llvm::DebugLoc());
  }

  llvm::DIScope* Scope;
  if (this->LexicalBlocks.empty()) {
    Scope = TheCU;
  } else {
    Scope = this->LexicalBlocks.back();
  }

  this->Builder->SetCurrentDebugLocation(llvm::DILocation::get(
    Scope->getContext(), AST->getLine(), AST->getCol(), Scope));
}

/**
 * @brief Code generation for NumberExprAST.
 *
 */
auto ASTToLLVMIRVisitor::visit(NumberExprAST* expr) -> void {
  this->emitLocation(expr);

  this->result_val =
    llvm::ConstantFP::get(*this->TheContext, llvm::APFloat(expr->Val));
}

/**
 * @brief Code generation for VariableExprAST.
 *
 */
auto ASTToLLVMIRVisitor::visit(VariableExprAST* expr) -> void {
  this->emitLocation(expr);

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
auto ASTToLLVMIRVisitor::visit(UnaryExprAST* expr) -> void {
  this->emitLocation(expr);

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
auto ASTToLLVMIRVisitor::visit(BinaryExprAST* expr) -> void {
  this->emitLocation(expr);

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
auto ASTToLLVMIRVisitor::visit(CallExprAST* expr) -> void {
  this->emitLocation(expr);

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
auto ASTToLLVMIRVisitor::visit(IfExprAST* expr) -> void {
  this->emitLocation(expr);

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
auto ASTToLLVMIRVisitor::visit(ForExprAST* expr) -> void {
  this->emitLocation(expr);

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
auto ASTToLLVMIRVisitor::visit(VarExprAST* expr) -> void {
  this->emitLocation(expr);

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
auto ASTToLLVMIRVisitor::visit(PrototypeAST* expr) -> void {
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
auto ASTToLLVMIRVisitor::visit(FunctionAST* expr) -> void {
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

  /* debugging-code:start*/
  // Create a subprogram DIE for this function.
  llvm::DIFile* Unit = this->DBuilder->createFile(
    this->TheCU->getFilename(), this->TheCU->getDirectory());
  llvm::DIScope* FContext = Unit;
  unsigned LineNo = P.getLine();
  unsigned ScopeLine = LineNo;
  llvm::DISubprogram* SP = this->DBuilder->createFunction(
    FContext,
    P.getName(),
    llvm::StringRef(),
    Unit,
    LineNo,
    CreateFunctionType(TheFunction->arg_size()),
    ScopeLine,
    llvm::DINode::FlagPrototyped,
    llvm::DISubprogram::SPFlagDefinition);
  TheFunction->setSubprogram(SP);

  // Push the current scope.
  this->LexicalBlocks.push_back(SP);

  // Unset the location for the prologue emission (leading instructions with no
  // location in a function are considered part of the prologue and the
  // debugger will run past them when breaking on a function)
  this->emitLocation(nullptr);
  /* debugging-code:end*/

  // Record the function arguments in the NamedValues map.
  // std::cout << "Record the function arguments in the NamedValues map.";
  this->NamedValues.clear();

  unsigned ArgIdx = 0;
  for (auto& Arg : TheFunction->args()) {
    // Create an alloca for this variable.
    llvm::AllocaInst* Alloca =
      CreateEntryBlockAlloca(TheFunction, Arg.getName());

    /* debugging-code: start */
    // Create a debug descriptor for the variable.
    llvm::DILocalVariable* D = this->DBuilder->createParameterVariable(
      SP, Arg.getName(), ++ArgIdx, Unit, LineNo, this->getDoubleTy(), true);

    this->DBuilder->insertDeclare(
      Alloca,
      D,
      this->DBuilder->createExpression(),
      llvm::DILocation::get(SP->getContext(), LineNo, 0, SP),
      this->Builder->GetInsertBlock());

    /* debugging-code-end */

    // Store the initial value into the alloca.
    this->Builder->CreateStore(&Arg, Alloca);

    // Add arguments to variable symbol table.
    this->NamedValues[std::string(Arg.getName())] = Alloca;
  }

  this->emitLocation(expr->Body.get());

  expr->Body->accept(this);
  llvm::Value* RetVal = this->result_val;

  if (RetVal) {
    // Finish off the function.
    this->Builder->CreateRet(RetVal);

    // Pop off the lexical block for the function.
    this->LexicalBlocks.pop_back();

    // Validate the generated code, checking for consistency.
    verifyFunction(*TheFunction);

    this->result_func = TheFunction;
    return;
  }

  // Error reading body, remove function.
  TheFunction->eraseFromParent();

  this->result_func = nullptr;

  // Pop off the lexical block for the function since we added it
  // unconditionally.
  this->LexicalBlocks.pop_back();
}

/**
 * @brief Initialize LLVM Module And PassManager.
 *
 */
auto ASTToLLVMIRVisitor::InitializeModuleAndPassManager() -> void {
  this->TheContext = std::make_unique<llvm::LLVMContext>();
  this->TheModule =
    std::make_unique<llvm::Module>("arx jit", *this->TheContext);
  this->TheModule->setDataLayout(TheJIT->getDataLayout());

  /** Create a new builder for the module. */
  this->Builder = std::make_unique<llvm::IRBuilder<>>(*this->TheContext);
}

/**
 * @brief The main loop that walks the AST.
 * top ::= definition | external | expression | ';'
 */
auto ASTToLLVMIRVisitor::MainLoop(TreeAST* ast) -> void {
  for (auto node = ast->nodes.begin(); node != ast->nodes.end(); ++node) {
    node->get()->accept(this);
  }
}

/**
 * @brief Compile an AST to object file.
 *
 * @param ast The AST tree object.
 */
auto compile_llvm_ir(TreeAST* ast) -> void {
  auto codegen = new ASTToLLVMIRVisitor();

  Lexer::getNextToken();

  codegen->InitializeModuleAndPassManager();

  // Run the main "interpreter loop" now.
  LOG(INFO) << "Starting MainLoop";

  // Construct the DIBuilder, we do this here because we need the module.
  codegen->DBuilder = std::make_unique<llvm::DIBuilder>(*codegen->TheModule);

  // Create the compile unit for the module.
  // Currently down as "fib.ks" as a filename since we're redirecting stdin
  // but we'd like actual source locations.
  codegen->TheCU = codegen->DBuilder->createCompileUnit(
    llvm::dwarf::DW_LANG_C,
    codegen->DBuilder->createFile("fib.ks", "."),
    "Kaleidoscope Compiler",
    false,
    "",
    0);

  codegen->MainLoop(ast);

  LOG(INFO) << "Initialize Target";

  // Initialize the target registry etc.
  /*
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();
  */
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  codegen->TheJIT = codegen->ExitOnErr(llvm::orc::ArxJIT::Create());

  codegen->InitializeModuleAndPassManager();

  // Add the current debug info version into the module.
  codegen->TheModule->addModuleFlag(
    llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);

  // Darwin only supports dwarf2.
  if (llvm::Triple(llvm::sys::getProcessTriple()).isOSDarwin()) {
    codegen->TheModule->addModuleFlag(
      llvm::Module::Warning, "Dwarf Version", 2);
  }

  // Construct the DIBuilder, we do this here because we need the module.
  codegen->DBuilder = std::make_unique<llvm::DIBuilder>(*codegen->TheModule);

  // Create the compile unit for the module.
  // Currently down as "fib.ks" as a filename since we're redirecting stdin
  // but we'd like actual source locations.
  codegen->TheCU = codegen->DBuilder->createCompileUnit(
    llvm::dwarf::DW_LANG_C,
    codegen->DBuilder->createFile("fib.arxks", "."),
    "Arx Compiler",
    false,
    "",
    0);

  // Run the main "interpreter loop" now.
  codegen->MainLoop(ast);

  // Finalize the debug info.
  codegen->DBuilder->finalize();

  // Print out all of the generated code.
  codegen->TheModule->print(llvm::errs(), nullptr);
}

/**
 * @brief Open the Arx shell.
 *
 */
auto open_shell_llvm_ir() -> void {
  // Prime the first token.
  fprintf(stderr, "Arx %s \n", ARX_VERSION.c_str());
  fprintf(stderr, ">>> ");

  compile_llvm_ir(new TreeAST());

  exit(0);
}
