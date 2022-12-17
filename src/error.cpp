/**
 * @file
 * @version 1.0
 * @author Ivan Ogasawara <ivan.ogasawara@gmail.com>
 * @date 2022-09-19
 *
 * @section LICENSE
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @section DESCRIPTION
 *
 *  This module handles errors.
 *
 */

/**
 * @brief Prints error in stderr.
 * @param Str String of the error.
 * @return nullptr objetct.
 *
 *  Class responsible for throw errors to stderr.
 *
 */

#include <cstdio>

// #include <llvm/IR/Value.h>
namespace llvm {
  class Value;
}

auto LogErrorV(const char* Str) -> llvm::Value* {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
