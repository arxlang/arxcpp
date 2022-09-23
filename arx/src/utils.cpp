#include <iostream>
#include <string>

#include <llvm/Support/raw_ostream.h>

#include "utils.h"

extern std::string ARX_VERSION;

/**
 * @brief
 * @param O
 * @param size
 * @return 
 *
 */
auto indent(llvm::raw_ostream& O, int size) -> llvm::raw_ostream& {
  return O << std::string(size, ' ');
}

/**
 * @brief
 * @return 
 *
 */
auto show_version() -> void {
  std::cout << "arx version: " << ARX_VERSION << std::endl;
}
