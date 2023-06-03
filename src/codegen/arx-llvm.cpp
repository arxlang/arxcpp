#include <string>

#include <glog/logging.h>               // for COMPACT_GOOGLE_LOG_INFO, LOG
#include <llvm/IR/DIBuilder.h>          // for DIBuilder
#include <llvm/IR/IRBuilder.h>          // for IRBuilder
#include <llvm/IR/Module.h>             // for Module
#include <llvm/Support/Error.h>         // for ExitOnError
#include <llvm/Support/TargetSelect.h>  // for InitializeAllAsmParsers, Init...
#include <llvm/Target/TargetMachine.h>  // for TargetMachine
#include <llvm/Target/TargetOptions.h>  // for TargetOptions

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

/* Debug Information Data types */
llvm::DIType* ArxLLVM::DI_FLOAT_TYPE;
llvm::DIType* ArxLLVM::DI_DOUBLE_TYPE;
llvm::DIType* ArxLLVM::DI_INT8_TYPE;
llvm::DIType* ArxLLVM::DI_INT32_TYPE;
llvm::DIType* ArxLLVM::DI_VOID_TYPE;

llvm::ExitOnError ArxLLVM::exit_on_err;

extern bool IS_BUILD_LIB = false;  // default value

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

auto ArxLLVM::initialize() -> void {
  // initialize the target registry etc.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  // initialize the target registry etc.
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  ArxLLVM::context = std::make_unique<llvm::LLVMContext>();
  ArxLLVM::module =
    std::make_unique<llvm::Module>("arx jit", *ArxLLVM::context);

  // Create a new builder for the module.
  ArxLLVM::ir_builder = std::make_unique<llvm::IRBuilder<>>(*ArxLLVM::context);

  // Data Types
  ArxLLVM::FLOAT_TYPE = llvm::Type::getFloatTy(*ArxLLVM::context);
  ArxLLVM::DOUBLE_TYPE = llvm::Type::getDoubleTy(*ArxLLVM::context);
  ArxLLVM::INT8_TYPE = llvm::Type::getInt8Ty(*ArxLLVM::context);
  ArxLLVM::INT32_TYPE = llvm::Type::getInt32Ty(*ArxLLVM::context);
  ArxLLVM::VOID_TYPE = llvm::Type::getVoidTy(*ArxLLVM::context);

  LOG(INFO) << "initialize Target";

  // LLVM IR

  ArxLLVM::jit = ArxLLVM::exit_on_err(llvm::orc::ArxJIT::Create());
  ArxLLVM::module->setDataLayout(ArxLLVM::jit->get_data_layout());

  // Create a new builder for the module.
  ArxLLVM::di_builder = std::make_unique<llvm::DIBuilder>(*ArxLLVM::module);

  // di data types
  ArxLLVM::DI_FLOAT_TYPE = ArxLLVM::di_builder->createBasicType(
    "float", 32, llvm::dwarf::DW_ATE_float);
  ArxLLVM::DI_DOUBLE_TYPE = ArxLLVM::di_builder->createBasicType(
    "double", 64, llvm::dwarf::DW_ATE_float);
  ArxLLVM::DI_INT8_TYPE = ArxLLVM::di_builder->createBasicType(
    "int8", 8, llvm::dwarf::DW_ATE_signed);
  ArxLLVM::DI_INT32_TYPE = ArxLLVM::di_builder->createBasicType(
    "int32", 32, llvm::dwarf::DW_ATE_signed);
}
