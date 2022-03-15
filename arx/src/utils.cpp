#include <string>
#include "llvm/Support/raw_ostream.h"

llvm::raw_ostream& indent(llvm::raw_ostream& O, int size) {
  return O << std::string(size, ' ');
}
