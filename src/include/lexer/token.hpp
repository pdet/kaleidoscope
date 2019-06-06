#pragma once
// Lexer returns [0-255 if it is an unkown character (return ASCII value), otherwise one of the following]
enum Token{
	tok_eof = -1,
	// commands
	tok_def = -2,
	tok_extern = -3,

	// primary
	tok_identifier = -4,
	tok_number = -5,
};