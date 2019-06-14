#include "ast/FunctionAST.hpp"
#include "ast/CallExprAST.hpp"
#include "util/global.hpp"

// It returns an LLVM Function

llvm::Function *FunctionAST::codegen() {
    //Transfer Ownership of the prototype to the fucntioprotosmap, but keep a reference to it for use below.
    auto &P = *Proto;
    // We need to update the FunctionProtos map first, then call get fucntion
    FunctionProtos[Proto->getName()] = std::move(Proto);
    llvm::Function *TheFunction = getFunction(P.getName());

    if (!TheFunction) {
        return nullptr;
    }
    // Assert function is empty
    if (!TheFunction->empty()) {
        return (llvm::Function *) LogErrorV("Function cannot be redefined");
    }

    //Create a new basic block to start insertion into
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
    // Tells the builder that the instruction should be added to end of the basic block
    Builder.SetInsertPoint(BB);

    // Record the function arguments in the NamesValues map
    NamedValues.clear();
    for (auto &Arg : TheFunction->args()) {
        NamedValues[Arg.getName()] = &Arg;
    }
    if (llvm::Value *RetVal = Body->codegen()) {
        // Finish off the function
        Builder.CreateRet(RetVal);
        // Validate the generated code, checking for consistency
        verifyFunction(*TheFunction);
        //Optimize the function
        TheFPM->run(*TheFunction);
        return TheFunction;
    }
    //Error reading body, removing function
    TheFunction->eraseFromParent();
    return nullptr;
}