*note : arrow will not be used yet * #include<arrow / api.h> *
#include <arrow / csv / api.h> * #include<arrow / io / api.h> *
#include <arrow / ipc / api.h> * #include<arrow / pretty_print.h> *
#include <arrow / result.h> * #include<arrow / status.h> *
#include <arrow / table.h>

#include <algorithm>
#include <cassert>
#include <cctype>
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
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/Scalar.h>

#include "codegen.h"
#include "io.h"
#include "jit.h"
#include "lexer.h"
#include "parser.h"
#include "settings.h"
#include "utils.h"

    extern std::map<char, int> BinopPrecedence;
extern int CurTok;
extern std::string INPUT_FILE;
extern std::string OUTPUT_FILE;
extern std::string ARX_VERSION;

/**
 * ===----------------------------------------------------------------------===
 *  Code Generation Globals
 * ===----------------------------------------------------------------------===
 */
DebugInfo KSDbgInfo;

std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::Module> TheModule;
std::unique_ptr<llvm::IRBuilder<>> Builder;
llvm::ExitOnError ExitOnErr;

std::map<std::string, llvm::AllocaInst*> NamedValues;
std::unique_ptr<llvm::orc::ArxJIT> TheJIT;
std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

/**
 * ===----------------------------------------------------------------------===
 *  Debug Info Support
 * ===----------------------------------------------------------------------===
 */
std::unique_ptr<llvm::DIBuilder> DBuilder;

/**
 * @brief
 * @return
 */
llvm::DIType* DebugInfo::getDoubleTy() {
  if (DblTy) return DblTy;

  DblTy = DBuilder->createBasicType("double", 64, llvm::dwarf::DW_ATE_float);
  return DblTy;
}

/**
 * @brief
 * @param AST
 */
void DebugInfo::emitLocation(ExprAST* AST) {
  if (!AST) return Builder->SetCurrentDebugLocation(llvm::DebugLoc());
  llvm::DIScope* Scope = nullptr;
  if (LexicalBlocks.empty())
    Scope = TheCU;
  else
    Scope = LexicalBlocks.back();
  Builder->SetCurrentDebugLocation(llvm::DILocation::get(
      Scope->getContext(), AST->getLine(), AST->getCol(), Scope));
}

/**
 * @brief
 * @param NumArgs
 * @param Unit
 * @return
 */
static auto CreateFunctionType(unsigned NumArgs, llvm::DIFile* Unit)
    -> llvm::DISubroutineType* {
  llvm::SmallVector<llvm::Metadata*, 8> EltTys;
  llvm::DIType* DblTy = KSDbgInfo.getDoubleTy();

  /**
   *  Add the result type.
   */
  EltTys.push_back(DblTy);

  for (unsigned i = 0, e = NumArgs; i != e; ++i)
    EltTys.push_back(DblTy);

  return DBuilder->createSubroutineType(
      DBuilder->getOrCreateTypeArray(EltTys));
}

/**
 * ===----------------------------------------------------------------------===
 *  Code Generation
 * ===----------------------------------------------------------------------===
 */

/**
 * @brief
 * @param Name
 * @return
 *
 * First, see if the function has already been added to the current module.
 * If not, check whether we can codegen the declaration from some existing
 * prototype.
 * If no existing prototype exists, return null.
 */
auto getFunction(std::string Name) -> llvm::Function* {
  if (auto* F = TheModule->getFunction(Name)) return F;

  auto FI = FunctionProtos.find(Name);
  if (FI != FunctionProtos.end()) return FI->second->codegen();

  return nullptr;
}

/**
 * @brief
 * @param TheFunction
 * @param VarName
 * @return
 *
 * CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
 * the function.  This is used for mutable variables etc.
 */
static auto CreateEntryBlockAlloca(
    llvm::Function* TheFunction, llvm::StringRef VarName)
    -> llvm::AllocaInst* {
  llvm::IRBuilder<> TmpB(
      &TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(
      llvm::Type::getDoubleTy(*TheContext), nullptr, VarName);
}

/**
 * @brief
 * @return
 *
 */
llvm::Value* NumberExprAST::codegen() {
  KSDbgInfo.emitLocation(this);
  return llvm::ConstantFP::get(*TheContext, llvm::APFloat(Val));
}

/**
 * @brief Stat a variable in the function.
 * @param NamedValues Variable to stat
 * @return The variable loaded into the llvm.
 *
 */
llvm::Value* VariableExprAST::codegen() {
  llvm::Value* V = NamedValues[Name];
  if (!V) return LogErrorV("Unknown variable name");

  KSDbgInfo.emitLocation(this);
  return Builder->CreateLoad(V, Name.c_str());
}

/**
 * @brief
 * @return
 *
 */
llvm::Value* UnaryExprAST::codegen() {
  llvm::Value* OperandV = Operand->codegen();
  if (!OperandV) return nullptr;

  llvm::Function* F = getFunction(std::string("unary") + Opcode);
  if (!F) return LogErrorV("Unknown unary operator");

  KSDbgInfo.emitLocation(this);
  return Builder->CreateCall(F, OperandV, "unop");
}

/**
 * @brief
 * @return
 *
 */
llvm::Value* BinaryExprAST::codegen() {
  KSDbgInfo.emitLocation(this);

  /** Special case '=' because we don't want to emit the LHS as an
   * expression.*/
  if (Op == '=') {
    /** Assignment requires the LHS to be an identifier.
     * This assume we're building without RTTI because LLVM builds that way by
     * default.  If you build LLVM with RTTI this can be changed to a
     * dynamic_cast for automatic error checking.*/
    VariableExprAST* LHSE = static_cast<VariableExprAST*>(LHS.get());
    if (!LHSE) return LogErrorV("destination of '=' must be a variable");
    /** Codegen the RHS.*/
    llvm::Value* Val = RHS->codegen();
    if (!Val) return nullptr;

    /** Look up the name.*/
    llvm::Value* Variable = NamedValues[LHSE->getName()];
    if (!Variable) return LogErrorV("Unknown variable name");

    Builder->CreateStore(Val, Variable);
    return Val;
  }

  llvm::Value* L = LHS->codegen();
  llvm::Value* R = RHS->codegen();
  if (!L || !R) return nullptr;

  switch (Op) {
    case '+':
      return Builder->CreateFAdd(L, R, "addtmp");
    case '-':
      return Builder->CreateFSub(L, R, "subtmp");
    case '*':
      return Builder->CreateFMul(L, R, "multmp");
    case '<':
      L = Builder->CreateFCmpULT(L, R, "cmptmp");
      /** Convert bool 0/1 to double 0.0 or 1.0*/
      return Builder->CreateUIToFP(
          L, llvm::Type::getDoubleTy(*TheContext), "booltmp");
    default:
      break;
  }

  /**
   * If it wasn't a builtin binary operator, it must be a user defined one.
   * Emit a call to it.
   */
  llvm::Function* F = getFunction(std::string("binary") + Op);
  assert(F && "binary operator not found!");

  llvm::Value* Ops[] = {L, R};
  return Builder->CreateCall(F, Ops, "binop");
}

/**
 * @brief Look up the name in the global module table.
 * @return
 *
 */
llvm::Value* CallExprAST::codegen() {
  KSDbgInfo.emitLocation(this);

  llvm::Function* CalleeF = getFunction(Callee);
  if (!CalleeF) return LogErrorV("Unknown function referenced");

  if (CalleeF->arg_size() != Args.size())
    return LogErrorV("Incorrect # arguments passed");

  std::vector<llvm::Value*> ArgsV;
  for (auto& Arg : Args) {
    ArgsV.push_back(Arg->codegen());
    if (!ArgsV.back()) return nullptr;
  }

  return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

/**
 * @brief
 */
llvm::Value* IfExprAST::codegen() {
  KSDbgInfo.emitLocation(this);

  llvm::Value* CondV = Cond->codegen();
  if (!CondV) return nullptr;

  /**
   *  Convert condition to a bool by comparing non-equal to 0.0.
   */
  CondV = Builder->CreateFCmpONE(
      CondV, llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0)), "ifcond");

  llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();

  /**
   *  Create blocks for the then and else cases.  Insert the 'then' block at
   * the end of the function.
   */
  llvm::BasicBlock* ThenBB =
      llvm::BasicBlock::Create(*TheContext, "then", TheFunction);
  llvm::BasicBlock* ElseBB = llvm::BasicBlock::Create(*TheContext, "else");
  llvm::BasicBlock* MergeBB = llvm::BasicBlock::Create(*TheContext, "ifcont");

  Builder->CreateCondBr(CondV, ThenBB, ElseBB);

  /**
   *  Emit then value.
   */
  Builder->SetInsertPoint(ThenBB);

  llvm::Value* ThenV = Then->codegen();
  if (!ThenV) return nullptr;

  Builder->CreateBr(MergeBB);
  /**
   *  Codegen of 'Then' can change the current block, update ThenBB for the
   * PHI.
   */
  ThenBB = Builder->GetInsertBlock();

  /**
   *  Emit else block.
   */
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder->SetInsertPoint(ElseBB);

  llvm::Value* ElseV = Else->codegen();
  if (!ElseV) return nullptr;

  Builder->CreateBr(MergeBB);
  /**
   *  Codegen of 'Else' can change the current block, update ElseBB for the
   * PHI.
   */
  ElseBB = Builder->GetInsertBlock();

  /**
   *  Emit merge block.
   */
  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder->SetInsertPoint(MergeBB);
  llvm::PHINode* PN =
      Builder->CreatePHI(llvm::Type::getDoubleTy(*TheContext), 2, "iftmp");

  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);
  return PN;
}
/**
 * @brief
 * @return
 *
 * Output for-loop as:
 *   var = alloca double
 *   ...
 *   start = startexpr
 *   store start -> var
 *   goto loop
 * loop:
 *   ...
 *   bodyexpr
 *   ...
 * loopend:
 *   step = stepexpr
 *   endcond = endexpr
 *
 *   curvar = load var
 *   nextvar = curvar + step
 *   store nextvar -> var
 *   br endcond, loop, endloop
 * outloop:
 */
llvm::Value* ForExprAST::codegen() {
  llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();

  /**
   *  Create an alloca for the variable in the entry block.
   */
  llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, VarName);

  KSDbgInfo.emitLocation(this);

  /**
   *  Emit the start code first, without 'variable' in scope.
   */
  llvm::Value* StartVal = Start->codegen();
  if (!StartVal) return nullptr;

  /**
   *  Store the value into the alloca.
   */
  Builder->CreateStore(StartVal, Alloca);

  /**
   *  Make the new basic block for the loop header, inserting after current
   *  block.
   */
  llvm::BasicBlock* LoopBB =
      llvm::BasicBlock::Create(*TheContext, "loop", TheFunction);

  /**
   *  Insert an explicit fall through from the current block to the LoopBB.
   */
  Builder->CreateBr(LoopBB);

  /**
   *  Start insertion in LoopBB.
   */
  Builder->SetInsertPoint(LoopBB);

  /**
   *  Within the loop, the variable is defined equal to the PHI node.  If it
   *  shadows an existing variable, we have to restore it, so save it now.
   */
  llvm::AllocaInst* OldVal = NamedValues[VarName];
  NamedValues[VarName] = Alloca;

  /**
   *  Emit the body of the loop.  This, like any other expr, can change the
   *  current BB.  Note that we ignore the value computed by the body, but
   * don't allow an error.
   */
  if (!Body->codegen()) return nullptr;

  /**
   *  Emit the step value.
   */
  llvm::Value* StepVal = nullptr;
  if (Step) {
    StepVal = Step->codegen();
    if (!StepVal) return nullptr;
  } else {
    /**
     *  If not specified, use 1.0.
     */
    StepVal = llvm::ConstantFP::get(*TheContext, llvm::APFloat(1.0));
  }

  /**
   *  Compute the end condition.
   */
  llvm::Value* EndCond = End->codegen();
  if (!EndCond) return nullptr;

  /**
   *  Reload, increment, and restore the alloca.  This handles the case where
   *  the body of the loop mutates the variable.
   */
  llvm::Value* CurVar = Builder->CreateLoad(Alloca, VarName.c_str());
  llvm::Value* NextVar = Builder->CreateFAdd(CurVar, StepVal, "nextvar");
  Builder->CreateStore(NextVar, Alloca);

  /**
   *  Convert condition to a bool by comparing non-equal to 0.0.
   */
  EndCond = Builder->CreateFCmpONE(
      EndCond,
      llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0)),
      "loopcond");

  /**
   *  Create the "after loop" block and insert it.
   */
  llvm::BasicBlock* AfterBB =
      llvm::BasicBlock::Create(*TheContext, "afterloop", TheFunction);

  /**
   *  Insert the conditional branch into the end of LoopEndBB.
   */
  Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

  /**
   *  Any new code will be inserted in AfterBB.
   */
  Builder->SetInsertPoint(AfterBB);

  /**
   *  Restore the unshadowed variable.
   */
  if (OldVal)
    NamedValues[VarName] = OldVal;
  else
    NamedValues.erase(VarName);

  /**
   *  for expr always returns 0.0.
   */
  return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*TheContext));
}

/**
 * @brief
 * @return Return the body computation.
 *
 */
llvm::Value* VarExprAST::codegen() {
  std::vector<llvm::AllocaInst*> OldBindings;

  llvm::Function* TheFunction = Builder->GetInsertBlock()->getParent();

  /**
   *  Register all variables and emit their initializer.
   */
  for (auto& i : VarNames) {
    const std::string& VarName = i.first;
    ExprAST* Init = i.second.get();

    /**
     *  Emit the initializer before adding the variable to scope, this prevents
     *  the initializer from referencing the variable itself, and permits stuff
     *  like this:
     *   var a = 1 in
     *     var a = a in ...   # refers to outer 'a'.
     */
    llvm::Value* InitVal = nullptr;
    if (Init) {
      InitVal = Init->codegen();
      if (!InitVal) return nullptr;
    } else { /** If not specified, use 0.0. */
      InitVal = llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0));
    }

    llvm::AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
    Builder->CreateStore(InitVal, Alloca);

    /**
     *  Remember the old variable binding so that we can restore the binding
     *  when we unrecurse.
     */
    OldBindings.push_back(NamedValues[VarName]);

    /**
     *  Remember this binding.
     */
    NamedValues[VarName] = Alloca;
  }

  KSDbgInfo.emitLocation(this);

  /**
   *  Codegen the body, now that all vars are in scope.
   */
  llvm::Value* BodyVal = Body->codegen();
  if (!BodyVal) return nullptr;

  /**
   *  Pop all our variables from scope.
   */
  for (unsigned i = 0, e = VarNames.size(); i != e; ++i)
    NamedValues[VarNames[i].first] = OldBindings[i];

  return BodyVal;
}

/**
 * @brief
 * @return
 *
 */
llvm::Function* PrototypeAST::codegen() {
  /**
   *  Make the function type:  double(double,double) etc.
   */
  std::vector<llvm::Type*> Doubles(
      Args.size(), llvm::Type::getDoubleTy(*TheContext));
  llvm::FunctionType* FT = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(*TheContext), Doubles, false);

  llvm::Function* F = llvm::Function::Create(
      FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

  /**
   *  Set names for all arguments.
   */
  unsigned Idx = 0;
  for (auto& Arg : F->args())
    Arg.setName(Args[Idx++]);

  return F;
}

/**
 * @brief
 * @return
 *
 */
llvm::Function* FunctionAST::codegen() {
  /**
   *  Transfer ownership of the prototype to the FunctionProtos map, but keep a
   *  reference to it for use below.
   */
  auto& P = *Proto;
  FunctionProtos[Proto->getName()] = std::move(Proto);
  llvm::Function* TheFunction = getFunction(P.getName());
  if (!TheFunction) return nullptr;

  /**
   *  If this is an operator, install it.
   */
  if (P.isBinaryOp())
    BinopPrecedence[P.getOperatorName()] = P.getBinaryPrecedence();

  /**
   *  Create a new basic block to start insertion into.
   */
  llvm::BasicBlock* BB =
      llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
  Builder->SetInsertPoint(BB);

  /**
   *  Create a subprogram DIE for this function.
   */
  llvm::DIFile* Unit = DBuilder->createFile(
      KSDbgInfo.TheCU->getFilename(), KSDbgInfo.TheCU->getDirectory());
  llvm::DIScope* FContext = Unit;
  unsigned LineNo = P.getLine();
  unsigned ScopeLine = LineNo;
  llvm::DISubprogram* SP = DBuilder->createFunction(
      FContext,
      P.getName(),
      llvm::StringRef(),
      Unit,
      LineNo,
      CreateFunctionType(TheFunction->arg_size(), Unit),
      ScopeLine,
      llvm::DINode::FlagPrototyped,
      llvm::DISubprogram::SPFlagDefinition);
  TheFunction->setSubprogram(SP);

  /**
   *  Push the current scope.
   */
  KSDbgInfo.LexicalBlocks.push_back(SP);

  /**
   *  Unset the location for the prologue emission (leading instructions with
   * no location in a function are considered part of the prologue and the
   *  debugger will run past them when breaking on a function)
   */
  KSDbgInfo.emitLocation(nullptr);

  /**
   *  Record the function arguments in the NamedValues map.
   */
  NamedValues.clear();
  unsigned ArgIdx = 0;
  for (auto& Arg : TheFunction->args()) {
    /**
     *  Create an alloca for this variable.
     */
    llvm::AllocaInst* Alloca =
        CreateEntryBlockAlloca(TheFunction, Arg.getName());

    /**
     *  Create a debug descriptor for the variable.
     */
    llvm::DILocalVariable* D = DBuilder->createParameterVariable(
        SP,
        Arg.getName(),
        ++ArgIdx,
        Unit,
        LineNo,
        KSDbgInfo.getDoubleTy(),
        true);

    DBuilder->insertDeclare(
        Alloca,
        D,
        DBuilder->createExpression(),
        llvm::DILocation::get(SP->getContext(), LineNo, 0, SP),
        Builder->GetInsertBlock());

    /**
     *  Store the initial value into the alloca.
     */
    Builder->CreateStore(&Arg, Alloca);

    /**
     *  Add arguments to variable symbol table.
     */
    NamedValues[std::string(Arg.getName())] = Alloca;
  }

  KSDbgInfo.emitLocation(Body.get());

  if (llvm::Value* RetVal = Body->codegen()) {
    /**
     *  Finish off the function.
     */
    Builder->CreateRet(RetVal);

    /**
     *  Pop off the lexical block for the function.
     */
    KSDbgInfo.LexicalBlocks.pop_back();

    /**
     *  Validate the generated code, checking for consistency.
     */
    verifyFunction(*TheFunction);

    return TheFunction;
  }

  /**
   *  Error reading body, remove function.
   */
  TheFunction->eraseFromParent();

  if (P.isBinaryOp()) BinopPrecedence.erase(Proto->getOperatorName());

  /**
   *  Pop off the lexical block for the function since we added it
   *  unconditionally.
   */
  KSDbgInfo.LexicalBlocks.pop_back();

  return nullptr;
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
auto InitializeModule() -> void {
  TheContext = std::make_unique<llvm::LLVMContext>();
  TheModule = std::make_unique<llvm::Module>("arx-module", *TheContext);
  TheModule->setDataLayout(TheJIT->getDataLayout());

  Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

/**
 * @brief Open a new module.
 *
 */
auto InitializeModuleAndPassManager() -> void {
  TheContext = std::make_unique<llvm::LLVMContext>();
  TheModule = std::make_unique<llvm::Module>("arx jit", *TheContext);

  /** Create a new builder for the module. */
  Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

/**
 * @brief
 *
 */
auto HandleDefinition() -> void {
  if (auto FnAST = ParseDefinition()) {
    if (!FnAST->codegen())
      fprintf(stderr, "Error reading function definition:");
  } else {
    /**  Skip token for error recovery.*/
    getNextToken();
  }
}

/**
 * @brief
 *
 */
auto HandleExtern() -> void {
  if (auto ProtoAST = ParseExtern()) {
    if (!ProtoAST->codegen())
      fprintf(stderr, "Error reading extern");
    else
      FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
  } else {
    /**  Skip token for error recovery. */
    getNextToken();
  }
}

/**
 * @brief Evaluate a top-level expression into an anonymous function.
 *
 */
auto HandleTopLevelExpression() -> void {
  if (auto FnAST = ParseTopLevelExpr()) {
    if (!FnAST->codegen()) {
      fprintf(stderr, "Error generating code for top level expr");
    }
  } else {
    /**   Skip token for error recovery. */
    getNextToken();
  }
}

/**
 * @brief
 * top ::= definition | external | expression | ';'
 */
auto MainLoop() -> void {
  while (true) {
    switch (CurTok) {
      case tok_eof:
        return;
      case ';':  // ignore top-level semicolons.
        getNextToken();
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

/**
 * ===----------------------------------------------------------------------===
 *  "Library" functions that can be "extern'd" from user code.
 * ===----------------------------------------------------------------------===
 */
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
  fputc((char)X, stderr);
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
auto show_llvm() -> void {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  load_settings();

  /**
   *  Prime the first token.
   */
  getNextToken();

  TheJIT = ExitOnErr(llvm::orc::ArxJIT::Create());

  InitializeModule();

  /**
   *  Add the current debug info version into the module.
   */
  TheModule->addModuleFlag(
      llvm::Module::Warning,
      "Debug Info Version",
      llvm::DEBUG_METADATA_VERSION);
  /**
   *  Darwin only supports dwarf2.
   */
  if (llvm::Triple(llvm::sys::getProcessTriple()).isOSDarwin())
    TheModule->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);

  /**
   *  Construct the DIBuilder, we do this here because we need the module.
   */
  DBuilder = std::make_unique<llvm::DIBuilder>(*TheModule);

  /**
   *  Create the compile unit for the module.
   *  Currently down as "fib" as a filename since we're redirecting stdin
   *  but we'd like actual source locations.
   */
  KSDbgInfo.TheCU = DBuilder->createCompileUnit(
      llvm::dwarf::DW_LANG_C,
      DBuilder->createFile(OUTPUT_FILE, "."),
      "Arx Compiler",
      false,
      "",
      0);

  /**
   *  Run the main "interpreter loop" now.
   */
  MainLoop();

  /**
   *  Finalize the debug info.
   */
  DBuilder->finalize();

  /**
   *  Print out all of the generated code.
   */
  TheModule->print(llvm::errs(), nullptr);
  exit(0);
}

/**
 * @brief
 *
 */
auto compile() -> void {
  load_settings();

  getNextToken();

  InitializeModuleAndPassManager();

  /**
   *  Run the main "interpreter loop" now.
   */
  LOG(INFO) << "Starting MainLoop";

  MainLoop();

  LOG(INFO) << "Initialize Target";
  /**
   *  Initialize the target registry etc.
   */
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
  /**
   *  Print an error and exit if we couldn't find the requested target.
   *  This generally occurs if we've forgotten to initialise the
   *  TargetRegistry or we have a bogus target triple.
   */
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
auto compile_to_file() -> void {
  compile();
  llvm::outs() << "Wrote " << OUTPUT_FILE << "\n";
}

/**
 * @brief
 *
 */
auto open_shell() -> void {
  /**
   *  Prime the first token.
   */
  fprintf(stderr, "Arx %s \n", ARX_VERSION.c_str());
  fprintf(stderr, ">>> ");

  compile();

  exit(0);
}
