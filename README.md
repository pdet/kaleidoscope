# Kaleidoscope
This repository follows the [LLVM Kaleidoscope Tutorial](https://llvm.org/docs/tutorial).

Kaleidoscope is a procedural language that supports:
* Function definitions
* 'extern' keyword to define a function febore you use it
* Conditionals (If/Then/Else)
* For loop
* Math
* 64-bit floating point as its unique data type

Below is one example that calculates Fibonacci Numbers:
```python
def fib(x)
	if x < 3 then 
		1
	else 
		fib(x-1) + fib(x-2)

fib(40)
```

## Components

### Lexer
Gets a stream of chars and breaks them into "tokens"
include/lexer/token.hpp has an ENUM structure with the token identifiers used in lexical analysis.
lexer/lexer.cpp has the gettok function that reads characters from stdin and gransforms them into tokens.
