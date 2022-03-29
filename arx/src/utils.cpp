#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include <glog/logging.h>

#include "llvm/Support/raw_ostream.h"

#include "utils.h"

const int BUFFER_SIZE = 10000;

auto indent(llvm::raw_ostream& O, int size) -> llvm::raw_ostream& {
  return O << std::string(size, ' ');
}

/* define IOSource */
char* IOSource::buffer = nullptr;

char IOSource::getchar() {
  return *IOSource::buffer++;
}

void IOSource::update_buffer(char* buffer) {
  if (!IOSource::buffer) {
    IOSource::buffer = new char[BUFFER_SIZE];
  }

  strncpy(IOSource::buffer, buffer, std::strlen(buffer));
}
