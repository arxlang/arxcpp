#include <glog/logging.h>
#include <gtest/gtest.h>

#include "lexer.h"
#include "utils.h"

std::string ARX_VERSION = "t.e.s.t";
extern std::string OUTPUT_FILE;

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);

  OUTPUT_FILE = "output.o";

  return RUN_ALL_TESTS();
}
