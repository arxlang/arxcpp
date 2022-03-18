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

#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"

#include "codegen.h"
#include "jit.h"
#include "lexer.h"
#include "parser.h"
#include "settings.h"
#include "utils.h"

std::string ARX_VERSION = "1.1.1";  // semantic-release

static bool check_version(const char* arg) {
  if (std::string(arg) == "--version") {
    std::cout << "arx version: " << ARX_VERSION << std::endl;
    return true;
  }
  return false;
}

int main(int argc, const char* argv[]) {
  for (int i = 0; i < argc; ++i) {
    if (check_version(argv[i])) {
      return 0;
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
  // Currently down as "fib.arx" as a filename since we're redirecting stdin
  // but we'd like actual source locations.
  KSDbgInfo.TheCU = DBuilder->createCompileUnit(
      llvm::dwarf::DW_LANG_C,
      DBuilder->createFile("fib.arx", "."),
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
