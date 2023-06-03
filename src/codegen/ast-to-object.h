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

auto compile_object(TreeAST&) -> int;
auto open_shell_object() -> int;

class ASTToObjectVisitor : public Visitor {
 public:
  llvm::Value* result_val;
  llvm::Function* result_func;

  ASTToObjectVisitor() = default;

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
  virtual void clean() override;

  auto getFunction(std::string name) -> void;
  auto create_entry_block_alloca(
    llvm::Function* fn, llvm::StringRef var_name, std::string type_name)
    -> llvm::AllocaInst*;
  auto main_loop(TreeAST&) -> void;
  auto initialize() -> void;
};
