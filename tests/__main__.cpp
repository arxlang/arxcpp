#include <glog/logging.h>
#include <gtest/gtest.h>

#include "lexer.h"
#include "utils.h"

// declared by lexer.h
getchar_ptr arx_getchar = &IOSource::getchar;

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);

  return RUN_ALL_TESTS();
}
