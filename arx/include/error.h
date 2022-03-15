#pragma once

#include <memory>
#include "llvm/IR/Value.h"
#include "parser.h"

std::unique_ptr<ExprAST> LogError(const char*);
std::unique_ptr<PrototypeAST> LogErrorP(const char*);
llvm::Value* LogErrorV(const char*);
