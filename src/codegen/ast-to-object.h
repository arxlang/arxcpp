#pragma once

#include <map>
#include <memory>

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

#include "jit.h"
#include "parser.h"

auto compile_object(TreeAST*) -> void;
auto open_shell_object() -> void;

class ASTToObjectVisitor : public Visitor {
 public:
  llvm::Value* result_val;
  llvm::Function* result_func;

  std::map<std::string, llvm::AllocaInst*> NamedValues;

  std::unique_ptr<llvm::LLVMContext> TheContext;
  std::unique_ptr<llvm::Module> TheModule;
  std::unique_ptr<llvm::IRBuilder<>> Builder;

  std::unique_ptr<llvm::orc::ArxJIT> TheJIT;
  std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

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

  auto getFunction(std::string Name) -> void;
  auto CreateEntryBlockAlloca(
    llvm::Function* TheFunction, llvm::StringRef VarName) -> llvm::AllocaInst*;
  auto MainLoop(TreeAST* ast) -> void;
  auto Initialize() -> void;
};
