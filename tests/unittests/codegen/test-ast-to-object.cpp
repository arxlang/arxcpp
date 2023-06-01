#include <gtest/gtest.h>
#include <memory>

#include "../src/codegen/ast-to-object.h"
#include "../src/io.h"

extern bool IS_BUILD_LIB;

// Check object generation
TEST(CodeGenTest, ObjectGeneration) {
  string_to_buffer((char*) R""""(
  function add_one(a):
    a + 1

  add(1);
  )"""");

  auto ast = std::make_unique<TreeAST>(TreeAST());
  IS_BUILD_LIB = true;
  compile_object(*ast);
}
