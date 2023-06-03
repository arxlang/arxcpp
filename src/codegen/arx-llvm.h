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

  static llvm::ExitOnError exit_on_err;

  /* Data types */
  static llvm::Type* DOUBLE_TYPE;
  static llvm::Type* FLOAT_TYPE;
  static llvm::Type* INT8_TYPE;
  static llvm::Type* INT32_TYPE;
  static llvm::Type* VOID_TYPE;

  /* Debug Information Data types */
  static llvm::DIType* DI_DOUBLE_TYPE;
  static llvm::DIType* DI_FLOAT_TYPE;
  static llvm::DIType* DI_INT8_TYPE;
  static llvm::DIType* DI_INT32_TYPE;
  static llvm::DIType* DI_VOID_TYPE;

  static auto get_data_type(std::string type_name) -> llvm::Type*;
  static auto get_di_data_type(std::string type_name) -> llvm::DIType*;
  static auto initialize() -> void;
};

extern bool IS_BUILD_LIB;
