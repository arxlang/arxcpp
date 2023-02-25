#pragma once

#include <llvm/ADT/StringRef.h>   // for StringRef
#include <llvm/IR/IRBuilder.h>    // for IRBuilder
#include <llvm/IR/LLVMContext.h>  // for LLVMContext
#include <llvm/IR/Module.h>       // for Module
#include <map>                    // for map
#include <memory>                 // for unique_ptr
#include <string>                 // for string
#include "jit.h"                  // for ArxJIT
#include "parser.h"               // for PrototypeAST (ptr only), TreeAST (p...

namespace llvm {
  class AllocaInst;
}

namespace llvm {
  class Function;
}

namespace llvm {
  class Value;
}

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
