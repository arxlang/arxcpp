#pragma once

// #include "llvm/Support/raw_ostream.h"
namespace llvm {
class raw_ostream;
}

llvm::raw_ostream& indent(llvm::raw_ostream&, int);

/* Used for wrapping getchar */
class IOSource {
 public:
  static char* buffer; /* used for testing */
  static char getchar();
  static void update_buffer(char*);
};

void show_version();
