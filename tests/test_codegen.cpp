#include <gtest/gtest.h>
#include "../arx/include/codegen.h"

// Check show llvm
TEST(CodeGenTest, ShowLLVM) {
  IOSource::update_buffer((char*)R""""(
  function add_one(a):
    a + 1

  add(1);
  )"""");

  show_llvm(1);
}

// Check object generation
TEST(CodeGenTest, ObjectGeneration) {
  IOSource::update_buffer((char*)R""""(
  function add_one(a):
    a + 1

  add(1);
  )"""");

  open_shell(1);
}
