#ifndef _lexer
#define _lexer 
#include "token.h"
#include "compiler.h"
#include <string>
#include <vector>

#define NUMBER_CASE \
    case '0': case '1': case '2': case '3': case '4': \
    case '5': case '6': case '7': case '8': case '9'

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

struct lexer 
{
    int lineNo , colNo, expressions ; 
    const char* fileName; 
    std::vector< Token::token* > *vecTokens; 
    compilation* compiler; 
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

    char nextCharInSourceFile( );
    char peekCharInSourceFile();
    void pushCharToSourceFile(char c);
    Token::token* getNextToken();
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