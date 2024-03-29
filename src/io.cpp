#include "io.h"
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>  // IWYU pragma: keep
#include <iostream>
#include <sstream>  // IWYU pragma: keep
#include <string>

std::stringstream input_buffer;
std::string INPUT_FILE{""};
std::string OUTPUT_FILE{""};
bool INPUT_FROM_STDIN = false;

/**
 * @brief Get a char from the buffer of from the default input.
 * @return An integer represenation of a char from the buffer.
 *
 */
auto get_char() -> int {
  if (INPUT_FROM_STDIN) {
    return getchar();
  }
  return input_buffer.get();
}

/**
 * @brief Copy the file content to the buffer.
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
 * @brief Copy the given string to the buffer.
 *
 */
auto string_to_buffer(std::string value) -> void {
  input_buffer.clear();
  input_buffer.str("");
  input_buffer << value << std::endl << EOF;
}

/**
 * @brief Load the content file or the standard input to the buffer.
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

auto ArxFile::create_tmp_file(std::string content) -> std::string {
  // template for our file.
  char filename[] = "/tmp/arx_XXXXXX";

  // Creates and opens a new temp file r/w.
  // Xs are replaced with a unique number.
  int fd = mkstemp(filename);

  // Check we managed to open the file.
  if (fd == -1) {
    return std::string("");
  }

  // note 4 bytes total: abc terminating '\0'
  write(fd, content.c_str(), std::strlen(content.c_str()));
  close(fd);

  std::string filename_str(filename);
  std::string filename_ext = filename_str + std::string(".cpp");

  if (rename(filename_str.c_str(), filename_ext.c_str()) != 0) {
    return std::string("");
  }

  return filename_ext;
}

auto ArxFile::delete_file(std::string filename) -> int {
  return unlink(filename.c_str());
}
