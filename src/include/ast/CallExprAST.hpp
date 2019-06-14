#pragma once
#include "ast/ExprAST.hpp"
#include "llvm/IR/IRBuilder.h"
#include "logger/logger.hpp"

//Holds most recent prototype for each function
// Expression class for functionc calls.
class CallExprAST: public ExprAST {
	std::string Callee;
	std::vector<std::unique_ptr<ExprAST>> Args;

public:
	CallExprAST (const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args):
		Callee(Callee), Args(std::move(Args)){}

    llvm::Value *codegen() override;


};