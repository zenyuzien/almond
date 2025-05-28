#ifndef _almond 
#define _almond 

#include "node.h"

#include <stdio.h>
#include <cstdint>
#include <cstdlib>
#include <vector>


#define debug_parse 1
#define ifd if(debug_parse)
#define ifdm(msg) do { if (debug_parse) std::cout << msg << std::endl; } while (0)

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

struct compilation; 
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
    void print();
    struct token* next_token(compilation* , int);
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
    int token_ptr; 
    FILE *ifile, *ofile;
    const char* path; 
    std::vector<token*>* vec_t;
    std::vector<Node::node*> *vec_n, *vec_tree;
    compilation() : 
        token_ptr(0),
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
    struct token* token_at(int ptr);
    //lexer* sandbox(std::string& custom);
};

#endif