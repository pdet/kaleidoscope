#include<cstdio>
#include <string>

// If current token is an identifier IndetifierStr holds the name of the identifier.
// Otherwise, if its numerical (e.g., 1.0) NumVal holds its value.
static std::string IndetifierStr; // Filled in if tok_identifier

static double NumVal; // Filled in if tok_number

// The actual implementation of the lexter is the gettok function
// It is called to return the next token from the std input
static int gettok(){
	static int LastChar = ' ';
	// Skip any whitespace
	while (isspace(LastChar))
		LastChar = getchar();

	// idenfitify specific keyworkds (e.g., def)
	if (isalpha(LastChar)){ // identifier [a-z A-Z 0-9]*
		IndetifierStr = LastChar;
		while (isalnum((LastChar = getchar())))
			IndetifierStr+=LastChar;
		if (IndetifierStr == "def")
			return tok_def;
		if (IndetifierStr == "extern")
			return tok_extern;
		return tok_identifier;

	}

	// Handling Numbers
	if (isdigit(LastChar) || LastChar == '.'){ //Number [0-9.]+
		std::string NumStr;
		do{
			NumStr += LastChar;
			LastChar = getchar();
		} while (isdigit(LastChar) || LastChar == '.');
		NumVal = strtod(NumStr.c_str(),0); // convert NumSrt to numeric value
		return tok_number;
	} 
	// Handling Comments
	// We skip to the end of the line and then return the next toek
	if (LastChar == '#'){
		// Comment until EOF
		do
			LastChar = getchar();
			while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
		if (LastChar != EOF)
			return gettok();	
	}

	//Handling Operator characters (e.g., +) and EOF
	if (LastChar == EOF)
		return tok_eof;
	// Otherwise, just return the character as its ASCII value
	int ThisChar = LastChar;
	LastChar = getchar();
	return ThisChar;
}