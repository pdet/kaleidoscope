#include "ast/BinaryExprAST.hpp"

llvm::Value *BinaryExprAST::codegen() {
    // Look this variable up in the function
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();

    if(!L || !R){
    	return nullptr;
    }

    switch(Op){
    	case '+':
    		return Builder.CreateFAdd(L,R,"addtmp");
		case '-':
    		return Builder.CreateFSub(L,R,"subtmp");
		case '*':
    		return Builder.CreateFMul(L,R,"multmp");
		case '<':
    		L = Builder.CreateFCmpULT(L,R,"cmptmp");
    		//convert bool (0/1) to double (0.0/1.1)
    		return Builder.CreateUIToFP(L, llvm::Type::getDoubleTy(TheContext),"booltmp");
		default:
			return LogErrorV("invalid binary operator");

    }
}