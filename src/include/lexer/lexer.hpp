#pragma once

#include <cstdlib>
#include <string>

extern int CurTok;
extern std::string IdentifierStr; // Filled in if tok_identifier
extern double NumVal; // Filled in if tok_number


int gettok();
int getNextToken();