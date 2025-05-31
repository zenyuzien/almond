#ifndef _almond 
#define _almond 

#include "node.h"

#include <string>
#include <stdio.h>
#include <cstdint>
#include <cstdlib>
#include <vector>


extern bool debug_parse ;
extern bool custom_parse;
#define ifd if(debug_parse)
#define ifc if(custom_parse)
#define ifdm(msg) do { if (debug_parse) std::cout << msg ; } while (0)
#define ifcm(msg) do { if (custom_parse) std::cout << msg ; } while (0)

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
    //struct token* next_token(compilation* , int);
};

// status 
enum
{
    COMPILATION_SUCCESS,
    COMPILATION_FAILED
};

struct compilation;

namespace SYM 
{
    enum class type 
    {
        node     = 0b0001,
        native_f = 0b0010,
        undef    = 0b0100
    };
    struct symbol
    {
        std::string name; 
        int type; 
        void* metadata;
    };
    struct symbol_resolver
    {
        compilation* compiler; 
        void init();
        void push(symbol* s);
        void new_table();
        void end_table();
        symbol* get(std::string& name);
        symbol* get_for_nf(std::string& name);
        symbol* make_symbol(const char* name, int type, void* content);
        Node::node* node(symbol* sym);

        /*
            building for variables, functions, structures, unions 
            pending
        */
        void build_for_node(Node::node* node);
    };
};



struct scope;
struct compilation
{
    int line_no , col_no , flags; 
    int token_ptr; 

    int scope_ptr; 
    scope* root_scope, *active_scope; 

    FILE *ifile, *ofile;
    const char* path; 
    std::vector<token*>* vec_t;
    std::vector<Node::node*> *vec_n, *vec_tree;

    std::vector<SYM::symbol*>* sym_table; 
    std::vector< std::vector<SYM::symbol*>* >* sym_table_table; 

    SYM::symbol_resolver* sym_resolver; 
    int sym_ptr;

    compilation() : 
        line_no(1),
        col_no(1),
        flags(0),
        token_ptr(0),
        scope_ptr(0),
        root_scope(nullptr),
        active_scope(nullptr),
        ifile(nullptr),
        ofile(nullptr),
        path(nullptr),
        sym_table(nullptr)
        //vec_t(new std::vector<token*>()),
        //vec_n(new std::vector<Node::node*>()),
        //vec_tree(new std::vector<Node::node*>()),
        //sym_table_table(new std::vector<std::vector<symbol*>*>())
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
    struct token* token_at();
    struct token* token_at(int ptr);
    //lexer* sandbox(std::string& custom);

    scope* root_scope_create(bool create);
    scope* new_scope(int flags);
    void finish_scope();
    void push_scope(void* address, size_t size);
    void* scope_from_to(scope* s, scope* e);
    void* scope_last_instance();

};

struct scope
{
    bool dec; // may remove;
    int flags, ptr; 
    std::vector<void*>* instances;
    size_t size;  
    scope* parent;
    compilation* compiler;

    scope()
        : ptr(0), flags(0), size(0), parent(nullptr), compiler(nullptr)
    {
        instances = new std::vector<void*>();
        dec = true; // LOOK HERE
    }

    ~scope() {
       // delete instances;
    }
  
    void iterate(bool start);
    void* instance_at(int index);
    void* top();
};

#endif