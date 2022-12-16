#include <gtest/gtest.h>
#include "../arx/include/codegen.h"
#include "../arx/include/io.h"

// Check object generation
TEST(CodeGenTest, ObjectGeneration) {
  string_to_buffer((char*)R""""(
  function add_one(a):
    a + 1

  add(1);
  )"""");

  compile();
}
