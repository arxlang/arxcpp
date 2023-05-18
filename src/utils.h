#pragma once

/**
 * @brief Include `llvm/Support/raw_ostream.h`
 *
 */
namespace llvm {
  class raw_ostream;
}

llvm::raw_ostream& indent(llvm::raw_ostream&, int);

/**
 * @brief Used for wrapping getchar
 *
 */
class IOSource {
 public:
  static char* buffer;  // used for testing //
  static char getchar();
  static void update_buffer(char*);
};

auto show_version() -> int;
