// note: arrow will not be used yet
// #include <arrow/api.h>
// #include <arrow/csv/api.h>
// #include <arrow/io/api.h>
// #include <arrow/ipc/api.h>
// #include <arrow/pretty_print.h>
// #include <arrow/result.h>
// #include <arrow/status.h>
// #include <arrow/table.h>

#include <glog/logging.h>  // for InitGoogleLogging
#include <CLI/CLI.hpp>
#include <cstdlib>  // for exit
#include <iostream>
#include <string>                    // for string, allocator
#include "codegen/arx-llvm.h"        // for ArxLLVM
#include "codegen/ast-to-llvm-ir.h"  // for compile_llvm_ir
#include "codegen/ast-to-object.h"   // for compile_object, open_shell_object
#include "codegen/ast-to-stdout.h"   // for print_ast
#include "io.h"                      // for load_input_to_buffer
#include "parser.h"                  // for Parser, TreeAST (ptr only)
#include "utils.h"                   // for show_version

std::string ARX_VERSION = "1.6.0";  // semantic-release
extern std::string INPUT_FILE;
extern std::string OUTPUT_FILE;
extern bool INPUT_FROM_STDIN;
extern bool IS_BUILD_LIB;

/**
 * @brief Open the Arx shell.
 * @param count An internal value from CLI11.
 *
 */
auto main_open_shell() -> int {
  INPUT_FROM_STDIN = true;
  return open_shell_object();
}

/**
 * @brief Show the Arx version number.
 * @param count An internal value from CLI11.
 *
 */
auto main_show_version() -> int {
  load_input_to_buffer();
  return show_version();
}

/**
 * @brief Show the AST for the given source.
 * @param count An internal value from CLI11.
 */
auto main_show_ast() -> int {
  load_input_to_buffer();
  auto ast = Parser::parse();
  return print_ast(*ast);
}

/**
 * @brief Show the LLVM IR for the given source.
 * @param count An internal value from CLI11.
 */
auto main_show_llvm_ir() -> int {
  load_input_to_buffer();
  auto ast = Parser::parse();
  return compile_llvm_ir(*ast);
}

/**
 * @brief Compile the given source code.
 *
 */
auto main_compile() -> int {
  load_input_to_buffer();
  auto ast = Parser::parse();
  return compile_object(*ast);
}

/**
 * @brief The main function.
 * @param argc used by CLI11.
 * @param argv used by CLI11.
 * @return exit code.
 *
 */
auto main(int argc, const char* argv[]) -> int {
  bool is_open_shell = false;
  bool is_show_ast = false;
  bool is_show_llvm_ir = false;
  bool is_show_version = false;

  google::InitGoogleLogging(argv[0]);

  Parser::setup();

  CLI::App app{"ArxLang"};

  // note: it is possible to call a function directly through `add_flag`
  //       but here we are doing it manually in order to have full control
  //       over the workflow.
  app.add_option("--input", INPUT_FILE, "Input file.");
  app.add_option("--output", OUTPUT_FILE, "Output file.");
  app.add_flag("--shell", is_open_shell, "Open Arx Shell.");
  app.add_flag("--show-ast", is_show_ast, "Show AST from source.");
  app.add_flag("--show-llvm-ir", is_show_llvm_ir, "Show LLVM IR from source.");
  app.add_flag("--version", is_show_version, "Show ArxLang version.");
  app.add_flag(
    "--build-lib",
    IS_BUILD_LIB,
    "Default False. When False it creates a program instead");

  CLI11_PARSE(app, argc, argv);

  if (is_open_shell) {
    return main_open_shell();
  }
  if (is_show_ast) {
    return main_show_ast();
  }
  if (is_show_llvm_ir) {
    return main_show_llvm_ir();
  }
  if (is_show_version) {
    return main_show_version();
  }

  return main_compile();
}
