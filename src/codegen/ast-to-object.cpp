#include <cassert>  // for assert
#include <cstdio>   // for fprintf, stderr, fputc
#include <cstdlib>  // for exit
#include <iostream>
#include <map>           // for map, operator==, _Rb_tree_ite...
#include <memory>        // for unique_ptr, allocator, make_u...
#include <string>        // for string, operator<=>, operator+
#include <system_error>  // for error_code
#include <utility>       // for pair, move
#include <vector>        // for vector

#include <glog/logging.h>               // for COMPACT_GOOGLE_LOG_INFO, LOG
#include <llvm/ADT/APFloat.h>           // for APFloat
#include <llvm/ADT/iterator_range.h>    // for iterator_range
#include <llvm/ADT/Optional.h>          // for Optional
#include <llvm/ADT/StringRef.h>         // for StringRef
#include <llvm/ADT/Twine.h>             // for Twine
#include <llvm/IR/Argument.h>           // for Argument
#include <llvm/IR/BasicBlock.h>         // for BasicBlock
#include <llvm/IR/Constant.h>           // for Constant
#include <llvm/IR/Constants.h>          // for ConstantFP
#include <llvm/IR/DerivedTypes.h>       // for FunctionType
#include <llvm/IR/Function.h>           // for Function
#include <llvm/IR/Instructions.h>       // for AllocaInst, CallInst, PHINode
#include <llvm/IR/IRBuilder.h>          // for IRBuilder
#include <llvm/IR/LegacyPassManager.h>  // for PassManager
#include <llvm/IR/LLVMContext.h>        // for LLVMContext
#include <llvm/IR/Module.h>             // for Module
#include <llvm/IR/Type.h>               // for Type
#include <llvm/IR/Verifier.h>           // for verifyFunction
#include <llvm/MC/TargetRegistry.h>     // for Target, TargetRegistry
#include <llvm/Support/CodeGen.h>       // for CodeGenFileType, Model
#include <llvm/Support/FileSystem.h>    // for OpenFlags
#include <llvm/Support/Host.h>          // for getDefaultTargetTriple
#include <llvm/Support/raw_ostream.h>   // for errs, raw_fd_ostream, raw_ost...
#include <llvm/Support/TargetSelect.h>  // for InitializeAllAsmParsers, Init...
#include <llvm/Target/TargetMachine.h>  // for TargetMachine
#include <llvm/Target/TargetOptions.h>  // for TargetOptions

#include "codegen/arx-llvm.h"       // for ArxLLVM
#include "codegen/ast-to-object.h"  // for ASTToObjectVisitor, compile_o...
#include "error.h"                  // for LogErrorV
#include "io.h"                     // for ArxFile
#include "lexer.h"                  // for Lexer
#include "parser.h"                 // for PrototypeAST, ExprAST, ForExp...

namespace llvm {
  class Value;
}

std::string string_join(
  const std::vector<std::string>& elements, const std::string& delimiter) {
  if (elements.empty()) {
    return "";
  }

  std::string str;
  for (auto v : elements) {
    str += v + delimiter;
  }
  str = str.substr(0, str.size() - delimiter.size());
  return str;
}

extern std::string INPUT_FILE;
extern std::string OUTPUT_FILE;
extern std::string ARX_VERSION;

/**
 * @brief Put the function defined by the given name to result_func.
 * @param name Function name
 *
 * First, see if the function has already been added to the current
 * module. If not, check whether we can codegen the declaration from some
 * existing prototype. If no existing prototype exists, return null.
 */
auto ASTToObjectVisitor::getFunction(std::string name) -> void {
  if (auto* fn = ArxLLVM::module->getFunction(name)) {
    this->result_func = fn;
    return;
  }

  auto FI = ArxLLVM::function_protos.find(name);
  if (FI != ArxLLVM::function_protos.end()) {
    FI->second->accept(*this);
  };
}

/**
 * @brief Create the Entry Block Allocation.
 * @param fn The llvm function
 * @param var_name The variable name
 * @return An llvm allocation instance.
 *
 * create_entry_block_alloca - Create an alloca instruction in the entry
 * block of the function.  This is used for mutable variables etc.
 */
auto ASTToObjectVisitor::create_entry_block_alloca(
  llvm::Function* fn, llvm::StringRef var_name, std::string type_name)
  -> llvm::AllocaInst* {
  llvm::IRBuilder<> tmp_builder(
    &fn->getEntryBlock(), fn->getEntryBlock().begin());
  return tmp_builder.CreateAlloca(
    ArxLLVM::get_data_type(type_name), nullptr, var_name);
}

/**
 * @brief Set to nullptr result_val and result_func in order to avoid trash.
 *
 */
auto ASTToObjectVisitor::clean() -> void {
  this->result_val = nullptr;
  this->result_func = nullptr;
}

/**
 * @brief Code generation for FloatExprAST.
 *
 */
auto ASTToObjectVisitor::visit(FloatExprAST& expr) -> void {
  this->result_val =
    llvm::ConstantFP::get(*ArxLLVM::context, llvm::APFloat(expr.val));
}

/**
 * @brief Code generation for VariableExprAST.
 *
 */
auto ASTToObjectVisitor::visit(VariableExprAST& expr) -> void {
  llvm::Value* expr_var = ArxLLVM::named_values[expr.name];

  if (!expr_var) {
    auto msg = std::string("Unknown variable name: ") + expr.name;
    this->result_val = LogErrorV(msg.c_str());
    return;
  }

  this->result_val = ArxLLVM::ir_builder->CreateLoad(
    ArxLLVM::FLOAT_TYPE, expr_var, expr.name.c_str());
}

/**
 * @brief Code generation for UnaryExprAST.
 *
 */
auto ASTToObjectVisitor::visit(UnaryExprAST& expr) -> void {
  expr.operand.get()->accept(*this);
  llvm::Value* operand_value = this->result_val;

  if (!operand_value) {
    this->result_val = nullptr;
    return;
  }

  this->getFunction(std::string("unary") + expr.op_code);
  llvm::Function* fn = this->result_func;
  if (!fn) {
    this->result_val = LogErrorV("Unknown unary operator");
    return;
  }

  this->result_val =
    ArxLLVM::ir_builder->CreateCall(fn, operand_value, "unop");
}

/**
 * @brief Code generation for BinaryExprAST.
 *
 */
auto ASTToObjectVisitor::visit(BinaryExprAST& expr) -> void {
  // Special case '=' because we don't want to emit the lhs as an
  // expression.*/
  if (expr.op == '=') {
    // Assignment requires the lhs to be an identifier.
    // This assume we're building without RTTI because LLVM builds that
    // way by default.  If you build LLVM with RTTI this can be changed
    // to a dynamic_cast for automatic error checking.
    VariableExprAST* var_lhs = static_cast<VariableExprAST*>(expr.lhs.get());
    if (!var_lhs) {
      this->result_val = LogErrorV("destination of '=' must be a variable");
      return;
    }

    // Codegen the rhs.//
    expr.rhs.get()->accept(*this);
    llvm::Value* val = this->result_val;

    if (!val) {
      this->result_val = nullptr;
      return;
    };

    // Look up the name.//
    llvm::Value* variable = ArxLLVM::named_values[var_lhs->get_name()];
    if (!variable) {
      this->result_val = LogErrorV("Unknown variable name");
      return;
    }

    ArxLLVM::ir_builder->CreateStore(val, variable);
    this->result_val = val;
  }

  expr.lhs.get()->accept(*this);
  llvm::Value* llvm_val_lhs = this->result_val;
  expr.rhs.get()->accept(*this);
  llvm::Value* llvm_val_rhs = this->result_val;

  if (!llvm_val_lhs || !llvm_val_rhs) {
    this->result_val = nullptr;
    return;
  }

  switch (expr.op) {
    case '+':
      this->result_val =
        ArxLLVM::ir_builder->CreateFAdd(llvm_val_lhs, llvm_val_rhs, "addtmp");
      return;
    case '-':
      this->result_val =
        ArxLLVM::ir_builder->CreateFSub(llvm_val_lhs, llvm_val_rhs, "subtmp");
      return;
    case '*':
      this->result_val =
        ArxLLVM::ir_builder->CreateFMul(llvm_val_lhs, llvm_val_rhs, "multmp");
      return;
    case '<':
      llvm_val_lhs = ArxLLVM::ir_builder->CreateFCmpULT(
        llvm_val_lhs, llvm_val_rhs, "cmptmp");
      // Convert bool 0/1 to float 0.0 or 1.0 //
      this->result_val = ArxLLVM::ir_builder->CreateUIToFP(
        llvm_val_lhs, ArxLLVM::FLOAT_TYPE, "booltmp");
      return;
  }

  // If it wasn't a builtin binary operator, it must be a user defined
  // one. Emit a call to it.
  this->getFunction(std::string("binary") + expr.op);
  llvm::Function* fn = this->result_func;
  assert(fn && "binary operator not found!");

  llvm::Value* Ops[] = {llvm_val_lhs, llvm_val_rhs};
  this->result_val = ArxLLVM::ir_builder->CreateCall(fn, Ops, "binop");
}

/**
 * @brief Code generation for CallExprAST.
 *
 */
auto ASTToObjectVisitor::visit(CallExprAST& expr) -> void {
  this->getFunction(expr.callee);
  llvm::Function* CalleeF = this->result_func;
  if (!CalleeF) {
    this->result_val = LogErrorV("Unknown function referenced");
    return;
  }

  if (CalleeF->arg_size() != expr.args.size()) {
    this->result_val = LogErrorV("Incorrect # arguments passed");
    return;
  }

  std::vector<llvm::Value*> ArgsV;
  for (unsigned i = 0, e = expr.args.size(); i != e; ++i) {
    expr.args[i].get()->accept(*this);
    llvm::Value* ArgsV_item = this->result_val;
    ArgsV.push_back(ArgsV_item);
    if (!ArgsV.back()) {
      this->result_val = nullptr;
      return;
    }
  }

  this->result_val =
    ArxLLVM::ir_builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

/**
 * @brief Code generation for IfExprAST.
 */
auto ASTToObjectVisitor::visit(IfExprAST& expr) -> void {
  expr.cond.get()->accept(*this);
  llvm::Value* CondV = this->result_val;

  if (!CondV) {
    this->result_val = nullptr;
    return;
  }

  // Convert condition to a bool by comparing non-equal to 0.0.
  CondV = ArxLLVM::ir_builder->CreateFCmpONE(
    CondV,
    llvm::ConstantFP::get(*ArxLLVM::context, llvm::APFloat(0.0)),
    "ifcond");

  llvm::Function* fn = ArxLLVM::ir_builder->GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block
  // at the end of the function.
  llvm::BasicBlock* ThenBB =
    llvm::BasicBlock::Create(*ArxLLVM::context, "then", fn);
  llvm::BasicBlock* ElseBB =
    llvm::BasicBlock::Create(*ArxLLVM::context, "else");
  llvm::BasicBlock* MergeBB =
    llvm::BasicBlock::Create(*ArxLLVM::context, "ifcont");

  ArxLLVM::ir_builder->CreateCondBr(CondV, ThenBB, ElseBB);

  // Emit then value.
  ArxLLVM::ir_builder->SetInsertPoint(ThenBB);

  expr.then.get()->accept(*this);
  llvm::Value* ThenV = this->result_val;
  if (!ThenV) {
    this->result_val = nullptr;
    return;
  }

  ArxLLVM::ir_builder->CreateBr(MergeBB);
  // Codegen of 'then' can change the current block, update ThenBB for
  // the PHI.
  ThenBB = ArxLLVM::ir_builder->GetInsertBlock();

  // Emit else block.
  fn->getBasicBlockList().push_back(ElseBB);
  ArxLLVM::ir_builder->SetInsertPoint(ElseBB);

  expr.else_.get()->accept(*this);
  llvm::Value* ElseV = this->result_val;
  if (!ElseV) {
    this->result_val = nullptr;
    return;
  }

  ArxLLVM::ir_builder->CreateBr(MergeBB);
  // Codegen of 'else_' can change the current block, update ElseBB for
  // the PHI.
  ElseBB = ArxLLVM::ir_builder->GetInsertBlock();

  // Emit merge block.
  fn->getBasicBlockList().push_back(MergeBB);
  ArxLLVM::ir_builder->SetInsertPoint(MergeBB);
  llvm::PHINode* PN =
    ArxLLVM::ir_builder->CreatePHI(ArxLLVM::FLOAT_TYPE, 2, "iftmp");

  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);

  this->result_val = PN;
  return;
}

/**
 * @brief Code generation for ForExprAST.
 *
 * @param expr A `for` expression.
 */
auto ASTToObjectVisitor::visit(ForExprAST& expr) -> void {
  llvm::Function* fn = ArxLLVM::ir_builder->GetInsertBlock()->getParent();

  // Create an alloca for the variable in the entry block.
  // TODO: maybe it would be safe to change it to void
  llvm::AllocaInst* alloca =
    this->create_entry_block_alloca(fn, expr.var_name, "float");

  // Emit the start code first, without 'variable' in scope.
  expr.start.get()->accept(*this);
  llvm::Value* StartVal = this->result_val;
  if (!StartVal) {
    this->result_val = nullptr;
    return;
  }

  // Store the value into the alloca.
  ArxLLVM::ir_builder->CreateStore(StartVal, alloca);

  // Make the new basic block for the loop header, inserting after
  // current block.
  llvm::BasicBlock* LoopBB =
    llvm::BasicBlock::Create(*ArxLLVM::context, "loop", fn);

  // Insert an explicit fall through from the current block to the
  // LoopBB.
  ArxLLVM::ir_builder->CreateBr(LoopBB);

  // start insertion in LoopBB.
  ArxLLVM::ir_builder->SetInsertPoint(LoopBB);

  // Within the loop, the variable is defined equal to the PHI node.  If
  // it shadows an existing variable, we have to restore it, so save it
  // now.
  llvm::AllocaInst* OldVal = ArxLLVM::named_values[expr.var_name];
  ArxLLVM::named_values[expr.var_name] = alloca;

  // Emit the body of the loop.  This, like any other expr, can change
  // the current basic_block.  Note that we ignore the value computed by the
  // body, but don't allow an error.
  expr.body.get()->accept(*this);
  llvm::Value* body_val = this->result_val;

  if (!body_val) {
    this->result_val = nullptr;
    return;
  }

  // Emit the step value.
  llvm::Value* StepVal = nullptr;
  if (expr.step) {
    expr.step.get()->accept(*this);
    StepVal = this->result_val;
    if (!StepVal) {
      this->result_val = nullptr;
      return;
    }
  } else {
    // If not specified, use 1.0.
    StepVal = llvm::ConstantFP::get(*ArxLLVM::context, llvm::APFloat(1.0));
  }

  // Compute the end condition.
  expr.end.get()->accept(*this);
  llvm::Value* EndCond = this->result_val;
  if (!EndCond) {
    this->result_val = nullptr;
    return;
  }

  // Reload, increment, and restore the alloca.  This handles the case
  // where the body of the loop mutates the variable.
  llvm::Value* CurVar = ArxLLVM::ir_builder->CreateLoad(
    ArxLLVM::FLOAT_TYPE, alloca, expr.var_name.c_str());
  llvm::Value* NextVar =
    ArxLLVM::ir_builder->CreateFAdd(CurVar, StepVal, "nextvar");
  ArxLLVM::ir_builder->CreateStore(NextVar, alloca);

  // Convert condition to a bool by comparing non-equal to 0.0.
  EndCond = ArxLLVM::ir_builder->CreateFCmpONE(
    EndCond,
    llvm::ConstantFP::get(*ArxLLVM::context, llvm::APFloat(0.0)),
    "loopcond");

  // Create the "after loop" block and insert it.
  llvm::BasicBlock* AfterBB =
    llvm::BasicBlock::Create(*ArxLLVM::context, "afterloop", fn);

  // Insert the conditional branch into the end of LoopEndBB.
  ArxLLVM::ir_builder->CreateCondBr(EndCond, LoopBB, AfterBB);

  // Any new code will be inserted in AfterBB.
  ArxLLVM::ir_builder->SetInsertPoint(AfterBB);

  // Restore the unshadowed variable.
  if (OldVal) {
    ArxLLVM::named_values[expr.var_name] = OldVal;
  } else {
    ArxLLVM::named_values.erase(expr.var_name);
  }

  // for expr always returns 0.0.
  this->result_val = llvm::Constant::getNullValue(ArxLLVM::FLOAT_TYPE);
}

/**
 * @brief Code generation for VarExprAST.
 *
 */
auto ASTToObjectVisitor::visit(VarExprAST& expr) -> void {
  std::vector<llvm::AllocaInst*> old_bindings;

  llvm::Function* fn = ArxLLVM::ir_builder->GetInsertBlock()->getParent();

  // Register all variables and emit their initializer.
  for (auto& i : expr.var_names) {
    const std::string& var_name = i.first;
    ExprAST* Init = i.second.get();

    // Emit the initializer before adding the variable to scope, this
    // prevents the initializer from referencing the variable itself, and
    // permits stuff like this:
    //  var a = 1 in
    //    var a = a in ...   # refers to outer 'a'.

    llvm::Value* InitVal = nullptr;
    if (Init) {
      Init->accept(*this);
      InitVal = this->result_val;
      if (!InitVal) {
        this->result_val = nullptr;
        return;
      }
    } else {  // If not specified, use 0.0.
      InitVal = llvm::ConstantFP::get(*ArxLLVM::context, llvm::APFloat(0.0));
    }

    // TODO: replace "float" for the actual type_name from the argument
    llvm::AllocaInst* alloca =
      create_entry_block_alloca(fn, var_name, "float");
    ArxLLVM::ir_builder->CreateStore(InitVal, alloca);

    // Remember the old variable binding so that we can restore the
    // binding when we unrecurse.
    old_bindings.push_back(ArxLLVM::named_values[var_name]);

    // Remember this binding.
    ArxLLVM::named_values[var_name] = alloca;
  }

  // Codegen the body, now that all vars are in scope.
  expr.body.get()->accept(*this);
  llvm::Value* body_val = this->result_val;
  if (!body_val) {
    this->result_val = nullptr;
    return;
  }

  // Pop all our variables from scope.
  for (unsigned i = 0, e = expr.var_names.size(); i != e; ++i) {
    ArxLLVM::named_values[expr.var_names[i].first] = old_bindings[i];
  }

  // Return the body computation.
  this->result_val = body_val;
}

/**
 * @brief Code generation for PrototypeExprAST.
 *
 */
auto ASTToObjectVisitor::visit(PrototypeAST& expr) -> void {
  std::vector<llvm::Type*> args_type(expr.args.size(), ArxLLVM::FLOAT_TYPE);
  llvm::Type* return_type = ArxLLVM::get_data_type("float");

  llvm::FunctionType* fn_type =
    llvm::FunctionType::get(return_type, args_type, false /* isVarArg */);

  llvm::Function* fn = llvm::Function::Create(
    fn_type,
    llvm::Function::ExternalLinkage,
    expr.name,
    ArxLLVM::module.get());

  // Set names for all arguments.
  unsigned idx = 0;
  for (auto& arg : fn->args()) {
    arg.setName(expr.args[idx++]->name);
  }

  this->result_func = fn;
}

/**
 * @brief Code generation for FunctionExprAST.
 *
 * Transfer ownership of the prototype to the ArxLLVM::function_protos map,
 * but keep a reference to it for use below.
 */
auto ASTToObjectVisitor::visit(FunctionAST& expr) -> void {
  auto& proto = *(expr.proto);
  ArxLLVM::function_protos[expr.proto->get_name()] = std::move(expr.proto);
  this->getFunction(proto.get_name());
  llvm::Function* fn = this->result_func;

  if (!fn) {
    this->result_func = nullptr;
    return;
  }

  // Create a new basic block to start insertion into.
  // std::cout << "Create a new basic block to start insertion into";
  llvm::BasicBlock* basic_block =
    llvm::BasicBlock::Create(*ArxLLVM::context, "entry", fn);
  ArxLLVM::ir_builder->SetInsertPoint(basic_block);

  // Record the function arguments in the named_values map.
  // std::cout << "Record the function arguments in the named_values map.";
  ArxLLVM::named_values.clear();

  for (auto& llvm_arg : fn->args()) {
    // Create an alloca for this variable.
    // TODO: replace "float" for the actual type_name from the argument
    llvm::AllocaInst* alloca =
      this->create_entry_block_alloca(fn, llvm_arg.getName(), "float");

    // Store the initial value into the alloca.
    ArxLLVM::ir_builder->CreateStore(&llvm_arg, alloca);

    // Add arguments to variable symbol table.
    ArxLLVM::named_values[std::string(llvm_arg.getName())] = alloca;
  }

  expr.body->accept(*this);
  llvm::Value* llvm_return_val = this->result_val;

  if (llvm_return_val) {
    // Finish off the function.
    ArxLLVM::ir_builder->CreateRet(llvm_return_val);

    // Validate the generated code, checking for consistency.
    llvm::verifyFunction(*fn);

    this->result_func = fn;
    return;
  }

  // Error reading body, remove function.
  fn->eraseFromParent();

  this->result_func = nullptr;
}

/**
 * @brief initialize LLVM Module And PassManager.
 *
 */
auto ASTToObjectVisitor::initialize() -> void {
  ArxLLVM::initialize();
}

/**
 * @brief The main loop that walks the AST.
 * top ::= definition | external | expression | ';'
 */
auto ASTToObjectVisitor::main_loop(TreeAST& ast) -> void {
  for (auto& node : ast.nodes) {
    node->accept(*this);
  }
}

//===----------------------------------------------------------------------===
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/**
 * @brief putchar that takes a float and returns 0.
 *
 */
extern "C" DLLEXPORT auto putchard(float X) -> float {
  fputc(static_cast<char>(X), stderr);
  return 0;
}

/**
 * @brief printf that takes a float prints it as "%f\n", returning 0.
 *
 */
extern "C" DLLEXPORT auto printd(float X) -> float {
  fprintf(stderr, "%f\n", X);
  return 0;
}

/**
 * @brief Compile an AST to object file.
 *
 * @param tree_ast The AST tree object.
 */
auto compile_object(TreeAST& tree_ast) -> int {
  auto codegen = std::make_unique<ASTToObjectVisitor>(ASTToObjectVisitor());

  Lexer::get_next_token();

  codegen->initialize();

  // Run the main "interpreter loop" now.
  LOG(INFO) << "Starting main_loop";

  codegen->main_loop(tree_ast);

  LOG(INFO) << "target_triple";

  auto target_triple = llvm::sys::getDefaultTargetTriple();
  ArxLLVM::module->setTargetTriple(target_triple);

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(target_triple, Error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (!Target) {
    llvm::errs() << Error;
    return 1;
  }

  auto CPU = "generic";
  auto Features = "";

  LOG(INFO) << "Target Options";

  llvm::TargetOptions opt;
  auto reloc_model = llvm::Optional<llvm::Reloc::Model>();

  LOG(INFO) << "Target Machine";
  auto the_target_machine = Target->createTargetMachine(
    target_triple, CPU, Features, opt, reloc_model);

  LOG(INFO) << "Set Data Layout";

  ArxLLVM::module->setDataLayout(the_target_machine->createDataLayout());

  LOG(INFO) << "dest output";
  std::error_code error_code;

  if (OUTPUT_FILE == "") {
    OUTPUT_FILE = INPUT_FILE + ".o";
  }

  llvm::raw_fd_ostream dest(OUTPUT_FILE, error_code, llvm::sys::fs::OF_None);

  if (error_code) {
    llvm::errs() << "Could not open file: " << error_code.message();
    delete the_target_machine;
    return 1;
  }

  llvm::legacy::PassManager pass;

  auto file_type = llvm::CGFT_ObjectFile;

  if (the_target_machine->addPassesToEmitFile(
        pass, dest, nullptr, file_type)) {
    llvm::errs() << "the_target_machine can't emit a file of this type";
    delete the_target_machine;
    return 1;
  }

  pass.run(*ArxLLVM::module);
  dest.flush();

  delete the_target_machine;

  if (IS_BUILD_LIB) {
    return 0;
  }

  // generate an executable file

  std::string linker_path = "clang++";
  std::string executable_path = INPUT_FILE + "c";
  // note: it just have a purpose to demonstrate an initial implementation
  //       it will be improved in a follow-up PR
  std::string content =
    "#include <iostream>\n"
    "int main() {\n"
    "  std::cout << \"ARX[WARNING]: "
    "This is an empty executable file\" << std::endl;\n"
    "}\n";

  std::string main_cpp_path = ArxFile::create_tmp_file(content);

  if (main_cpp_path == "") {
    llvm::errs() << "ARX[FAIL]: Executable file was not created.";
    return 1;
  }

  /* Example (running it from a shell prompt):
     clang++ \
       ${CLANG_EXTRAS} \
       ${DEBUG_FLAGS} \
       -fPIC \
       -std=c++20 \
       "${TEST_DIR_PATH}/integration/${test_name}.cpp" \
       ${OBJECT_FILE} \
       -o "${TMP_DIR}/main"
  */

  std::vector<std::string> compiler_args{
    "-fPIC", "-std=c++20", main_cpp_path, OUTPUT_FILE, "-o", executable_path};

  // Add any additional compiler flags or include paths as needed
  // compiler_args.push_back("-I/path/to/include");

  std::string compiler_cmd =
    linker_path + " " + string_join(compiler_args, " ");

  std::cout << "ARX[INFO]: " << compiler_cmd << std::endl;
  int compile_result = system(compiler_cmd.c_str());

  ArxFile::delete_file(main_cpp_path);

  if (compile_result != 0) {
    llvm::errs() << "failed to compile and link object file";
    exit(1);
  }

  return 0;
}

/**
 * @brief Open the Arx shell.
 *
 */
auto open_shell_object() -> int {
  // Prime the first token.
  fprintf(stderr, "Arx %s \n", ARX_VERSION.c_str());
  fprintf(stderr, ">>> ");

  auto ast = std::make_unique<TreeAST>(TreeAST());

  return compile_object(*ast);
}
