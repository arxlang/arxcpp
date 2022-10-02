#pragma once

#include <cctype>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>

#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"

#include "error.h"
#include "jit.h"
#include "lexer.h"
#include "parser.h"
#include "utils.h"

class AST2Object {};

auto compile() -> void;
auto compile_to_file() -> void;
auto open_shell() -> void;
