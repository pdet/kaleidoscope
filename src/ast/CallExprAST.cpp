#include "ast/CallExprAST.hpp"
#include "util/global.hpp"

// We first lookup in the LLVM Module's symbolical table for the function name
// After having the function we need to call, every argument is recursively codegen.
llvm::Value *CallExprAST::codegen() {
    // Look up the name in the global module table
    llvm::Function *CalleeF = getFunction(Callee);
    if (!CalleeF){
        return LogErrorV("Unkown function referenced");
    }
    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size()){
        return LogErrorV("Incorrect # arguments passed");
    }
    std::vector <llvm::Value *> ArgsV;
    for (unsigned i = 0, e = Args.size(); i!=e;++i){
        ArgsV.push_back(Args[i]->codegen());
        if (!ArgsV.back()){
            return nullptr;
        }
    }
    return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}