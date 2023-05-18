#include <iosfwd>  // for basic_stringstream, stringstream
#include <string>  // for string

extern std::stringstream input_buffer;
extern std::string OUTPUT_FILE;
extern std::string INPUT_FILE;
extern bool INPUT_FROM_STDIN;

auto get_char() -> int;
auto file_to_buffer(std::string filename) -> void;
auto string_to_buffer(std::string value) -> void;
auto load_input_to_buffer() -> void;

class ArxFile {
 public:
  static auto create_tmp_file(std::string content) -> std::string;
  static auto delete_file(std::string filename) -> int;
};
