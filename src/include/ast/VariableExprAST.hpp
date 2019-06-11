#pragma once
#include "ast/ExprAST.hpp"
#include "logger/logger.hpp"
// Expression class for referencing a variable (e.g., a)
class VariableExprAST : public ExprAST {
	std::string Name;

public:
	VariableExprAST(const std::string &Name): Name(Name){}
	llvm::Value *codegen() override;

};