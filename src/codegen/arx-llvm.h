#pragma once

#include <llvm/IR/DIBuilder.h>   // for DIBuilder
#include <llvm/IR/IRBuilder.h>   // for IRBuilder
#include <llvm/IR/Module.h>      // for Module
#include <llvm/Support/Error.h>  // for ExitOnError

#include "codegen/jit.h"  // for ArxJIT
#include "parser.h"       // for ArxJIT

class ArxLLVM {
 public:
  static std::unique_ptr<llvm::LLVMContext> context;
  static std::unique_ptr<llvm::Module> module;
  static std::unique_ptr<llvm::IRBuilder<>> ir_builder;
  static std::unique_ptr<llvm::DIBuilder> di_builder;
  static std::unique_ptr<llvm::orc::ArxJIT> jit;

  static std::map<std::string, llvm::AllocaInst*> named_values;
  static std::map<std::string, std::unique_ptr<PrototypeAST>> function_protos;

  /* Data types */
  static llvm::Type* DOUBLE_TYPE;
  static llvm::Type* FLOAT_TYPE;
  static llvm::Type* INT8_TYPE;
  static llvm::Type* INT32_TYPE;
};
