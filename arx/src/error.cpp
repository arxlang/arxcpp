#include <memory>
#include "llvm/IR/Value.h"

auto LogErrorV(const char* Str) -> llvm::Value* {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
