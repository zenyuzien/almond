#include "parser.h"
#include <iostream>
#include "sleep.h"
#include <memory.h>

#define gap(level) for(int i = 0 ; i < level ; i++) std::cout << "  ";

#define EXP_CASE            \
    case Node::exp_ :       \
    case Node::exp_bracket_:\
    case Node::unary_:      \
    case Node::id_:         \
    case Node::number_:     \
    case Node::string_

std::vector<parse_priority> op_precedence = {
    {{"++", "--", "()", "[]", "(", "[", ".", "->"}, left_to_right},
    {{"*", "/", "%"}, left_to_right},
    {{"+", "-"}, left_to_right},
    {{"<<", ">>"}, left_to_right},
    {{"<", "<=", ">", ">="}, left_to_right},
    {{"==", "!="}, left_to_right},
    {{"&"}, left_to_right},
    {{"^"}, left_to_right},
    {{"|"}, left_to_right},
    {{"&&"}, left_to_right},
    {{"||"}, left_to_right},
    {{"?", ":"}, right_to_left},
    {{"=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^=", "|="}, right_to_left},
    {{","}, left_to_right}
};

bool DT::is_datatype(const char* str)
{
    return strcmp(str, "void") == 0   ||
           strcmp(str, "char") == 0   ||
           strcmp(str, "int") == 0    ||
           strcmp(str, "short") == 0  ||
           strcmp(str, "float") == 0  ||
           strcmp(str, "double") == 0 ||
           strcmp(str, "long") == 0   ||
           strcmp(str, "struct") == 0 ||
           strcmp(str, "union") == 0;
}
bool DT::is_var_modifier(const char* str)
{
    return strcmp(str, "unsigned") == 0 ||
           strcmp(str, "signed") == 0 ||
           strcmp(str, "static") == 0 ||
           strcmp(str, "const") == 0 ||
           strcmp(str, "extern") == 0 ||
           strcmp(str, "__ignore_typecheck__") == 0;
}

void DT::datatype::set_flag(DT::flag f) {
    flags |= static_cast<uint16_t>(f);
}
void DT::datatype::unset_flag(DT::flag f) {
    flags &= ~static_cast<uint16_t>(f);
}
bool DT::datatype::check_flag(DT::flag f) {
    return (flags & static_cast<uint16_t>(f)) != 0;
}

void DT::datatype::parse_datatype_modifiers(compilation* compiler)
{
    auto tok = compiler->token_at(compiler->token_ptr);
    while(tok && tok->type == TT_KW)
    {
        if( ! DT::is_var_modifier(tok->string_val) )
            break;
        if( tok->string_val == "signed" )
            set_flag(DT::flag::IS_SIGNED);
        else if(tok->string_val == "unsigned")
            unset_flag(DT::flag::IS_SIGNED);
        else if(tok->string_val == "static")
            set_flag(DT::flag::IS_STATIC);
        else if(tok->string_val == "const")
            set_flag(DT::flag::IS_CONST);
        else if(tok->string_val == "extern")
            set_flag(DT::flag::IS_EXTERN);
        else if(tok->string_val == "__ignore_typecheck__")
            set_flag(DT::flag::IGNORE_TYPE_CHECKING);
    }
    tok = compiler->token_at(++compiler->token_ptr);
}
void DT::datatype::parse_datatype_type(compilation* compiler)
{
    ;
}

void DT::datatype::parse(compilation* compiler)
{
    memset(this,0,sizeof(datatype));
    set_flag(DT::flag::IS_SIGNED);

    // static int const a = 5 ;
    parse_datatype_modifiers(compiler);
    parse_datatype_type(compiler);
    parse_datatype_modifiers(compiler);
}
void record::parse_var_func_struct_union()
{
    struct DT::datatype dt; 
    dt.parse(compiler);
}

void record::parse_kw(token* tok)
{
    if( DT::is_var_modifier(tok->string_val) ||
    DT::is_datatype(tok->string_val) )
    {
        parse_var_func_struct_union();
        return ;
    }
}


int get_priority_for(const char* op )
{
    for(int i = 0 ; i < op_precedence.size(); i++)
        for(auto j: op_precedence[i].operators )
            if(op == j)
                return i;
    return -1;
}
bool should_we_eval_left(const char* left, const char* right)
{
    if(left==right) return false; 
    int left_rank = get_priority_for(left);
    int right_rank = get_priority_for(right);
    
    if( op_precedence[left_rank].direction == right_to_left )
        return false;

    return left_rank <= right_rank;
}

void Node::node::node_shift_children_left()
{
    //ip num1 * (num2 + num3)
    //op (num1 * num2) + num3
    if(type != Node::exp_ || (expunion.exp.right->type != Node::exp_))
    {
        ifdm("can't shift left: invalid op \n");
        exit(-1);
    }
    const char* right_op = expunion.exp.right->expunion.exp.op;
    Node::node* new_leftchild = expunion.exp.left; 
    Node::node* new_rightchild = expunion.exp.right->expunion.exp.left; 
    
    Node::node* new_leftnode = new Node::node();
    new_leftnode->type= Node::exp_ ; 
    new_leftnode->expunion.exp.left = new_leftchild;
    new_leftnode->expunion.exp.right = new_rightchild;
    new_leftnode->expunion.exp.op = expunion.exp.op;

    Node::node* new_rightnode = expunion.exp.right->expunion.exp.right;
    
    expunion.exp.left = new_leftnode;
    expunion.exp.right = new_rightnode;
    expunion.exp.op = right_op;
}

void Node::node::reorder_expression()
{
    ifdm("called reorder exp \n");
    // check if node is exp, and check if there's an exp in atleast one child
    if (type != Node::exp_)
    {
        ifdm("it is not exp, returning \n");
        return;
    }

    auto left = expunion.exp.left;
    auto right = expunion.exp.right;

    bool left_is_exp = left && (left->type == Node::exp_);
    bool right_is_exp = right && (right->type == Node::exp_);

    if (!left_is_exp && !right_is_exp)
    {
        ifdm("no child is exp, so returning \n");
        return;
    }

    if (!left_is_exp && right_is_exp) 
    {
        // left op, right op  a*(b+c)
        ifdm("checking if we eval left \n");
        if(should_we_eval_left(expunion.exp.op, expunion.exp.right->expunion.exp.op))
        {
            ifdm("we r to eval left, so lets print before first \n");   
            ifd display(0);     
            node_shift_children_left();
            ifdm("printing after \n");
            ifd display(0);
            if (expunion.exp.left) expunion.exp.left->reorder_expression();
            if (expunion.exp.right) expunion.exp.right->reorder_expression();
        }
    }
}

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
    compiler->token_ptr++;
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
    exp->reorder_expression();
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
    ifdm("entered parse_expressionable_single \n");
    // peek next 
    token* tok = compiler->token_at(compiler->token_ptr);
    if(!tok)
        return -1;
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
            compiler->token_ptr++;
            res=0;
        break;

        case TT_ID:
            // parse identifier
            n = new Node::node();
            n->push(compiler->vec_n);
            ifdm("pusehd to vecn \n");
            n->type = Node::id_;
            n->val.string_val = tok->string_val; 
            ifdm("INC PTR++ \n");
            compiler->token_ptr++;
            res=0;     
        break;

        case TT_OP:
            parse_exp(tok);
            res=0;
        break;

        case TT_KW:
            parse_kw(tok); 
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
    if(compiler->token_ptr >= compiler->vec_t[0].size())
    {
        ifdm("EOF ! \n");
        return 0;
    }
    ifd std::cout << "in parse_next() \n";
    auto tok = compiler->token_at(compiler->token_ptr);
    if(!tok)
        return 0;
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
    compiler->token_ptr=0; // it will point to the next token that is unprocessed
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
void Node::node::display(int level)
{
    if(type == exp_)
    {

        gap(level);
        std::cout<<"It is an expression Node with op: "<< expunion.exp.op << std::endl ;
        gap(level);
        std::cout<< "Left node: \n";
        expunion.exp.left->display(level+1);
        gap(level);
        std::cout<<"right node: \n";
        expunion.exp.right->display(level+1);
    }
    else if(type == number_)
    {
        gap(level);
        std::cout<<"number val: "<< val.ull_val << std::endl;
    }
}