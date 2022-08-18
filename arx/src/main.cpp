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
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include <glog/logging.h>

#include "codegen.h"
#include "lexer.h"
#include "utils.h"

const std::string ARX_VERSION = "1.4.0";  // semantic-release
std::string OUTPUT_FILE{"output.o"};

auto main(int argc, const char* argv[]) -> int {
  google::InitGoogleLogging(argv[0]);

  CLI::App app{"ArxLang"};

  app.add_option("--output", OUTPUT_FILE, "Output file.");
  app.add_flag("--shell", open_shell, "Open Arx Shell.");
  app.add_flag("--show-llvm", show_llvm, "Show LLVM IR.");
  app.add_flag("--version", show_version, "Show ArxLang version.");

  CLI11_PARSE(app, argc, argv);

  open_shell(1);

  return 0;
}
