#include "main.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "lexer/token.hpp"
#include "llvm/Support/raw_ostream.h"
#include "KaleidoscopeJIT.hpp"

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

std::unique_ptr<llvm::orc::KaleidoscopeJIT> TheJIT;

//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//


// Optimizer
void InitializeModuleAndPassManager(void){
    // Open a new module
    TheModule = llvm::make_unique<llvm::Module>("my cool jit", TheContext);
    TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());

    // Create a new pass manager attached to the module
    TheFPM = llvm::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());

    //Now we add a bunch of calls related to LLVM passes: (All "cleanup" optimizations)
    // Do simple "peephole"and but-twiddling optimizations.
    TheFPM->add(llvm::createInstructionCombiningPass());

    //Reassociate expressions
    TheFPM->add(llvm::createReassociatePass());

    //Eliminate common subexpressions
    TheFPM->add(llvm::createNewGVNPass());

    //Simplify the control flow graph (e.g., delete unreachable blocks)
    TheFPM->add(llvm::createCFGSimplificationPass());

    TheFPM->doInitialization();

}

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
        if (FnAST->codegen()) {
            //JIT the module containing the anonymous expression keep a handle to free it later
            //addModule triggers the code generation for all functions in the model and returns a handler that can be
            //used to remove the module from the JIT later.
            auto H = TheJIT->addModule(std::move(TheModule));
            InitializeModuleAndPassManager();

            //Search the JIT for the __anon_expr symbol
            //We get the pointer to the final generated code by search for the __anon_expr symbol
            auto ExprSymbol = TheJIT->findSymbol("__anon_expr");
            assert(ExprSymbol && "Function not found");

            //Get the symbol's address and cast it to the right type.Takes nor arguments, returns a double
            // We can call it as a native function
            //Since the LLVM JIT compiler matches the native platform ABI, we can just cast the result pointer to a
            //function pointer of that type and call it directly.
            //There is no difference beween JIT compiled code and native machine code that is statically linked to the
            //application
            double (*FP)() = (double (*)())(intptr_t)cantFail(ExprSymbol.getAddress());
            fprintf(stderr, "Evaluated to %f\n", FP());

            // Delete the anonymous expression module from the JIT.
            // Since we don'' support re-evaluation of top-level expressions we remove the module from the JIT to free
            // the associated memory
            TheJIT->removeModule(H);
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


int main (){
    //Initializing the JIT
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();

    // Install standard binary operators
    // 1 is lowest precedence
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 30;
    BinopPrecedence['*'] = 40;

    // Prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();

    TheJIT = llvm::make_unique<llvm::orc::KaleidoscopeJIT>();

    InitializeModuleAndPassManager();

    // Run the main "interpreter loop" now.
    MainLoop();

    // Print out all of the generated code.
    TheModule->print(llvm::errs(), nullptr);

    return 0;
}