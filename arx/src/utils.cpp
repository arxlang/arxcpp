#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "llvm/Support/raw_ostream.h"

const int BUFFER_SIZE = 1000;

llvm::raw_ostream& indent(llvm::raw_ostream& O, int size) {
  return O << std::string(size, ' ');
}

/* define IOSource */
char* IOSource::buffer = nullptr;

char IOSource::getchar() {
  if (IOSource::buffer) {
    return *IOSource::buffer++;
  }
  return getchar();
}

void IOSource::update_buffer(char* buffer) {
  if (!IOSource::buffer) {
    IOSource::buffer = new char[BUFFER_SIZE];
  }

  strncpy(IOSource::buffer, buffer, std::strlen(buffer));
}
