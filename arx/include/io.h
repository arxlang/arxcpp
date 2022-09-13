#include <fstream>   // IWYU pragma: keep
#include <iostream>  // IWYU pragma: keep
#include <sstream>   // IWYU pragma: keep
#include <string>
#include "iosfwd"  // for stringstream

extern std::stringstream input_buffer;
extern std::string OUTPUT_FILE;
extern std::string INPUT_FILE;

int get_char();
void file_to_buffer(std::string filename);
void string_to_buffer(std::string value);
void load_input_to_buffer();
