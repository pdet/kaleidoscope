#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "lexer/token.hpp"
#include "lexer/lexer.hpp"

#include "ast/ExprAST.hpp"
#include "ast/NumberExprAST.hpp"
#include "ast/VariableExprAST.hpp"
#include "ast/CallExprAST.hpp"
#include "ast/BinaryExprAST.hpp"
#include "ast/FunctionAST.hpp"

#include "logger/logger.hpp"
#include "parser/parser.hpp"
#include "main.hpp"

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
#include "llvm/Support/raw_ostream.h"

//Holds the precedence for each binary operator that is defined
static std::map<char,int> BinopPrecedence;

// Get the precedence of the pending binary operator token
static int GetTokPrecedence() {
    if (!isascii(CurTok)) {
        return -1;
    }

    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0) {
        return -1;
    }

    return TokPrec;
}

// expects to be called when tue current token is a tok_number
//creates a NumberExprAST node, advances the lexer to next token and returns
std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = llvm::make_unique<NumberExprAST>(NumVal);
    getNextToken(); // consume the number
    return std::move(Result);
}

// Parses expressions in "(" and ")"
std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken();

    auto V = ParseExpression();
    if (!V) {
        return nullptr;
    }

    if (CurTok != ')') {
        return LogError("Expected )");
    }

    getNextToken();
    return V;
}

// Called when current token is tok_identifier
std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;

    getNextToken(); // eat identifier

    if (CurTok != '(') { // Simple variable ref
        return llvm::make_unique<VariableExprAST>(IdName);
    }

    getNextToken(); //eat(
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        while (true) {
            if (auto Arg = ParseExpression()) {
                Args.push_back(std::move(Arg));
            } else {
                return nullptr;
            }

            if (CurTok == ')') {
                break;
            }

            if (CurTok != ',') {
                return LogError("Expected ')' or ',' in argument list");
            }

            getNextToken();
        }
    }
    //Eat the ')'

    getNextToken();

    return llvm::make_unique<CallExprAST>(IdName, std::move(Args));
}

// wrap all expression-parsing logic together
std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
        default:
            return LogError("Unknown token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}

std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
    // If this is a binop, find its precedence
    while (true) {
        int TokPrec = GetTokPrecedence();
        // If this is a binop that binds at least as tightly as the current binop
        // cosume it. otherwise we are done.
        if (TokPrec < ExprPrec) {
            return LHS;
        }
        //ok, this is a binop
        int BinOp = CurTok;
        getNextToken();// eat binop
        // Parse the primary expression after the binary operator
        auto RHS = ParsePrimary();
        if (!RHS) {
            return nullptr;
        }
        // If binop binds less tihgly with RHS than the operator after RHS
        // The pending operator take RHS as its LHS.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS) {
                return nullptr;
            }
        }
        // Merge LHS/RHS
        LHS = llvm::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();

    if (!LHS) {
        return nullptr;
    }

    return ParseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<PrototypeAST> ParsePrototype() {
    if (CurTok != tok_identifier) {
        return LogErrorP("Expected function name in prototype");
    }

    std::string FnName = IdentifierStr;
    getNextToken();

    if (CurTok != '(') {
        return LogErrorP("Expected '(' in prototype");
    }
    // Read the list of argument names.
    std::vector<std::string> ArgNames;
    while (getNextToken() == tok_identifier) {
        ArgNames.push_back(IdentifierStr);
    }

    if (CurTok != ')') {
        return LogErrorP("Expected ')' in prototype");
    }
    // success.
    getNextToken();  // eat ')'.

    return llvm::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}


//'def' prototype expression
std::unique_ptr<FunctionAST> ParseDefinition() {
    getNextToken();// eat def.

    auto Proto = ParsePrototype();
    if (!Proto) {
        return nullptr;
    }

    if (auto E = ParseExpression()) {
        return llvm::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }

    return nullptr;
}


// 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {
  getNextToken();  // eat extern.
  return ParsePrototype();
}

// arbitrary top-level expressions evaluated on the fly
std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        // Make an anonymous proto.
        auto Proto = llvm::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
        return llvm::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }

    return nullptr;
}

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

  // Run the main "interpreter loop" now.
  MainLoop();

  // Print out all of the generated code.
  TheModule->print(llvm::errs(), nullptr);

  return 0;
}