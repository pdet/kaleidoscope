#pragma once
#include <string>
#include <vector>
#include "ast/ExprAST.hpp"
#include "llvm/IR/IRBuilder.h"

// This reprensets the "prototype" for a funciton
// captures its name, and its argument names
// implicitly the number of arguments the function takes

class PrototypeAST{
	std::string Name;
	std::vector<std::string> Args;

public:
	PrototypeAST(const std::string &name, std::vector<std::string> Args):
	Name(name), Args(std::move(Args)){}

	const std::string &getName() const{
		return Name;
	}
	llvm::Function *codegen();


};