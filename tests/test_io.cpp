#include <memory>

#include <glog/logging.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "../arx/include/io.h"
#include "../arx/include/lexer.h"

TEST(InputTest, GetCharTest) {
  string_to_buffer((char*)"1");
  EXPECT_EQ(get_char(), 49);

  string_to_buffer((char*)"2");
  EXPECT_EQ(advance(), 50);

  string_to_buffer((char*)"3");
  EXPECT_EQ(advance(), 51);
}
