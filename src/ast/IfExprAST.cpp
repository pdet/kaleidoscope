#include "ast/IfExprAST.hpp"
#include "util/global.hpp"

llvm::Value *IfExprAST::codegen() {
    llvm::Value *CondV = Cond->codegen();
    if (!CondV){
        return nullptr;
    }
    // Convert condition to a bool by comparing non-equal to 0.0
    CondV = Builder.CreateFCmpONE(CondV, llvm::ConstantFP::get(TheContext, llvm::APFloat(0.0)),"ifcond");

    //Getting the current function being built. Get'' it by asking the builder for the current BasicBlock, and sking the block for
    // its "parent" (Function it is currently ebedded into)
    llvm::Function * TheFunction = Builder.GetInsertBlock()->getParent();

    //Create blocks for then and else cases. Intert the 'then' block at the end of the function
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(TheContext,"then",TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(TheContext,"else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(TheContext,"ifcont");

    Builder.CreateCondBr(CondV,ThenBB,ElseBB);

    //Emit then value
    Builder.SetInsertPoint(ThenBB);

    llvm::Value *ThenV = Then->codegen();
    if(!ThenV){
        return nullptr;
    }
    Builder.CreateBr(MergeBB);
    //Codegen of 'Then' can change current block, update ThenBB for the PHI
    ThenBB = Builder.GetInsertBlock();

    //Emit else block (Identical to 'then' with the addition of adding the 'else' to the function)
    TheFunction->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);
    llvm::Value *ElseV = Else->codegen();
    if (!ElseV){
        return nullptr;

    }
    Builder.CreateBr(MergeBB);
    //Codegen of 'Else' can change current block, update ElseBB for the PHI
    ElseBB = Builder.GetInsertBlock();

    //Emit merge block
    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
    llvm::PHINode * PN = Builder.CreatePHI(llvm::Type::getDoubleTy(TheContext),2,"iftmp");
    PN->addIncoming(ThenV,ThenBB);
    PN->addIncoming(ElseV,ElseBB);
    return PN;


}