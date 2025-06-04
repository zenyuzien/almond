#ifndef _lexer
#define _lexer 
#include "token.h"
#include "compiler.h"
#include <string>
#include <vector>

// all case labels in one macro to look neat at functions responsible for lexing
#define NUMBER_CASE \
    case '0': case '1': case '2': case '3': case '4': \
    case '5': case '6': case '7': case '8': case '9'

// the division is spearated due to the symbol ambiguity in division, comments, which need separate handling
#define OPERATOR_CASE_WITHOUT_DIVISION \
    case '+': case '-': case '*': case '%': case '^': /* arithmetic */ \
    case '>': case '<': case '=': case '!':           /* comparison */ \
    case '&': case '|': case '~':                     /* bitwise */   \
    case '(': case '[': case ',': case '.': case '?'

#define SYMBOL_CASE \
    case '{': case '}':           /* braces */       \
    case ')': case ']': /* parens/brackets */ \
    case ':': case ';':           /* separators */   \
    case '#': case '\\'           /* preproc/escape */

// this structure helps bring all resources required for lexing in one place.
struct lexer 
{
    // line number, column number, expressions count is useful for debugging
    int lineNo , colNo, expressions ; 
    // filename of the source input file 
    const char* fileName; 
    // a vector of tokens as a pointer for easy referencing
    std::vector< Token::token* > *vecTokens; 
    // a reference to the compiler will be handy as to access token pointers and other useful utilities ( refer it for details )
    compilation* compiler; 
    // useful for debugging
    std::string parenContent; 

    lexer(compilation* compiler)
        : lineNo(1),
          colNo(1),
          vecTokens(new std::vector<Token::token*>()),
          compiler(compiler),
          expressions(0)
    {}

    ~lexer() {
        delete vecTokens;
    }

    // the following three functions are used to navigate through the input source file

    // this returns current pointed character and moves the file pointer to next character in the source input file
    char nextCharInSourceFile( );
    // this returns the currently pointed character
    char peekCharInSourceFile();
    /* not technically pushing to source file but achieves a similar functionality. 
    It stores in a temp buffer for the peek and next functions to take from, until buffer free. 
    This works because next/peek focus on next ones, when called this, this will be the next they require*/
    void pushCharToSourceFile(char c);

    // returns the next token from processing the input source code stream
    Token::token* getNextToken();

    // the function that performs the entire lexing phase, returns 1 when done, 0 if failed.
    int lex(); 
};

   //void* lexer_note();
   //std::vector<Token::token*> *lexer_tokens();
   // std::string notes; 
   // int notes_ptr ;
   
   //,
          //notes(notes),
          //notes_ptr(0)

#endif 