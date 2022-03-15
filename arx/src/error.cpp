#include <memory>
#include "llvm/IR/Value.h"
#include "parser.h"

/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char* Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char* Str) {
  LogError(Str);
  return nullptr;
}

llvm::Value* LogErrorV(const char* Str) {
  LogError(Str);
  return nullptr;
}
