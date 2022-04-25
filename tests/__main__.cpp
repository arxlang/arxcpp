#include <glog/logging.h>
#include <gtest/gtest.h>

#include "lexer.h"
#include "utils.h"

// declared by lexer.h
getchar_ptr arx_getchar = &IOSource::getchar;

std::string ARX_VERSION = "t.e.s.t";
std::string OUTPUT_FILE{"output.o"};

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);

  return RUN_ALL_TESTS();
}
