#pragma once

#include <llvm/ADT/StringRef.h>   // for StringRef
#include <llvm/IR/IRBuilder.h>    // for IRBuilder
#include <llvm/IR/LLVMContext.h>  // for LLVMContext
#include <llvm/IR/Module.h>       // for Module
#include <map>                    // for map
#include <memory>                 // for unique_ptr
#include <string>                 // for string
#include "codegen/jit.h"          // for ArxJIT
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

auto compile_object(std::unique_ptr<TreeAST>) -> void;
auto open_shell_object() -> void;

class ASTToObjectVisitor
    : public std::enable_shared_from_this<ASTToObjectVisitor>,
      public Visitor {
 public:
  llvm::Value* result_val;
  llvm::Function* result_func;

  ASTToObjectVisitor() = default;

  virtual void visit(FloatExprAST*) override;
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
  auto MainLoop(std::unique_ptr<TreeAST> ast) -> void;
  auto Initialize() -> void;
};
