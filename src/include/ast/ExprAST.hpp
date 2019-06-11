#pragma once
#include "llvm/IR/BasicBlock.h"

// Base Class for all expression nodes.
class ExprAST{
	public:
		virtual ~ExprAST() = default;
		// Value is an LLVM object used to represent  SSA register
		// Its value is computed as the related instruction executes and it does not get a new value until
		// (and if) the instruction re-executes
		virtual llvm::Value *codegen() = 0;
};