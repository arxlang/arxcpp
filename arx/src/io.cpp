#include <stdio.h>
#include <filesystem>
#include <fstream>  // IWYU pragma: keep
#include <iostream>
#include <sstream>  // IWYU pragma: keep
#include <string>

std::stringstream input_buffer;
std::string INPUT_FILE{""};
std::string OUTPUT_FILE{""};

/**
 * @brief
 * @return
 *
 */
auto get_char() -> int {
  return input_buffer.get();
}

/**
 * @brief
 *
 */
auto file_to_buffer(std::string filename) -> void {
  std::ifstream arxfile(filename);
  std::string line;

  input_buffer.clear();
  if (arxfile.is_open()) {
    while (getline(arxfile, line)) {
      input_buffer << line << std::endl;
    }
    arxfile.close();
  }
}

/**
 * @brief
 *
 */
auto string_to_buffer(std::string value) -> void {
  input_buffer.clear();
  input_buffer.str("");
  input_buffer << value << std::endl << EOF;
}

/**
 * @brief
 *
 */
auto load_input_to_buffer() -> void {
  if (INPUT_FILE != "") {
    std::filesystem::path input_file_path = INPUT_FILE;
    file_to_buffer(std::filesystem::absolute(INPUT_FILE));
  } else {
    std::string file_content;
    std::cin >> file_content;

    if (file_content != "") {
      string_to_buffer(file_content);
    }
  }
}
