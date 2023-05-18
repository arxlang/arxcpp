
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
llvm::Type* ArxLLVM::DOUBLE_TYPE;
llvm::Type* ArxLLVM::FLOAT_TYPE;
llvm::Type* ArxLLVM::INT8_TYPE;
llvm::Type* ArxLLVM::INT32_TYPE;

extern bool IS_BUILD_LIB = false;  // default value
