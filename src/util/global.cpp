#include "util/global.hpp"

// This is an object that owns LLVM core data structures
llvm::LLVMContext TheContext;

// This is a helper object that makes easy to generate LLVM instructions
llvm::IRBuilder<> Builder(TheContext);

// This is an LLVM construct that contains functions and global variables
std::unique_ptr<llvm::Module> TheModule;

// This map keeps track of which values are defined in the current scope
std::map<std::string, llvm::Value *> NamedValues;

std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;

std::unique_ptr<llvm::orc::KaleidoscopeJIT> TheJIT;

std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;


//A module is a unit of allocation for the JIT and functions are part of the module that contains anonymous expressions
// We need to put anonymous expressions in different modules so we can delete it without affecting the rest of the func
llvm::Function *getFunction(std::string name){
    //First, see if the function has already been added to the current module.
    if (auto* F = TheModule->getFunction(name))
        return F;
    // If not, check whether we can codegen the declaration from some existing prototype.
    auto FI = FunctionProtos.find(name);
    if(FI != FunctionProtos.end())
        return FI->second->codegen();
    // If no existing protoype exists, return null.
    return nullptr;
}