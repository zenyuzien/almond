#include "parser.h"
#include <iostream>
#include "sleep.h"

#define debug_parse 0
#define ifd if(debug_parse)
#define ifdm(msg) do { if (debug_parse) std::cout << msg << std::endl; } while (0)

#define EXP_CASE            \
    case Node::exp_ :       \
    case Node::exp_bracket_:\
    case Node::unary_:      \
    case Node::id_:         \
    case Node::number_:     \
    case Node::string_

static int token_ptr;
record* record::clone(int flags)
{
    auto r = new record(flags);
    r->compiler = this->compiler;
    return r;
}
void record::parse_expressionable_for_op(const char* op)
{
    ifdm("in parseexpressionableforop() \n");
    parse_expressionable();
}
void record::parse_exp_normal(token* tok)
{
    ifdm("in exp_normal() \n");
    const char* op = tok->string_val;
    Node::node* left ;
    if(compiler->vec_n[0].size())
        left = compiler->vec_n[0].back();
    ifdm("left node: \n");
    ifd std::cout << left->val.ull_val <<std::endl;
    switch (left->type)
    {
        EXP_CASE:
        break;
        default:
            left = nullptr;
            return;
        break;
    }
    ifdm("INC PTR++ \n");
    token_ptr++;
    Node::pop(compiler->vec_n);
    left->flags |= NODE_FLAG_INSIDE_EXPRESSION; 
    
    auto rec1 = this->clone(this->flags);
    rec1->parse_expressionable_for_op(op);
    Node::node* right = Node::pop(compiler->vec_n);
    if(right)
        right->flags |= NODE_FLAG_INSIDE_EXPRESSION ;
    else {
        std::cout<<"FATAL ERROR \n";
        exit(1);
    }
    ifdm("right node: \n");
    ifd std::cout << right->val.ull_val <<std::endl;
    auto exp = new Node::node();
    exp->type = Node::exp_ ;
    //exp->expunion.exp
    exp->expunion.exp.left = left;
    exp->expunion.exp.right = right ;
    exp->expunion.exp.op = op;
    exp->push(compiler->vec_n);
}

int record::parse_exp(token* tok)
{
    ifdm("ENTERED PARSE EXP WITH TOK \n");
         ifd std::cout<< "***"<<compiler->vec_t[0][2]->type<< std::endl;

    ifd tok->print();
    parse_exp_normal(tok);
    return 0;
}
static int sucide;
int record::parse_expressionable_single()
{
         ifd std::cout<< "***"<<compiler->vec_t[0][2]->type<< std::endl;

        if(++sucide == 7)
        {
            std::cout<<"alla hu akbar ! \n";
            exit(1);
        }
    ifdm("entered parse_expressionable_single \n");
    // peek next 
    token* tok= nullptr;

    if(token_ptr < (compiler->vec_t[0].size()))
        tok = compiler->vec_t[0][token_ptr];

    if(!tok)
    {
        ifdm("tok invalid from current pointer, eof \n");
        return -1; // EOF
    }

    ifdm("printing the token in single()___________ \n");
    ifd compiler->vec_t[0][token_ptr]->print();
    ifdm("___________ \n");

    ifdm("printing the token in single()___________ \n");
    ifd tok->print();
    ifdm("___________ \n");


    while(tok)
    {
        if( tok->type == TT_Newl||
            tok->type == TT_C   ||
          ( tok->type == TT_Sym && tok->char_val == '\\' )
        )// check newline comment and // 
           if(token_ptr < (compiler->vec_t[0].size()-1)){
                tok = compiler->vec_t[0][++token_ptr]; 
                ifdm("INC PTR++ \n");
           }
           else // EOF
                return -1;
        else break;

    }
    ifdm("after check of newl comm escape. printing the token jic \n");
    ifd tok->print();
    // peek next end
    flags |= NODE_FLAG_INSIDE_EXPRESSION;
    int res=-1;
    Node::node* n ; 
     // token next
    switch(tok->type)
    {
        case TT_Num:
            n = new Node::node();
            n->push(compiler->vec_n);
            ifdm("pusehd to vecn \n");
            n->type = Node::number_;
            n->val.ull_val = tok->ull_val; 
            ifdm("INC PTR++ \n");
            token_ptr++;
            res=0;
        break;

        case TT_OP:
            parse_exp(tok);
            res=0;
        break;

        default:
            std::cout<<"not handled this yet! \n";
        break;
    }
    
    ifdm("returning res ");
    ifd std::cout<< res <<std::endl;
    return res;
    // at this point we are pointing to a valid token 
    // ptr is not yet incremented which is to be noted

}

void record::parse_expressionable()
{
    ifdm("entered parse expressionable, it will check the record's parse_expressionable_single \n");

    while(record::parse_expressionable_single() ==0)
    {
     ifd std::cout<< "***"<<compiler->vec_t[0][2]->type<< std::endl;
        ifdm("inside while loop of parse_expressionable \n");
    }
}

int parser::parse_next()
{
    ifd std::cout << "in parse_next() \n";
    auto tok = compiler->vec_t[0][token_ptr]; 
    ifd {
        std::cout <<"received a token \n";
        if(tok)
            tok->print(); 
    }
    if(!tok)
        return 0;
    //std::cout<<"token: ptr: "<< token_ptr-1 <<" type: "<< tok->type << std::endl;
    while(tok &&
        ( tok->type == TT_Newl||
          tok->type == TT_C ||
          ( tok->type == TT_Sym && tok->char_val == '\\' )
        )
    )
    {
        std::cout<<"its one of the newl, comm, escape so skipping, also INC PTR++\n";
        //std::cout<<"token skip to \n";
        tok = (token_ptr < (compiler->vec_t[0].size() -1 )) ? compiler->vec_t[0][++token_ptr]: nullptr;
        if(tok)
            ifd {
                std::cout<<" so got other \n";
                tok->print();
            }
       // std::cout<<"token: ptr: "<< token_ptr-1 <<" type: "<< tok->type << std::endl;
    }
    int res = 0;
    auto rec = new record(0);
    rec->compiler = compiler;
    switch(tok->type)
    {
        case TT_Num:
    //z        n->type = Node::number_;
    //z        n->val.ull_val = tok->ull_val;   
    //z        break;
    
        case TT_ID:
    //z        n->type = Node::id_;
    //z        n->val.string_val = tok->string_val; 
    //z        break; 

        case TT_Str:
    //z        n->type = Node::string_;
    //z        n->val.string_val = tok->string_val;
    //z        break; 

        ifdm("calling parse_expressionable()\n");
        rec->parse_expressionable();
        break; 

        default:   
            compiler->error_msg("this token cant be conv to node \n");
        break;
            
    }
    return 1;
}
int parser::parse()
{
    ifd std::cout<< "started parsing \n";
    Node::node* node= nullptr; 
    token* last = nullptr;
    token_ptr=0; // it will point to the next token that is unprocessed
    sucide = 0; 
    ifd std::cout << "parse_next() calling \n";
    while(parse_next())
    {
        ifd std::cout << "parse_next() calling \n";
        node = compiler->vec_n[0].back();
        compiler->vec_tree->push_back(node);
    }  
    return PARSE_SUCCESS;
}
void Node::node::push(std::vector<Node::node*>* v)
{
    v->push_back( this );
}
Node::node* Node::top(std::vector<Node::node*>* v)
{
    if(v->size())
        return v->back();
    return nullptr; 
}
Node::node* Node::pop(std::vector<Node::node*>* v)
{
    if(v->size())
    {
        auto x = v->back();
        v->pop_back();
        return x; 
    }
    return nullptr;
}
       