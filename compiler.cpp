#include "compiler.h"
#include "lexer.h"
#include <memory>
#include <stdlib.h>
#include <stdarg.h>
#include <cstring>
#include <iostream>

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
            {
                switch(x->type)
                {
                    case TT_Num:
                        std::cout<<"num "<<x->ull_val<<std::endl;
                        break;
                    case TT_Str:
                        std::cout<<"str "<<x->string_val<<std::endl;
                        break;
                    case TT_OP:
                        std::cout<<"op "<<x->string_val<<std::endl;
                        break;
                    case TT_Sym:
                        std::cout<<"sym "<<x->char_val<<std::endl;
                        break;
                    case TT_ID:
                        std::cout<<"id "<<x->string_val<<std::endl;
                        break;
                    case TT_KW:
                        std::cout<<"kw "<<x->string_val<<std::endl;
                        break;
                    case TT_Newl:
                        std::cout<<"new line \n";
                        break;
                    case TT_C:
                        std::cout<<"comm "<<x->string_val<<std::endl;
                        break;
                }

            }
    // parsing 

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