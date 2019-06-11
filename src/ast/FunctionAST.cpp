#include "ast/FunctionAST.hpp"

// It returns an LLVM Function

llvm::Function *FunctionAST::codegen() {
  //First, check for an existing function from a previous 'extern' declaration
  // in case it has alreadt been created using the 'extern' statement
    llvm::Function *TheFunction = TheModule->getFunction(Proto->getName());
  // If function has not been created we codegen one from prototype.
  if (!TheFunction){
    TheFunction = Proto->codegen();
  }
  if (!TheFunction){
    return nullptr;
  }
  // Assert function is empty
  if(!TheFunction->empty()){
    return (llvm::Function*) LogErrorV("Function cannot be redefined");
  }

  //Create a new basic block to start insertion into
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext,"entry",TheFunction);
  // Tells the builder that the instruction should be added to end of the basic block
  Builder.SetInsertPoint(BB);

  // Record the function arguments in the NamesValues map
  NamedValues.clear();
  for (auto &Arg : TheFunction->args()){
    NamedValues[Arg.getName()] = & Arg;
  }
  if(llvm::Value *RetVal = Body->codegen()){
    // Finish off the function
    Builder.CreateRet(RetVal);
    // Validate the generated code, checking for consistency
    verifyFunction(*TheFunction);
    return TheFunction;
  }
  //Error reading body, removing function
  TheFunction->eraseFromParent();
  return nullptr;
}