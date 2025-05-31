#include "compiler.h"
#include "lexer.h"
#include "parser.h"

#include <memory>
#include <stdlib.h>
#include <stdarg.h>
#include <cstring>
#include <iostream>
struct token* compilation::token_at()
{
    return token_at(token_ptr);
}
struct token* compilation::token_at(int token_ptr)
{
    token* tok= nullptr;
    if(token_ptr < (vec_t[0].size()))
        tok = vec_t[0][token_ptr];

    if(!tok)
    {
        ifdm("tok invalid from current pointer, eof \n");
        return nullptr; // EOF
    }

    ifdm("printing the token in single()\n");
    ifd vec_t[0][token_ptr]->print();

    while(tok)
    {
        if( tok->type == TT_Newl||
            tok->type == TT_C   ||
          ( tok->type == TT_Sym && tok->char_val == '\\' )
        )// check newline comment and // 
           if(token_ptr < (vec_t[0].size()-1)){
                tok = vec_t[0][++token_ptr]; 
                ifdm("INC PTR++ \n");
           }
           else // EOF
                return nullptr;
        else break;
    }
    // peek next end
    return tok;
}

void compilation::error_msg(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr," on line %i, col %i in file %s\n", line_no, col_no, path);
    exit(-1);
}
void compilation::warn_msg(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr," on line %i, col %i in file %s\n", line_no, col_no, path);
}
int compilation::compile_file
(
        const char* input_file, 
        const char* output_file,
        int flags
)
{    
    ifile = fopen(input_file,"r");
    ofile = fopen(output_file,"w");
    this->flags = flags;

    if(!ofile)
        return COMPILATION_FAILED;

    // lexical
    auto lex_process = new lexer(this,std::string{});



    if(!lex_process)
        return COMPILATION_FAILED;
    
    if(lex_process->lex() != LEXER_SUCCESS)
        return COMPILATION_FAILED;
    
            std::cout<< "size: " <<(*vec_t).size() << std::endl;
            for(auto x : (*vec_t))
                x->print();
            

    std::cout<<"Lexed successfully ! \n";
    // parsing 
    
    auto parse_process = new parser();
    parse_process->compiler = this;
    vec_n = new std::vector<Node::node*>();
    vec_tree = new std::vector<Node::node*>();
    if(!parse_process)
        return COMPILATION_FAILED;
    if(parse_process->parse() != PARSE_SUCCESS)
        return COMPILATION_FAILED;
    
    std::cout<< "Parsing success \n";

    std::cout<< "Nodes summary: size: "<< vec_n[0].size()<<std::endl;
    for(auto x : vec_n[0])
    {
        std::cout<<"______________\n";
        x->display(0);
        std::cout<<"______________\n";
        
    }


    // code gen

    return COMPILATION_SUCCESS;
}
/*
lexer* compilation::sandbox(std::string& custom)
{
    std::string tmp = custom;
    auto lexp = new lexer(this, strdup(custom.c_str()));
    if(!lexp)
        return NULL; 
    if( lexp->lex() != LEXER_SUCCESS )
        return NULL;
    return lexp;
}*/