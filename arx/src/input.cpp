#include <stdio.h>
#include <fstream>  // IWYU pragma: keep
#include <iostream>
#include <sstream>  // IWYU pragma: keep
#include <string>

std::stringstream input_buffer;

auto get_char() -> int {
  return input_buffer.get();
}

auto file_to_buffer(std::string filename) -> void {
  std::ifstream arxfile(filename);
  std::string line;

  input_buffer.clear();

  if (arxfile.is_open()) {
    std::cout << "file is open" << std::endl;
    while (getline(arxfile, line)) {
      input_buffer << line << std::endl;
    }
    arxfile.close();
  }
}

auto string_to_buffer(std::string value) -> void {
  input_buffer.clear();
  input_buffer.str("");
  input_buffer << value << std::endl << EOF;
}
