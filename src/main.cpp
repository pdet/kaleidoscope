#include "main.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "lexer/token.hpp"
#include "llvm/Support/raw_ostream.h"

//Optimization Passes imports
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"

// This is an object that owns LLVM core data structures
llvm::LLVMContext TheContext;

// This is a helper object that makes easy to generate LLVM instructions
llvm::IRBuilder<> Builder(TheContext);

// This is an LLVM construct that contains functions and global variables
std::unique_ptr<llvm::Module> TheModule;

// This map keeps track of which values are defined in the current scope
std::map<std::string, llvm::Value *> NamedValues;

std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;

//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//

static void HandleDefinition() {
    if (auto FnAST = ParseDefinition()) {
        if (auto *FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read function definition:");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleExtern() {
    if (auto ProtoAST = ParseExtern()) {
        if (auto *FnIR = ProtoAST->codegen()) {
            fprintf(stderr, "Read extern: ");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (auto FnAST = ParseTopLevelExpr()) {
        if (auto *FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read top-level expression:");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}


static void MainLoop() {
    while (1) {
        fprintf(stderr, "ready> ");
        switch (CurTok) {
            case tok_eof:
                return;
            case ';': // ignore top-level semicolons.
                getNextToken();
                break;
            case tok_def:
                HandleDefinition();
                break;
            case tok_extern:
                HandleExtern();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
    }
}

// Optimizer
void InitializeModuleAndPassManager(void){
    // Open a new module
    TheModule = llvm::make_unique<llvm::Module>("my cool jit", TheContext);

    // Create a new pass manager attached to the module
    TheFPM = llvm::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());

    //Now we add a bunch of calls related to LLVM passes: (All "cleanup" optimizations)
    // Do simple "peephole"and but-twiddling optimizations.
    TheFPM->add(llvm::createInstructionCombiningPass());

    //Reassociate expressions
    TheFPM->add(llvm::createReassociatePass());

    //Eliminate common subexpressions
    TheFPM->add(llvm::createGVNPass());

    //Simplify the control flow graph (e.g., delete unreachable blocks)
    TheFPM->add(llvm::createCFGSimplificationPass());

    TheFPM->doInitialization();

}

int main (){
    // Install standard binary operators
    // 1 is lowest precedence
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 30;
    BinopPrecedence['*'] = 40;

    // Prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();

    // Make the module, which holds all the code.
    TheModule = llvm::make_unique<llvm::Module>("my cool jit", TheContext);
    InitializeModuleAndPassManager();
    // Run the main "interpreter loop" now.
    MainLoop();

    // Print out all of the generated code.
    TheModule->print(llvm::errs(), nullptr);

    return 0;
}