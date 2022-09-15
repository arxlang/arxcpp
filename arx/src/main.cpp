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

#include "codegen.h"
#include "io.h"
#include "lexer.h"
#include "settings.h"
#include "utils.h"

std::string ARX_VERSION = "1.4.1";  // semantic-release
extern std::string INPUT_FILE;
extern std::string OUTPUT_FILE;

auto main_open_shell(int count) {
  load_input_to_buffer();
  open_shell();
  exit(0);
}

auto main_show_llvm(int count) {
  load_input_to_buffer();
  show_llvm();
  exit(0);
}

auto main_show_version(int count) {
  load_input_to_buffer();
  show_version();
  exit(0);
}

auto main_compile() {
  load_input_to_buffer();
  show_version();
  exit(0);
}

auto main(int argc, const char* argv[]) -> int {
  google::InitGoogleLogging(argv[0]);

  CLI::App app{"ArxLang"};

  load_settings();

  app.add_option("--input", INPUT_FILE, "Input file.");
  app.add_option("--output", OUTPUT_FILE, "Output file.");
  app.add_flag("--shell", main_open_shell, "Open Arx Shell.");
  app.add_flag("--show-llvm", main_show_llvm, "Show LLVM IR.");
  app.add_flag("--version", main_show_version, "Show ArxLang version.");

  CLI11_PARSE(app, argc, argv);

  main_compile();

  return 0;
}
