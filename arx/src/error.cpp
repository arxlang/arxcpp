#include <cstdio>

// #include <llvm/IR/Value.h>
namespace llvm {
class Value;
}

auto LogErrorV(const char* Str) -> llvm::Value* {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
