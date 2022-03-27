#include <memory>
#include "llvm/IR/Value.h"

llvm::Value* LogErrorV(const char* Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
