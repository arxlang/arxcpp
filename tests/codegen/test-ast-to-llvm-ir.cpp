#include <gtest/gtest.h>                    // for Test, TestInfo (ptr only)
#include <memory>                           // for allocator
#include "../src/codegen/ast-to-llvm-ir.h"  // for compile_llvm_ir
#include "../src/io.h"                      // for string_to_buffer
#include "parser.h"                         // for TreeAST

// Check object generation
TEST(CodeGenTest, ObjectGeneration) {
  string_to_buffer((char*) R""""(
  function add_one(a):
    a + 1

  add(1);
  )"""");

  auto ast = std::make_unique<TreeAST>(TreeAST());
  compile_llvm_ir(*ast);
}
