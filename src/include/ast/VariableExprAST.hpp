#pragma once
// Expression class for referencing a variable (e.g., a)
class VariableExprAST : public ExprAST {
	std::string Name;

public:
	VariableExprAST(const std::string &Name): Name(Name){}
};