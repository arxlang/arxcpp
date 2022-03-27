#pragma once

#include <memory>
#include <string>

#include "llvm/Support/raw_ostream.h"

llvm::raw_ostream& indent(llvm::raw_ostream&, int);

/* Used for wrapping getchar */
class IOSource {
 public:
  static char* buffer; /* used for testing */
  static char getchar();
  static void update_buffer(char*);
};
