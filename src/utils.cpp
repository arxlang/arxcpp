#include <iostream>
#include <string>

#include <llvm/Support/raw_ostream.h>

#include "utils.h"

extern std::string ARX_VERSION;

/**
 * @brief Ident the output.
 * @param O Buffer used by the indentation.
 * @param size Buffer size.
 * @return a buffer with the indentation with given width.
 *
 */
auto indent(llvm::raw_ostream& O, int size) -> llvm::raw_ostream& {
  return O << std::string(size, ' ');
}

/**
 * @brief Show the Arx version.
 *
 */
auto show_version() -> int {
  std::cout << "arx version: " << ARX_VERSION << std::endl;
  return 0;
}
