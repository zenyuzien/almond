#ifndef _lexer
#define _lexer 

#include "compiler.h"
#include <string>
#include <vector>

enum 
{
    DEFAULT_NUM ,
    LONG_NUM, 
    FLOAT_NUM,
    DOUBLE_NUM
};
#define NUMBER_CASE \
    case '0':       \
    case '1':       \
    case '2':       \
    case '3':       \
    case '4':       \
    case '5':       \
    case '6':       \
    case '7':       \
    case '8':       \
    case '9' 

#define OPERATOR_CASE                   \
    case '+':                           \
    case '-':                           \
    case '*':                           \
    case '>':                           \
    case '<':                           \
    case '^':                           \
    case '%':                           \
    case '!':                           \
    case '=':                           \
    case '~':                           \
    case '|':                           \
    case '&':                           \
    case '(':                           \
    case '[':                           \
    case ',':                           \
    case '.':                           \
    case '?'

#define SYMBOL_CASE \
    case '{':       \
    case '}':       \
    case ':':       \
    case ';':       \
    case '#':       \
    case '\\':      \
    case ')':       \
    case ']'  
enum 
{
    LEXER_SUCCESS,
    LEXER_INPUT_ERROR
};

struct lexer 
{
    lexer(compilation* compiler, std::string notes)
        : line_no(1),
          col_no(1),
          vec_t(new std::vector<token*>()),
          compiler(compiler),
          expressions(0),
          notes(notes),
          notes_ptr(0)
    {}

    ~lexer() {
        delete vec_t;
    }
    int line_no , col_no ; 
    const char* filename; 
    std::vector< token* > *vec_t; 
    compilation* compiler; 

    int expressions; 
    std::string bracket_content; 

    std::string notes; 
    int notes_ptr ;

    char next_char(bool mode = false );
    char top_char(bool mode = false );
    void push_char(char c, bool mode = false);
    //void* lexer_note();
    //std::vector<token*> *lexer_tokens();
    token* read_next_token();
    int lex(); 
};



#endif 