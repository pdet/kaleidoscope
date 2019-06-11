#pragma once

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

// LLVM core data structures
extern llvm::LLVMContext TheContext;

// LLVM instruction generator
extern llvm::IRBuilder<> Builder;

// Functions and global variables
extern std::unique_ptr<llvm::Module> TheModule;

// Values are defined in the current scope
extern std::map<std::string, llvm::Value *> NamedValues;
