#pragma once

#include <cstdio>
#include <memory>

namespace llvm {
  /**
   * @brief "llvm/IR/Value.h"
   *
   */
  class Value;
}  // namespace llvm

/**
 * @brief LogError* - A little helper function for error handling.
 *
 */
template <typename T>
std::unique_ptr<T> LogError(const char* msg) {
  fprintf(stderr, "Error: %s\n", msg);
  return nullptr;
}

/**
 * @brief LogError* - A little helper function for error handling with line
 *  and col information.
 *
 */
template <typename T>
std::unique_ptr<T> LogParserError(const char* msg, int line, int col) {
  fprintf(stderr, "ParserError[%i:%i]: %s\n", line, col, msg);
  return nullptr;
}

llvm::Value* LogErrorV(const char*);
