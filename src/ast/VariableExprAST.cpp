#include "ast/VariableExprAST.hpp"

// Assume Variable has been emmited somewhere thus is already available.
// The only values that can be in NamedValues map are function arguments
llvm::Value *VariableExprAST::codegen() {
    // Look this variable up in the function
    llvm::Value *V = NamedValues[Name];
    if (!V){
    	LogErrorV("Unkown variable name");
    }
    return V;
}