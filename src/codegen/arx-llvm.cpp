#include <string>

#include <llvm/IR/DIBuilder.h>   // for DIBuilder
#include <llvm/IR/IRBuilder.h>   // for IRBuilder
#include <llvm/IR/Module.h>      // for Module
#include <llvm/Support/Error.h>  // for ExitOnError

#include "codegen/arx-llvm.h"  // for ArxLLVM
#include "codegen/jit.h"       // for ArxJIT
#include "parser.h"            // for ArxJIT

std::unique_ptr<llvm::LLVMContext> ArxLLVM::context;
std::unique_ptr<llvm::Module> ArxLLVM::module;
std::unique_ptr<llvm::IRBuilder<>> ArxLLVM::ir_builder;
std::unique_ptr<llvm::DIBuilder> ArxLLVM::di_builder;
std::unique_ptr<llvm::orc::ArxJIT> ArxLLVM::jit;

std::map<std::string, llvm::AllocaInst*> ArxLLVM::named_values;
std::map<std::string, std::unique_ptr<PrototypeAST>> ArxLLVM::function_protos;

/* Data types */
llvm::Type* ArxLLVM::FLOAT_TYPE;
llvm::Type* ArxLLVM::DOUBLE_TYPE;
llvm::Type* ArxLLVM::INT8_TYPE;
llvm::Type* ArxLLVM::INT32_TYPE;
llvm::Type* ArxLLVM::VOID_TYPE;

auto ArxLLVM::get_data_type(std::string type_name) -> llvm::Type* {
  if (type_name == "float") {
    return ArxLLVM::FLOAT_TYPE;
  } else if (type_name == "double") {
    return ArxLLVM::DOUBLE_TYPE;
  } else if (type_name == "int8") {
    return ArxLLVM::INT8_TYPE;
  } else if (type_name == "int32") {
    return ArxLLVM::INT32_TYPE;
  } else if (type_name == "char") {
    return ArxLLVM::INT8_TYPE;
  } else if (type_name == "void") {
    return ArxLLVM::VOID_TYPE;
  }

  llvm::errs() << "[EE] type_name not valid.\n";
  return nullptr;
}

/* Debug Information Data types */
llvm::DIType* ArxLLVM::DI_FLOAT_TYPE;
llvm::DIType* ArxLLVM::DI_DOUBLE_TYPE;
llvm::DIType* ArxLLVM::DI_INT8_TYPE;
llvm::DIType* ArxLLVM::DI_INT32_TYPE;
llvm::DIType* ArxLLVM::DI_VOID_TYPE;

extern bool IS_BUILD_LIB = false;  // default value

llvm::Type* ArxLLVM::VOID_TYPE;

auto ArxLLVM::get_data_type(std::string type_name) -> llvm::Type* {
  if (type_name == "float") {
    return ArxLLVM::FLOAT_TYPE;
  } else if (type_name == "double") {
    return ArxLLVM::DOUBLE_TYPE;
  } else if (type_name == "int8") {
    return ArxLLVM::INT8_TYPE;
  } else if (type_name == "int32") {
    return ArxLLVM::INT32_TYPE;
  } else if (type_name == "char") {
    return ArxLLVM::INT8_TYPE;
  } else if (type_name == "void") {
    return ArxLLVM::VOID_TYPE;
  }

  llvm::errs() << "[EE] type_name not valid.\n";
  return nullptr;
}

auto ArxLLVM::get_di_data_type(std::string di_type_name) -> llvm::DIType* {
  if (di_type_name == "float") {
    return ArxLLVM::DI_FLOAT_TYPE;
  } else if (di_type_name == "double") {
    return ArxLLVM::DI_DOUBLE_TYPE;
  } else if (di_type_name == "int8") {
    return ArxLLVM::DI_INT8_TYPE;
  } else if (di_type_name == "int32") {
    return ArxLLVM::DI_INT32_TYPE;
  } else if (di_type_name == "char") {
    return ArxLLVM::DI_INT8_TYPE;
  } else if (di_type_name == "void") {
    return ArxLLVM::DI_VOID_TYPE;
  }

  llvm::errs() << "[EE] di_type_name not valid.\n";
  return nullptr;
}
