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

bool DT::has_datatype_changed(struct datatype* current) 
{

    std::cout<<"[datatype tracker called] \n";
        static struct datatype last = {0};
        static bool first_call = true;

        bool changed = false;

        if (first_call) {
            memcpy(&last, current, sizeof(struct datatype));
            first_call = false;
            printf("[datatype tracker] First-time initialization.\n");
            return true;
        }

        #define CHECK_AND_REPORT(field, fmt) \
            if (last.field != current->field) { \
                printf("[datatype changed] %-12s: " fmt " -> " fmt "\n", #field, last.field, current->field); \
                changed = true; \
            }

        CHECK_AND_REPORT(pdepth, "%d");
        CHECK_AND_REPORT(type, "%d");
        CHECK_AND_REPORT(flags, "%" PRIu16);
        CHECK_AND_REPORT(size, "%" PRIu64);
        CHECK_AND_REPORT(sec, "%p");
        CHECK_AND_REPORT(struct_node, "%p"); // applies to both union_node and struct_node
        if (last.type_str != current->type_str) {
            printf("[datatype changed] %-12s: '%s' -> '%s'\n", "type_str",
                last.type_str ? last.type_str : "(null)",
                current->type_str ? current->type_str : "(null)");
            changed = true;
        }

        if (changed)
            printf("[datatype tracker] Updated internal copy.\n");
        else 
            printf("[datatype tracker] UNCHANGED.\n");

        memcpy(&last, current, sizeof(struct datatype));
        return changed;
    }

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

void DT::datatype::print()
        {
            std::cout<<"pdepth: "<< pdepth <<" type: "<< type<<" \n";
            std::bitset<16> binary(flags);
            std::cout <<"flags: "<< binary << std::endl;
            std::cout<<"size: "<< size <<" \n";
            if(type_str) std::cout<< " type_str: "<< std::string(type_str) <<" \n";
            if(sec != nullptr)
            {
                std::cout<<"secondary: \n\n";
                sec->print();
            }
            else 
                std::cout<<"No secondary: \n\n";
        }

void DT::datatype::parse_datatype_modifiers(compilation* compiler)
{
    auto tok = compiler->token_at();

    ifcm("\tentered parse_mod (100) with token: \n");
    ifc tok->print();
    ifdm("tok: \n");
    ifd tok->print();
    // static const 
    ifcm("\tchecking flags for the consecutive mods \n");
    while(tok && tok->type == TT_KW)
    {
        if( ! DT::is_var_modifier(tok->string_val) )
            break;
        if( strcmp(tok->string_val, "signed") == 0 )
        {
            ifcm("\tapplied sign \n");
            set_flag(DT::flag::IS_SIGNED);
        }
        else if(strcmp(tok->string_val, "unsigned") == 0)
        {
            ifcm("\tapplied unsign \n");
            unset_flag(DT::flag::IS_SIGNED);
        }
        else if(strcmp(tok->string_val, "static") == 0)
        {
            ifcm("\tapplied static \n");
            set_flag(DT::flag::IS_STATIC);
        }
        else if(strcmp(tok->string_val, "const") == 0)
        {
            ifcm("\tapplied cosnt \n");
            set_flag(DT::flag::IS_CONST);
        }
        else if(strcmp(tok->string_val, "extern") == 0)
        {
            ifcm("\tapplied extern \n");
            set_flag(DT::flag::IS_EXTERN);
        }
        else if(strcmp(tok->string_val, "__ignore_typecheck__") == 0)
        {
            ifcm("\tapplied ignorety \n");
            set_flag(DT::flag::IGNORE_TYPE_CHECKING);
        }


        tok = compiler->token_at(++compiler->token_ptr);
        ifcm("\tchecking next token while also moving ptr, tok: \n");
        ifc tok->print();
    }
}

void DT::datatype::parser_datatype_init(compilation* compiler, token* dt1, token* dt2, int stars, int exp_type)
{
    pdepth = stars; // I ADDED
    ifcm("in dt init \n");
    bool flag = false ; // dt1 0 dt2 1
    bool come_back = false;
    if((exp_type != static_cast<int>(DT::expect::primitive_))
    && (dt2) )
        compiler->error_msg("invalid 2nd datatype \n");

    if(exp_type == static_cast<int>(DT::expect::primitive_))
    {
        auto tmp1 = dt1;
        auto tmp2 = this; 
        auto dt_dt2 = new datatype();

        if(flag)
        if(((strcmp(tmp1->string_val, "long") == 0) || 
            (strcmp(tmp1->string_val, "short") == 0) || 
            (strcmp(tmp1->string_val, "double") == 0) || 
            (strcmp(tmp1->string_val, "float") == 0) 
        ) && (dt2))
            compiler->error_msg("can't have %s as sec token\n", tmp1->string_val); // not for 64, some other

        rec_for_dt2:;
            tmp2->type_str = tmp1->string_val;
        if(strcmp(tmp1->string_val, "void") == 0)
        {
            ifc if(flag) std::cout << "\tdt2 type void \n"; 
                else     std::cout << "\tdt1 type void \n";
            tmp2->type = static_cast<int>(DT::type::void_);
            tmp2->size = 0;
        
        }
        else if(strcmp(tmp1->string_val, "char") == 0)
        {
            ifc if(flag) std::cout << "\tdt2 type char \n"; 
                else     std::cout << "\tdt1 type char \n";
            tmp2->type = static_cast<int>(DT::type::char_);
            tmp2->size = 1;
        }
        else if(strcmp(tmp1->string_val, "short") == 0)
        {
            ifc if(flag) std::cout << "\tdt2 type short \n"; 
                else     std::cout << "\tdt1 type short \n";
            tmp2->type = static_cast<int>(DT::type::short_);
            tmp2->size = 2;
        }
        else if(strcmp(tmp1->string_val, "int") == 0)
        {
            ifc if(flag) std::cout << "\tdt2 type int \n"; 
                else     std::cout << "\tdt1 type int \n";
            tmp2->type = static_cast<int>(DT::type::int_);
            tmp2->size = 4;
        }
        else if(strcmp(tmp1->string_val, "long") == 0)
        {
            ifc if(flag) std::cout << "\tdt2 type long \n"; 
                else     std::cout << "\tdt1 type long \n";
            tmp2->type = static_cast<int>(DT::type::long_);
            tmp2->size = 4;
        }
        else if(strcmp(tmp1->string_val, "float") == 0)
        {
            ifc if(flag) std::cout << "\tdt2 type flt \n"; 
                else     std::cout << "\tdt1 type flt \n";
            tmp2->type = static_cast<int>(DT::type::void_);
            tmp2->size = 4;
        }
        else if(strcmp(tmp1->string_val, "double") == 0)
        {
            ifc if(flag) std::cout << "\tdt2 type dble \n"; 
                else     std::cout << "\tdt1 type dble \n";
            tmp2->type = static_cast<int>(DT::type::void_);
            tmp2->size = 4;
        }
        else 
            compiler->error_msg("FATAL ERROR: not a valid primate \n");

        if(come_back)
            goto come_back_here;
        if(dt2 && !flag)
        {
            tmp1 = dt2;
            tmp2 = dt_dt2;

            flag = true;
            come_back = true;
            goto rec_for_dt2;
            come_back_here:;
            ifcm("\tdt2 attaching to dt1 \n");
            this->size += dt_dt2->size;
            this->sec = dt_dt2; 
            set_flag(DT::flag::IS_SECONDARY);
        }
    }
    else if ((exp_type == static_cast<int>(DT::expect::struct_)) || (exp_type == static_cast<int>(DT::expect::union_)))
        compiler->error_msg("structs/unions unsupported for now \n");
    else 
        compiler->error_msg("FATAL ERROR: unnsuported datatyp exp \n");
    
    //this->type_str = dt1->string_val; moving to switch blocks
    if(
        (strcmp(dt1->string_val,"long")==0)
        &&
        (strcmp(dt2->string_val,"long")==0)
    )
        compiler->error_msg("64bits unsupported for now \n");
}   

// int  
// struct x 
// long long
static uint32_t name_giver ;
void DT::datatype::parse_datatype_type(compilation* compiler)
{
    auto dt_tok1 = compiler->token_at();
    compiler->token_ptr++;
    auto dt_tok2 = compiler->token_at();
    ifcm("\tat parse dt type (010) with the 2 tokens: \n");
    ifc if(dt_tok1) dt_tok1->print();
    ifc if(dt_tok2) dt_tok2->print(); else std::cout<<"No sec token \n";

   // if(! dt_tok2) return ;
    if(dt_tok2 && dt_tok2->type == TT_KW &&
        (strcmp(dt_tok2->string_val, "void") == 0 ||
        strcmp(dt_tok2->string_val, "char") == 0 ||
        strcmp(dt_tok2->string_val, "short") == 0 ||
        strcmp(dt_tok2->string_val, "int") == 0 ||
        strcmp(dt_tok2->string_val, "long") == 0 ||
        strcmp(dt_tok2->string_val, "float") == 0 ||
        strcmp(dt_tok2->string_val, "double") == 0)
    )
        compiler->token_ptr++;
    else dt_tok2 = nullptr;

    ifc if(dt_tok2) std::cout << "\tWe got a valid 2nd dt type \n"; 
        else std::cout << "\tWe DIDNOT got a valid 2nd dt type \n";

    int expected_type = static_cast<int>(DT::expect::primitive_) ;
    bool flag = false; // set true in the IF condition itself @details not idiomatic
    if( std::string(dt_tok1->string_val) == "union" && (flag = true))
        expected_type = static_cast<int>(DT::expect::union_);
    else if( std::string(dt_tok1->string_val) == "struct" && (flag = true) )
        expected_type = static_cast<int>(DT::expect::struct_);
    if(flag)
    {
        ifcm("\tIts a struct/union, checking name byy checking tokne: \n");
        auto name = compiler->token_at();        // we now have the data of struct or union in expected type so we can store the string in token (id of sttruct/union)
        ifc name->print();
        if(name->type == TT_ID) compiler->token_ptr++;
        else 
        {
            // struct {} abc; // nameless struct with direct instance. we generate name for our conveniance
            name = new token();
            name->type == TT_ID;
            std::string name_str = "_nameless_" + std::to_string(name_giver++);
            name->string_val = strdup(name_str.c_str());
            set_flag(DT::flag::STRUCT_UNION_NO_NAME);
        }
    }
    ifcm("\tchecking star count, check token first: \n");
    // int** // struct abc*** // long long ** 
    int star_cnt = 0 ;
    auto tok = compiler->token_at();
    ifc tok->print();
    while (
        (tok) && (tok->type == TT_OP) && 
    ( strcmp(tok->string_val,"*")==0 ))
    {   
        tok = compiler->token_at(++compiler->token_ptr); 
        ifcm("\tSTAR++ ");
        star_cnt++;
    }

    ifc std::cout << "\n\tstar cnt: "<< star_cnt<< std::endl;
    parser_datatype_init(compiler,dt_tok1, dt_tok2,star_cnt,expected_type);
}
void DT::datatype::parse(compilation* compiler)
{

    ifcm("\tentered parse of actual dt \n");
    memset(this,0,sizeof(datatype));
    //set_flag(DT::flag::IS_SIGNED);

    ifdm("starting series mod-type-mod \n");
    // static int const a = 5 ;
    parse_datatype_modifiers(compiler);
    ifdm("in series mod-type-mod, first mod done \n");

    std::cout<<"_\n";
    has_datatype_changed(this);
    std::cout<<"_\n";


    parse_datatype_type(compiler);
    ifdm("in series mod-type-mod, type done \n");

    std::cout<<"_\n";
    has_datatype_changed(this);
    std::cout<<"_\n";

    parse_datatype_modifiers(compiler);
    ifdm("in series mod-type-mod, second mod done \n");


    std::cout<<"_\n";
    has_datatype_changed(this);
    std::cout<<"_\n";

    print();
}
void record::parse_var_func_struct_union()
{
    //std::cout<<"pvfsu entry \n";
    ifcm("\tentered p_v_f_s_u \n");
    auto dt = new DT::datatype(); 
    dt->set_flag(DT::flag::IS_SIGNED);
    has_datatype_changed(dt);
    dt->parse(compiler);
    // dt here ?
    //std::cout<<"pvfsu exit \n";
}

void record::parse_kw(token* tok)
{
    ifcm("\tentered parse_kw with token: \n");
    ifc tok->print();
    if( DT::is_var_modifier(tok->string_val) ||
    DT::is_datatype(tok->string_val) )
    {
        ifcm("\tthe token must be a mod or dt \n");
        parse_var_func_struct_union();
       /* if(compiler)
        {
            std::cout<<"compiler alive \n";
            if(compiler->vec_n)
                std::cout<<"compiler->vcn alive with size"<<compiler->vec_n[0].size() <<"\n";            
        }
        else std::cout<<"compiler lost \n";

        std::cout<<"parsing kw exit2 \n";*/
        return ;
    }
    //std::cout<<"parsing kw exit1 \n";
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
    auto r = new record(this->compiler, flags);
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
    ifdm("entered parse_expressionable_single \n");
    // peek next 
    token* tok = compiler->token_at();
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

void record::parse_kw_for_global()
{
    ifdm("entered parse kw for flobal \n");

    ifcm("\tentered parse_kw_for_global\n");
    parse_kw(compiler->token_at(compiler->token_ptr));
    // at this point we have a node ?

}

int parser::parse_next()
{
    if(compiler->token_ptr >= compiler->vec_t[0].size())
    {
        ifdm("EOF ! \n");
        return 0;
    }
    ifd std::cout << "in parse_next() \n";
    auto tok = compiler->token_at();
    if(!tok)
        return 0;
    int res = 0;
    auto rec = new record(0);
    rec->compiler = compiler;
    switch(tok->type)
    {
        case TT_Num:    
        case TT_ID:
        case TT_Str:

            ifdm("calling parse_expressionable()\n");
            rec->parse_expressionable();
            break; 

        case TT_KW:
            rec->parse_kw_for_global();
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
    name_giver = 0;
    sucide = 0; 
    ifd std::cout << "parse_next() calling \n";
    while(parse_next())
    {

        if(!compiler->vec_n[0].size())
            break;

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