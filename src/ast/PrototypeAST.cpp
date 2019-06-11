#include "ast/PrototypeAST.hpp"

// It returns an LLVM Function

llvm::Function *PrototypeAST::codegen() {
    // Make the function type : double(double,double) etc..
   std::vector<llvm::Type*> Doubles(Args.size(), llvm::Type::getDoubleTy(TheContext));
   // creates the FunctionType that should be used for a given Prototype
   // Types are unique like constants
   llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(TheContext),Doubles,false);
   // Creates the IR function corresponding to the prototype.
   // It indicates the type, linkage and name to use.
   // The External Linkage means the function may be defined outside the current module
   // or that is callable by functions outside the module.
   llvm::Function *F = llvm::Function::Create(FT,llvm::Function::ExternalLinkage, Name, TheModule.get());
   // Set names for all arguments.
   // Not necessary but keeps the names consistent.
   unsigned Idx = 0;
   for (auto &Arg : F->args()){
        Arg.setName(Args[Idx++]);
   }
   return F;
}