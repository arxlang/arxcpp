#include <gtest/gtest.h>
#include "../arx/include/utils.h"

// Demonstrate some basic assertions.
TEST(UtilsTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}
