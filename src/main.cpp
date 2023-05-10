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
#include <stdlib.h>        // for exit
#include <CLI/CLI.hpp>
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

/**
 * @brief Open the Arx shell.
 * @param count An internal value from CLI11.
 *
 */
auto main_open_shell(__attribute__((unused)) int count) -> void {
  INPUT_FROM_STDIN = true;
  open_shell_object();
  exit(0);
}

/**
 * @brief Show the Arx version number.
 * @param count An internal value from CLI11.
 *
 */
auto main_show_version(__attribute__((unused)) int count) -> void {
  load_input_to_buffer();
  show_version();
  exit(0);
}

/**
 * @brief Show the AST for the given source.
 * @param count An internal value from CLI11.
 */
auto main_show_ast(__attribute__((unused)) int count) -> void {
  load_input_to_buffer();
  print_ast(std::move(Parser::parse()));
  exit(0);
}

/**
 * @brief Show the LLVM IR for the given source.
 * @param count An internal value from CLI11.
 */
auto main_show_llvm_ir(__attribute__((unused)) int count) -> void {
  load_input_to_buffer();
  compile_llvm_ir(std::move(Parser::parse()));
  exit(0);
}

/**
 * @brief Compile the given source code.
 *
 */
auto main_compile() -> void {
  load_input_to_buffer();
  compile_object(std::move(Parser::parse()));
  exit(0);
}

/**
 * @brief The main function.
 * @param argc used by CLI11.
 * @param argv used by CLI11.
 * @return exit code.
 *
 */
auto main(int argc, const char* argv[]) -> int {
  google::InitGoogleLogging(argv[0]);

  Parser::setup();

  CLI::App app{"ArxLang"};

  app.add_option("--input", INPUT_FILE, "Input file.");
  app.add_option("--output", OUTPUT_FILE, "Output file.");
  app.add_flag("--shell", main_open_shell, "Open Arx Shell.");
  app.add_flag("--show-ast", main_show_ast, "Show AST from source.");
  app.add_flag(
    "--show-llvm-ir", main_show_llvm_ir, "Show LLVM IR from source.");
  app.add_flag("--version", main_show_version, "Show ArxLang version.");

  CLI11_PARSE(app, argc, argv);

  main_compile();

  return 0;
}
