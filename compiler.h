#ifndef _almond 
#define _almond 

#include <stdio.h>
#include <cstdint>
#include <cstdlib>
#include <vector>
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
    int num_type; 
    int row_no, col_no ;
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
    token()
        : type(0), num_type(0), flags(0), row_no(0), col_no(0),
          ull_val(0), 
          space_next(false), bracket_grp(nullptr) {
          }

    ~token() {
        // If using string_val, delete if allocated
        // Assumes string_val was allocated with new or strdup
        if (type == TT_Str && string_val) {
            free(string_val);  // or delete[] string_val if you used new[]
            string_val = nullptr;
        }
    }

};

// status 
enum
{
    COMPILATION_SUCCESS,
    COMPILATION_FAILED
};

struct compilation
{
    int line_no , col_no , flags; 
    FILE *ifile, *ofile;
    const char* path; 
    std::vector<token*>* vec_t; 
    compilation() :
        line_no(1), col_no(1), flags(0),
        ifile(nullptr), ofile(nullptr),
        path(nullptr)
    {}

    ~compilation() {
        if (ifile) fclose(ifile);
        if (ofile) fclose(ofile);
    }
    int compile_file(
        const char* input_file, 
        const char* output_file,
        int flags
    );
    void error_msg(const char* msg, ...);
    void warn_msg(const char *msg, ...);
    //lexer* sandbox(std::string& custom);
};

#endif