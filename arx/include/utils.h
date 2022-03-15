#pragma once

#include <string>
#include "llvm/Support/raw_ostream.h"

llvm::raw_ostream& indent(llvm::raw_ostream&, int);
