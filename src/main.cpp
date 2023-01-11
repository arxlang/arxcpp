// note: arrow will not be used yet
// #include <arrow/api.h>
// #include <arrow/csv/api.h>
// #include <arrow/io/api.h>
// #include <arrow/ipc/api.h>
// #include <arrow/pretty_print.h>
// #include <arrow/result.h>
// #include <arrow/status.h>
// #include <arrow/table.h>

#include <cctype>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include <glog/logging.h>

// #include "codegen/ast-to-llvm.h"
#include "codegen/ast-to-object.h"
#include "codegen/ast-to-stdout.h"

#include "io.h"
#include "parser.h"
#include "utils.h"

std::string ARX_VERSION = "1.5.0";  // semantic-release
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
  open_shell();
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
  TreeAST* ast = Parser::parse();
  print_ast(ast);
  exit(0);
}

/**
 * @brief Compile the given source code.
 *
 */
auto main_compile() -> void {
  load_input_to_buffer();
  TreeAST* ast = Parser::parse();
  compile(ast);
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
  app.add_flag("--version", main_show_version, "Show ArxLang version.");

  CLI11_PARSE(app, argc, argv);

  main_compile();

  return 0;
}
