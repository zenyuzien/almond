

#ifndef _parse
#define _parse

#include "compiler.h"
#include "node.h"
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
    record * clone(int flags);
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





#endif