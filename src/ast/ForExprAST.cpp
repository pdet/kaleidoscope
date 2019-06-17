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

    //Now that the preheader of the loop is set up we start emmiting code for the body of the loop
    //Start insertion in LoopBB
    Builder.SetInsertPoint(LoopBB);

    //Start the PHI node with an entry for Start
    llvm::PHINode *Variable = Builder.CreatePHI(llvm::Type::getDoubleTy(TheContext),2,VarName.c_str());

    Variable->addIncoming(StartVal,PreheaderBB);


    //Within the Loop, the variable is defined equal to the PHI node. If it shadows an existing variable
    // we have to restore it. Save it now.
    llvm::Value *OldVal = NamedValues[VarName];

    //Emit the body of the loop. This, like any other expr, can change the current BB.
    // Note that we ignore the value computed by the body, but don't allow any error
    if(!Body->codegen()){
        return nullptr;
    }

    //Emit the step value.
    llvm::Value *StepVal = nullptr;
    if(Step) {
        StepVal = Step->codegen();
        if (!StepVal) {
            return nullptr;
        }
    }
    else{
        // if not specified use 1.0
        StepVal = llvm::ConstantFP::get(TheContext,llvm::APFloat(1.0));
    }
    llvm::Value * NextVar = Builder.CreateFAdd(Variable,StepVal,"nextvar");

    // Compute the end condition
    llvm::Value * EndCond = End->codegen();
    if(!EndCond){
        return nullptr;
    }
    // Convert condition to a bool by comparing non-equal to 0.0
    EndCond = Builder.CreateFCmpONE(EndCond,llvm::ConstantFP::get(TheContext,llvm::APFloat(0.0)),"loopcond");


    //With the code of the body of the loop complete we just need to finish up the control flow for it.
    // This code remembers the end block (for the phi node), then creates the block for the loop exit ("afterloop")
    // Based on the value of the exit condition it rcreates a conditional branch that chooses between executing
    // the loop again or exiting the loop. Any future code is emitted in the "afterloop" block
    //Create the after loop block and insert it
    llvm::BasicBlock *LoopEndBB = Builder.GetInsertBlock();
    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(TheContext,"afterloop",TheFunction);

    //Insert the conditional branch into the end of the LoopEndBB
    Builder.CreateCondBr(EndCond,LoopBB,AfterBB);

    //Any new code will come in AfterBB
    Builder.SetInsertPoint(AfterBB);


    // Now that we have the "NextVar"value we an add the incoming value to the loop phi node.
    // After, we remove the loop variable from the symbol table so that it isn'' in the scope after the loop
    // Finally, codegen for the loop always returns 0.0
    //Add a new entry to the PHI node for the backend
    Variable->addIncoming(NextVar,LoopEndBB);
    // Restore the unshadowed variable
    if (OldVal){
        NamedValues[VarName] = OldVal;
    }
    else{
        NamedValues.erase(VarName);
    }
    //for expr alawys returns 0.0
    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(TheContext));

}

