#ifndef _almond 
#define _almond 

#include <stdio.h>
#include <cstdint>

enum // token classes
{
    TT_ID, // identifier
    TT_KW, // keyword
    TT_OP,  // operator 
    TT_Sym,  // symbol 
    TT_Num, // number
    TT_Str, // string
    TT_C,   // comment 
    TT_Newl   // new line
};

struct token
{
    int type;
    int flags; 

    union 
    {
        char char_val; 
        char* string_val;
        uint32_t integer_val ; 
        unsigned long long_val; 
        unsigned long long ull_val ;
        void* any; 
    };

    bool space_next; 
    const char* bracket_grp ;

};

enum
{
    COMPILATION_SUCCESS,
    COMPILATION_FAILED
};

struct compilation
{
    int flags; 
    FILE *ifile, *ofile;
    const char* path; 
    int compile_file(
        const char* input_file, 
        const char* output_file,
        int flags
    );
};

#endif