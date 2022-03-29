#include <stdio.h>
#include <string.h>

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

#include <glog/logging.h>

#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Scalar.h"

#include "codegen.h"
#include "jit.h"
#include "lexer.h"
#include "parser.h"
#include "settings.h"
#include "utils.h"

auto sys_getchar() -> char {
  return getchar();
}

// declared by lexer.h
getchar_ptr arx_getchar = &sys_getchar;

std::string ARX_VERSION = "1.2.0";  // semantic-release

static auto check_version(const char* arg) -> bool {
  if (std::string(arg) == "--version") {
    std::cout << "arx version: " << ARX_VERSION << std::endl;
    return true;
  }
  return false;
}

auto main(int argc, const char* argv[]) -> int {
  google::InitGoogleLogging(argv[0]);

  std::string output_filename;

  for (int i = 0; i < argc; ++i) {
    if (check_version(argv[i])) {
      return 0;
    }

    if (std::string(argv[i]) == "--output") {
      output_filename = argv[++i];
    }
  }

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  load_settings();

  // Prime the first token.
  getNextToken();

  TheJIT = ExitOnErr(llvm::orc::ArxJIT::Create());

  InitializeModule();

  // Add the current debug info version into the module.
  TheModule->addModuleFlag(
      llvm::Module::Warning,
      "Debug Info Version",
      llvm::DEBUG_METADATA_VERSION);

  // Darwin only supports dwarf2.
  if (llvm::Triple(llvm::sys::getProcessTriple()).isOSDarwin())
    TheModule->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);

  // Construct the DIBuilder, we do this here because we need the module.
  DBuilder = std::make_unique<llvm::DIBuilder>(*TheModule);

  // Create the compile unit for the module.
  // Currently down as "fib" as a filename since we're redirecting stdin
  // but we'd like actual source locations.
  KSDbgInfo.TheCU = DBuilder->createCompileUnit(
      llvm::dwarf::DW_LANG_C,
      DBuilder->createFile(output_filename, "."),
      "Arx Compiler",
      0,
      "",
      0);

  // Run the main "interpreter loop" now.
  MainLoop();

  // Finalize the debug info.
  DBuilder->finalize();

  // Print out all of the generated code.
  TheModule->print(llvm::errs(), nullptr);

  return 0;
}
