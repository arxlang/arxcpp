#pragma once

#include <vector>  // for vector

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
#include <llvm/Support/Error.h>  // for ExitOnError
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/Scalar.h>

#include "codegen/ast-to-object.h"
#include "parser.h"

auto compile_llvm_ir(TreeAST&) -> int;
auto open_shell_llvm_ir() -> int;

class ASTToLLVMIRVisitor : public ASTToObjectVisitor {
 public:
  // DebugInfo
  llvm::DICompileUnit* llvm_di_compile_unit;
  std::vector<llvm::DIScope*> llvm_di_lexical_blocks;

  llvm::ExitOnError exit_on_err;

  ASTToLLVMIRVisitor() = default;

  virtual void visit(FloatExprAST&) override;
  virtual void visit(VariableExprAST&) override;
  virtual void visit(UnaryExprAST&) override;
  virtual void visit(BinaryExprAST&) override;
  virtual void visit(CallExprAST&) override;
  virtual void visit(IfExprAST&) override;
  virtual void visit(ForExprAST&) override;
  virtual void visit(VarExprAST&) override;
  virtual void visit(PrototypeAST&) override;
  virtual void visit(FunctionAST&) override;

  auto initialize() -> void;
  auto CreateFunctionType(unsigned NumArgs) -> llvm::DISubroutineType*;

  // DebugInfo
  void emitLocation(ExprAST& AST);
};
