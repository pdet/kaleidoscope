#include "ast/ForExprAST.hpp"
#include "util/global.hpp"

llvm::Value *ForExprAST::codegen(){
    // Emit the start code forst, without 'variable'in scope.
    llvm::Value * StartVal = Start->codegen();
    if(!StartVal){
        return nullptr;
    }
    // Make the new basic block for the loop header
    //inserting after current block.
    llvm::Function * TheFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *PreheaderBB = Builder.GetInsertBlock();
    llvm::BasicBlock * LoopBB = llvm::BasicBlock::Create(TheContext, "loop", TheFunction);

    //Insert an explicit fall through from the current block to the LoopBB
    // Since we need to create the Phi node, we remember the block that falls through into the loop.
    // Once we have that we create the actual block that starts the loop and create an unconditional branch
    // for the fall-through between two blocks.
    Builder.CreateBr(LoopBB);

}