#include <iosfwd>  // for basic_stringstream, stringstream
#include <string>  // for string

extern std::stringstream input_buffer;
extern std::string OUTPUT_FILE;
extern std::string INPUT_FILE;
extern bool INPUT_FROM_STDIN;

int get_char();
void file_to_buffer(std::string filename);
void string_to_buffer(std::string value);
void load_input_to_buffer();
