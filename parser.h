

#ifndef _parse
#define _parse

#include "compiler.h"
#include "node.h"

#include <vector>
#include <string>




enum 
{
    PARSE_SUCCESS,
    PARSE_FAILED
};

struct record 
{
    compilation* compiler;
    int flags;
    record(int f = 0)
    {
        compiler = nullptr;
        flags = f ;
    }
    record(compilation* c, int f =0)
    {
        compiler = c;
        flags =f ;
    }
    int parse_expressionable_single();
    void parse_expressionable();
    void parse_expressionable_for_op(const char* op);
    void parse_exp_normal(token* tok);
    int parse_exp(token* tok);
    void parse_kw(token* tok);
    void parse_var_func_struct_union();
    record * clone(int flags);
};

namespace DT 
{
    enum class flag : uint16_t {
        IS_SIGNED               = 0x01,
        IS_STATIC               = 0x02,
        IS_CONST                = 0x04,
        IS_POINTER              = 0x08,
        IS_ARRAY                = 0x10,
        IS_EXTERN               = 0x20,
        IS_RESTRICT             = 0x40,
        IGNORE_TYPE_CHECKING    = 0x80,
        IS_SECONDARY            = 0x100,
        STRUCT_UNION_NO_NAME    = 0x200,
        IS_LITERAL              = 0x400
    };
    struct datatype 
    {
        int pdepth,type; // can be int or float or long 
        uint16_t flags; // refer enum class DT 
        size_t size;
        struct datatype* sec; // long long
        const char* type_str ;
        union 
        {
            struct node* struct_node, *union_node ;
        };
        void parse(compilation* c);
        bool check_flag(DT::flag f);
        void set_flag(DT::flag f);
        void unset_flag(DT::flag f);
        void parse_datatype_modifiers(compilation* c);
        void parse_datatype_type(compilation* c);
    };
    enum class type
    {
        void_,
        char_,
        short_,
        int_,
        long_,
        double_,
        struct_,
        union_,
        unknown_
    };
    bool is_datatype(const char* str);
    bool is_var_modifier(const char* str);
};



struct parser {

    compilation* compiler;

    parser() {
        compiler = nullptr;
    }

    ~parser() {
        if(compiler)
        {
            for (Node::node* n : *compiler->vec_n) delete n;
            for (Node::node* n : *compiler->vec_tree) delete n;
        }

        delete compiler->vec_n;
        delete compiler->vec_tree;
    }

    int parse();
    int parse_next();
};

const int left_to_right = 0;
const int right_to_left = 1;

struct parse_priority {
    std::vector<std::string> operators;
    int direction;
};





#endif