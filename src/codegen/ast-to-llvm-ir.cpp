#include "codegen/ast-to-llvm-ir.h"     // for ASTToLLVMIRVisitor, compile_l...
#include <glog/logging.h>               // for COMPACT_GOOGLE_LOG_INFO, LOG
#include <llvm/ADT/iterator_range.h>    // for iterator_range
#include <llvm/ADT/SmallVector.h>       // for SmallVector
#include <llvm/ADT/StringRef.h>         // for StringRef
#include <llvm/ADT/Triple.h>            // for Triple
#include <llvm/BinaryFormat/Dwarf.h>    // for SourceLanguage, TypeKind
#include <llvm/IR/Argument.h>           // for Argument
#include <llvm/IR/BasicBlock.h>         // for BasicBlock
#include <llvm/IR/DebugInfoMetadata.h>  // for DISubprogram, DICompileUnit
#include <llvm/IR/DebugLoc.h>           // for DebugLoc
#include <llvm/IR/DIBuilder.h>          // for DIBuilder
#include <llvm/IR/Function.h>           // for Function
#include <llvm/IR/Instructions.h>       // for AllocaInst
#include <llvm/IR/IRBuilder.h>          // for IRBuilder
#include <llvm/IR/Metadata.h>           // for LLVMConstants, Metadata (ptr ...
#include <llvm/IR/Module.h>             // for Module
#include <llvm/IR/Verifier.h>           // for verifyFunction
#include <llvm/Support/Host.h>          // for getProcessTriple
#include <llvm/Support/raw_ostream.h>   // for errs, raw_fd_ostream
#include <llvm/Support/TargetSelect.h>  // for InitializeNativeTarget, Initi...
#include <cstdio>                       // for fprintf, stderr
#include <cstdlib>                      // for exit
#include <fstream>                      // for operator<<
#include <map>                          // for map
#include <memory>                       // for unique_ptr, make_unique
#include <string>                       // for string, operator<=>
#include <utility>                      // for move
#include <vector>                       // for vector
#include "codegen/arx-llvm.h"           // for ArxLLVM
#include "codegen/ast-to-object.h"      // for ASTToObjectVisitor
#include "codegen/jit.h"                // for ArxJIT
#include "lexer.h"                      // for Lexer
#include "parser.h"                     // for PrototypeAST, FunctionAST

namespace llvm {
  class Value;
}

extern std::string INPUT_FILE;
extern std::string OUTPUT_FILE;
extern std::string ARX_VERSION;

auto ASTToLLVMIRVisitor::CreateFunctionType(unsigned NumArgs)
  -> llvm::DISubroutineType* {
  llvm::SmallVector<llvm::Metadata*, 8> EltTys;

  // Add the result type.
  EltTys.emplace_back(ArxLLVM::DI_FLOAT_TYPE);

  for (unsigned i = 0, e = NumArgs; i != e; ++i) {
    EltTys.emplace_back(ArxLLVM::DI_FLOAT_TYPE);
  }

  return ArxLLVM::di_builder->createSubroutineType(
    ArxLLVM::di_builder->getOrCreateTypeArray(EltTys));
}

// DebugInfo

auto ASTToLLVMIRVisitor::emitLocation(ExprAST& ast) -> void {
  if (!std::addressof(ast)) {
    return ArxLLVM::ir_builder->SetCurrentDebugLocation(llvm::DebugLoc());
  }

  llvm::DIScope* di_scope;
  if (this->llvm_di_lexical_blocks.empty()) {
    di_scope = llvm_di_compile_unit;
  } else {
    di_scope = this->llvm_di_lexical_blocks.back();
  }

  ArxLLVM::ir_builder->SetCurrentDebugLocation(llvm::DILocation::get(
    di_scope->getContext(), ast.get_line(), ast.getCol(), di_scope));
}

/**
 * @brief Code generation for FloatExprAST.
 *
 */
auto ASTToLLVMIRVisitor::visit(FloatExprAST& expr) -> void {
  this->emitLocation(expr);
  ASTToObjectVisitor::visit(expr);
}

/**
 * @brief Code generation for VariableExprAST.
 *
 */
auto ASTToLLVMIRVisitor::visit(VariableExprAST& expr) -> void {
  this->emitLocation(expr);
  ASTToObjectVisitor::visit(expr);
}

/**
 * @brief Code generation for UnaryExprAST.
 *
 */
auto ASTToLLVMIRVisitor::visit(UnaryExprAST& expr) -> void {
  this->emitLocation(expr);
  ASTToObjectVisitor::visit(expr);
}

/**
 * @brief Code generation for BinaryExprAST.
 *
 */
auto ASTToLLVMIRVisitor::visit(BinaryExprAST& expr) -> void {
  this->emitLocation(expr);
  ASTToObjectVisitor::visit(expr);
}

/**
 * @brief Code generation for CallExprAST.
 *
 */
auto ASTToLLVMIRVisitor::visit(CallExprAST& expr) -> void {
  this->emitLocation(expr);
  ASTToObjectVisitor::visit(expr);
}

/**
 * @brief Code generation for IfExprAST.
 */
auto ASTToLLVMIRVisitor::visit(IfExprAST& expr) -> void {
  this->emitLocation(expr);
  ASTToObjectVisitor::visit(expr);
}

/**
 * @brief Code generation for ForExprAST.
 *
 * @param expr A `for` expression.
 */
auto ASTToLLVMIRVisitor::visit(ForExprAST& expr) -> void {
  this->emitLocation(expr);
  ASTToObjectVisitor::visit(expr);
}

/**
 * @brief Code generation for VarExprAST.
 *
 */
auto ASTToLLVMIRVisitor::visit(VarExprAST& expr) -> void {
  this->emitLocation(expr);
  ASTToObjectVisitor::visit(expr);
}

/**
 * @brief Code generation for PrototypeExprAST.
 *
 */
auto ASTToLLVMIRVisitor::visit(PrototypeAST& expr) -> void {
  ASTToObjectVisitor::visit(expr);
}

/**
 * @brief Code generation for FunctionExprAST.
 *
 * Transfer ownership of the prototype to the ArxLLVM::function_protos map,
 * but keep a reference to it for use below.
 */
auto ASTToLLVMIRVisitor::visit(FunctionAST& expr) -> void {
  auto& proto = *(expr.proto);
  ArxLLVM::function_protos[expr.proto->get_name()] = std::move(expr.proto);
  this->getFunction(proto.get_name());
  llvm::Function* the_function = this->result_func;

  if (!the_function) {
    this->result_func = nullptr;
    return;
  }

  // Create a new basic block to start insertion into.
  // std::cout << "Create a new basic block to start insertion into";
  llvm::BasicBlock* basic_block =
    llvm::BasicBlock::Create(*ArxLLVM::context, "entry", the_function);
  ArxLLVM::ir_builder->SetInsertPoint(basic_block);

  /* debugging-code:start*/
  // Create a subprogram DIE for this function.
  llvm::DIFile* di_unit = ArxLLVM::di_builder->createFile(
    this->llvm_di_compile_unit->getFilename(),
    this->llvm_di_compile_unit->getDirectory());
  llvm::DIScope* file_context = di_unit;
  unsigned line_no = proto.get_line();
  unsigned ScopeLine = line_no;
  llvm::DISubprogram* di_subprogram = ArxLLVM::di_builder->createFunction(
    file_context,
    proto.get_name(),
    llvm::StringRef(),
    di_unit,
    line_no,
    CreateFunctionType(the_function->arg_size()),
    ScopeLine,
    llvm::DINode::FlagPrototyped,
    llvm::DISubprogram::SPFlagDefinition);
  the_function->setSubprogram(di_subprogram);

  // Push the current scope.
  this->llvm_di_lexical_blocks.emplace_back(di_subprogram);

  // Unset the location for the prologue emission (leading instructions with no
  // location in a function are considered part of the prologue and the
  // debugger will run past them when breaking on a function)
  ExprAST null_ast;
  this->emitLocation(null_ast);

  /* debugging-code:end*/

  // Record the function arguments in the named_values map.
  // std::cout << "Record the function arguments in the named_values map.";
  ArxLLVM::named_values.clear();

  unsigned arg_idx = 0;
  for (auto& llvm_arg : the_function->args()) {
    // Create an alloca for this variable.
    llvm::AllocaInst* alloca =
      CreateEntryBlockAlloca(the_function, llvm_arg.getName());

    /* debugging-code: start */
    // Create a debug descriptor for the variable.
    llvm::DILocalVariable* di_local_variable =
      ArxLLVM::di_builder->createParameterVariable(
        di_subprogram,
        llvm_arg.getName(),
        ++arg_idx,
        di_unit,
        line_no,
        ArxLLVM::DI_FLOAT_TYPE,
        true);

    ArxLLVM::di_builder->insertDeclare(
      alloca,
      di_local_variable,
      ArxLLVM::di_builder->createExpression(),
      llvm::DILocation::get(
        di_subprogram->getContext(), line_no, 0, di_subprogram),
      ArxLLVM::ir_builder->GetInsertBlock());

    /* debugging-code-end */

    // Store the initial value into the alloca.
    ArxLLVM::ir_builder->CreateStore(&llvm_arg, alloca);

    // Add arguments to variable symbol table.
    ArxLLVM::named_values[std::string(llvm_arg.getName())] = alloca;
  }

  this->emitLocation(*expr.body.get());

  expr.body->accept(*this);
  llvm::Value* llvm_return_val = this->result_val;

  if (llvm_return_val) {
    // Finish off the function.
    ArxLLVM::ir_builder->CreateRet(llvm_return_val);

    // Pop off the lexical block for the function.
    this->llvm_di_lexical_blocks.pop_back();

    // Validate the generated code, checking for consistency.
    llvm::verifyFunction(*the_function);

    this->result_func = the_function;
    return;
  }

  // Error reading body, remove function.
  the_function->eraseFromParent();

  this->result_func = nullptr;

  // Pop off the lexical block for the function since we added it
  // unconditionally.
  this->llvm_di_lexical_blocks.pop_back();
}

/**
 * @brief initialize LLVM Module And PassManager.
 *
 */
auto ASTToLLVMIRVisitor::initialize() -> void {
  ArxLLVM::initialize();
}

/**
 * @brief Compile an AST to object file.
 *
 * @param ast The AST tree object.
 */
auto compile_llvm_ir(TreeAST& ast) -> int {
  auto codegen = std::make_unique<ASTToLLVMIRVisitor>(ASTToLLVMIRVisitor());

  Lexer::get_next_token();

  codegen->initialize();

  // Run the main "interpreter loop" now.
  LOG(INFO) << "Starting main_loop";

  // Create the compile unit for the module.
  // Currently down as "fib.ks" as a filename since we're redirecting stdin
  // but we'd like actual source locations.
  codegen->llvm_di_compile_unit = ArxLLVM::di_builder->createCompileUnit(
    llvm::dwarf::DW_LANG_C,
    ArxLLVM::di_builder->createFile("fib.ks", "."),
    "Arx Compiler",
    false,
    "",
    0);

  LOG(INFO) << "initialize Target";

  // Add the current debug info version into the module.
  ArxLLVM::module->addModuleFlag(
    llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);

  // Darwin only supports dwarf2.
  if (llvm::Triple(llvm::sys::getProcessTriple()).isOSDarwin()) {
    ArxLLVM::module->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);
  }

  // Construct the DIBuilder, we do this here because we need the module.
  ArxLLVM::di_builder = std::make_unique<llvm::DIBuilder>(*ArxLLVM::module);

  // Create the compile unit for the module.
  // Currently down as "fib.ks" as a filename since we're redirecting stdin
  // but we'd like actual source locations.
  codegen->llvm_di_compile_unit = ArxLLVM::di_builder->createCompileUnit(
    llvm::dwarf::DW_LANG_C,
    ArxLLVM::di_builder->createFile("fib.arxks", "."),
    "Arx Compiler",
    false,
    "",
    0);

  // Run the main "interpreter loop" now.
  codegen->main_loop(ast);

  // Finalize the debug info.
  ArxLLVM::di_builder->finalize();

  // Print out all of the generated code.
  ArxLLVM::module->print(llvm::errs(), nullptr);

  return 0;
}

/**
 * @brief Open the Arx shell.
 *
 */
auto open_shell_llvm_ir() -> int {
  // Prime the first token.
  fprintf(stderr, "Arx %s \n", ARX_VERSION.c_str());
  fprintf(stderr, ">>> ");

  auto ast = std::make_unique<TreeAST>(TreeAST());
  return compile_llvm_ir(*ast);
}
