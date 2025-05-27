#ifndef _node 
#define _node 
#include <vector>

enum 
{
    NODE_FLAG_INSIDE_EXPRESSION = 0b00000001
};
namespace Node
{
    enum 
    {
        exp_, 
        exp_bracket_,
        number_, 
        id_, 
        string_,
        var_,
        varlist_,
        func_,
        body_,
        return_,
        if_,
        else_,
        while_,
        for_,
        break_,
        continue_,
        switch_,
        case_,
        default_,
        goto_,
        unary_,
        ternary_,
        label_,
        struct_,
        union_,
        bracket_,
        cast_,
        blank_
    };
    struct node 
    {
        int type, flags; 
        int row_no, col_no;
        const char* path; 
        struct node_binded
        {
            node* head = nullptr; // body
            node* func = nullptr; // function

            node_binded() = default;
            ~node_binded() = default;
        }binded;// as a pointer ?
        union 
        {
            struct expression
            {
                Node::node *left,*right;
                const char*op;
            }exp;
        }expunion;
        union 
        {
            char char_val; 
            char* string_val;
            unsigned int integer_val ; 
            unsigned long long_val; 
            unsigned long long ull_val ;
            void* any; 
        }val; // val not in peach
        node()
            : type(0), flags(0), row_no(0), col_no(0), path(nullptr)
        {
            // binding here required
            // push node ? to where ?
            val.any = nullptr;  // safely init union
        }

        // Destructor
        ~node()
        {
            // If you use string_val or dynamically allocated memory, free it here
            // delete[] value.string_val; // Uncomment only if memory is owned by node
        }


        // implementatios of these can be found in parser.cpp
        void push(std::vector<node*>* v);
        
        
        // idk what this node_set_vector is trying to do
    };
    node* top(std::vector<node*>* v);
    node* pop(std::vector<node*>* v);
};

#endif 