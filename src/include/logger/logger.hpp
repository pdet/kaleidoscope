#pragma once

#include "ast/ExprAST.hpp"
#include "ast/PrototypeAST.hpp"


std::unique_ptr<ExprAST> LogError (const char *Str);
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str);
llvm::Value *LogErrorV(const char *Str);