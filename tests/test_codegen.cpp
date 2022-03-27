#include <gtest/gtest.h>
#include "../arx/include/codegen.h"

// Demonstrate some basic assertions.
TEST(CodeGenTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}
