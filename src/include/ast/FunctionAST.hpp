#pragma once
#include "PrototypeAST.hpp"
// Function definition itself
class FunctionAST{
	std::unique_ptr<PrototypeAST> Proto;
	std::unique_ptr<ExprAST> Body;

public:
	FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body):
	Proto(std::move(Proto)), Body(std::move(Body)){}
};