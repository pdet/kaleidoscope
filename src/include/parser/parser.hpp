#pragma once
#include "ast/ExprAST.hpp"
#include "ast/FunctionAST.hpp"

extern std::map<char,int> BinopPrecedence;

std::unique_ptr<ExprAST> ParseExpression();

std::unique_ptr<FunctionAST> ParseDefinition();

std::unique_ptr<PrototypeAST> ParseExtern();

std::unique_ptr<FunctionAST> ParseTopLevelExpr();