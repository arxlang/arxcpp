#include <gtest/gtest.h>
#include <memory>

#include "../src/codegen/ast-to-stdout.h"
#include "../src/io.h"

// Check object generation
TEST(CodeGenTest, ObjectGeneration) {
  string_to_buffer((char*) R""""(
  fn add_one(a: float) -> float:
    a + 1

  add(1);
  )"""");

  auto ast = std::make_unique<TreeAST>(TreeAST());
  print_ast(*ast);
}
