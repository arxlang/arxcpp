#pragma once

#include <stdio.h>
#include <memory>

// "llvm/IR/Value.h"
namespace llvm {
class Value;
}

/// LogError* - These are little helper functions for error handling.
template <typename T>
std::unique_ptr<T> LogError(const char* Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}

llvm::Value* LogErrorV(const char*);
