#include "logger/logger.hpp"
// LogError - Little helper functions for error handling
std::unique_ptr<ExprAST> LogError (const char *Str){
	fprintf(stderr, "LogError: %s\n", Str);
	return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str){
	LogError(Str);
	return nullptr;
}