#include <cstdlib>
#include <iostream>
#include <string>

#include <llvm/Support/raw_ostream.h>

#include "utils.h"

extern std::string ARX_VERSION;

auto indent(llvm::raw_ostream& O, int size) -> llvm::raw_ostream& {
  return O << std::string(size, ' ');
}

auto show_version() -> void {
  std::cout << "arx version: " << ARX_VERSION << std::endl;
}
