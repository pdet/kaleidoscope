#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast/NumberExprAST.hpp"
#include "ast/VariableExprAST.hpp"
#include "ast/CallExprAST.hpp"
#include "ast/BinaryExprAST.hpp"

#include "lexer/token.hpp"
#include "lexer/lexer.hpp"
#include "logger/logger.hpp"
#include "parser/parser.hpp"

//Holds the precedence for each binary operator that is defined
std::map<char,int> BinopPrecedence;

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
std::unique_ptr<PrototypeAST> ParseExtern() {
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