#pragma once

// Expression class for a binary operator
class BinaryExprAST : public ExprAST{
	char Op;
	std::unique_ptr<ExprAST> LHS,RHS;

public:
	BinaryExprAST(char op, std::unique_ptr<ExprAST> LHS, std:: unique_ptr<ExprAST> RHS):
					Op(op),LHS(std::move(LHS)), RHS (std::move(RHS)){}

};