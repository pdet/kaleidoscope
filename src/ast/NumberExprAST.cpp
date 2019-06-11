#include "ast/NumberExprAST.hpp"

// Numeric contants are represented with the ConstantFP class
// It holds the numeric value in an APFloat (which has the capacity of holding floating points)
// LLVM constants are uniquely created, hence foo::get()
llvm::Value *NumberExprAST::codegen() {
    return llvm::ConstantFP::get(TheContext,llvm::APFloat(Val));
}