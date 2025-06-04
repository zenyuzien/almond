//#include "parser.h"
#include "compiler.h"
#include <iostream>
#include <memory.h>
#include <iomanip>

static int gaper;

extern bool PARSER_DEBUG ;
extern std::ofstream parserDebugger; 

#define gap(level) for(int i = 0 ; i < level ; i++) std::cout << "  ";
#define gapd(level) for(int i = 0 ; i < level ; i++) parserDebugger << "  ";
#define ifdp if(PARSER_DEBUG) for(int i = 0, lim = (gaper>15?15:gaper) ; i < lim ; i++) parserDebugger << "  "; if(PARSER_DEBUG) 
#define ifdpm(msg) if(PARSER_DEBUG) { for(int i = 0, lim = (gaper>15?15:gaper) ; i < lim ; i++) parserDebugger << "  "; parserDebugger<< msg; }

#define IS_EXPRESSION_ABLE  \
    case Node::exp_ :       \
    case Node::exp_bracket_:\
    case Node::unary_:      \
    case Node::id_:         \
    case Node::number_:     \
    case Node::string_

// a small gdb- like feature to track changes to struct data_type, purely for debugging purpose
bool DT::hasDatatypeChanged(struct datatype* current) 
{
    parserDebugger<<"[datatype tracker called] \n";
        static struct datatype last = {0};
        static bool firstCall = true;

        bool changed = false;

        if (firstCall) {
            memcpy(&last, current, sizeof(struct datatype));
            firstCall = false;
            ifdpm("[datatype tracker] First-time initialization.\n");
            return true;
        }

        #define CHECK_AND_REPORT(field, fmt) \
            if (last.field != current->field) { \
                parserDebugger << "[datatype changed] " << std::left << std::setw(12) << #field << ": " \
                            << last.field << " -> " << current->field << "\n"; \
                changed = true; \
            }

        CHECK_AND_REPORT(pdepth, "%d");
        CHECK_AND_REPORT(type, "%d");
        CHECK_AND_REPORT(flags, "%" PRIu16);
        CHECK_AND_REPORT(size, "%" PRIu64);
        CHECK_AND_REPORT(sec, "%p");
        CHECK_AND_REPORT(structNode, "%p"); // applies to both union_node and struct_node
        if (last.typeStr != current->typeStr) {
            parserDebugger << "[datatype changed] "
            << std::left << std::setw(12) << "typeStr" << ": '"
            << (last.typeStr ? last.typeStr : "(null)") << "' -> '"
            << (current->typeStr ? current->typeStr : "(null)") << "'\n";
            changed = true;
        }


        if (changed)
            ifdpm("[datatype tracker] Updated internal copy.\n")
        else 
            ifdpm("[datatype tracker] UNCHANGED.\n");

        memcpy(&last, current, sizeof(struct datatype));
        return changed;
    }

// This structure stores the priority table of all operators in C
const std::vector<parsePriority> opPrecedence = {
    {{"++", "--", "()", "[]", "(", "[", ".", "->"}, leftToRight},
    {{"*", "/", "%"}, leftToRight},
    {{"+", "-"}, leftToRight},
    {{"<<", ">>"}, leftToRight},
    {{"<", "<=", ">", ">="}, leftToRight},
    {{"==", "!="}, leftToRight},
    {{"&"}, leftToRight},
    {{"^"}, leftToRight},
    {{"|"}, leftToRight},
    {{"&&"}, leftToRight},
    {{"||"}, leftToRight},
    {{"?", ":"}, rightToLeft},
    {{"=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^=", "|="}, rightToLeft},
    {{","}, leftToRight}
};

bool DT::isDatatype(const char* str)
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
bool DT::isTypeModifier(const char* str)
{
    return strcmp(str, "unsigned") == 0 ||
           strcmp(str, "signed") == 0   ||
           strcmp(str, "static") == 0   ||
           strcmp(str, "const") == 0    ||
           strcmp(str, "extern") == 0   ||
           strcmp(str, "__ignore_typecheck__") == 0;
}

void DT::datatype::setFlag(DT::flag f) {
    flags |= static_cast<uint16_t>(f);
}
void DT::datatype::unsetFlag(DT::flag f) {
    flags &= ~static_cast<uint16_t>(f);
}
bool DT::datatype::checkFlag(DT::flag f) {
    return (flags & static_cast<uint16_t>(f)) != 0;
}

void DT::datatype::print(bool isdebug)
        {
            if(isdebug)
            {
                ifdp parserDebugger <<"pdepth: "<< pdepth <<" type: "<< type<<" \n";
                std::bitset<16> binary(flags);
                parserDebugger <<"flags: "<< binary << std::endl;
                parserDebugger <<"size: "<< size <<" \n";
                if(typeStr) parserDebugger << " typeStr: "<< std::string(typeStr) <<" \n";
                if(sec != nullptr)
                {
                    parserDebugger <<"secondary: \n\n";
                    sec->print();
                }
                else 
                    parserDebugger <<"No secondary: \n\n";
                return ;
            }

            std::cout<<"pdepth: "<< pdepth <<" type: "<< type<<" \n";
            std::bitset<16> binary(flags);
            std::cout <<"flags: "<< binary << std::endl;
            std::cout<<"size: "<< size <<" \n";
            if(typeStr) std::cout<< " typeStr: "<< std::string(typeStr) <<" \n";
            if(sec != nullptr)
            {
                std::cout<<"secondary: \n\n";
                sec->print();
            }
            else 
                std::cout<<"No secondary: \n\n";
        }

void DT::datatype::parseDatatypeModifiers(compilation* compiler)
{
    auto tok = compiler->tokenAt();

    ifdpm("\tentered parseMod (100) with token: \n");
    ifdp tok->print(parserDebugger);
    // static const 
    ifdpm("\tchecking flags for the consecutive mods \n");
    while(tok && tok->type == static_cast<int>(Token::type::KW))
    {
        if( ! DT::isTypeModifier(tok->stringVal) )
            break;
        if( strcmp(tok->stringVal, "signed") == 0 )
        {
            ifdpm("\tapplied sign \n");
            setFlag(DT::flag::IS_SIGNED);
        }
        else if(strcmp(tok->stringVal, "unsigned") == 0)
        {
            ifdpm("\tapplied unsign \n");
            unsetFlag(DT::flag::IS_SIGNED);
        }
        else if(strcmp(tok->stringVal, "static") == 0)
        {
            ifdpm("\tapplied static \n");
            setFlag(DT::flag::IS_STATIC);
        }
        else if(strcmp(tok->stringVal, "const") == 0)
        {
            ifdpm("\tapplied const \n");
            setFlag(DT::flag::IS_CONST);
        }
        else if(strcmp(tok->stringVal, "extern") == 0)
        {
            ifdpm("\tapplied extern \n");
            setFlag(DT::flag::IS_EXTERN);
        }
        else if(strcmp(tok->stringVal, "__ignore_typecheck__") == 0)
        {
            ifdpm("\tapplied IGNORE_TYPE_CHECKING \n");
            setFlag(DT::flag::IGNORE_TYPE_CHECKING);
        }

        tok = compiler->tokenAt(++compiler->tokenPtr);
        ifdpm("\tchecking next token while also moving ptr, tok: \n");
        ifdp tok->print(parserDebugger);
    }
}

void DT::datatype::parserDatatypeInit(compilation* compiler, Token::token* dt1, Token::token* dt2, int stars, int exp_type)
{
    pdepth = stars; // I ADDED
    ifdpm("in dt init \n");
    bool flag = false ; // dt1 0 dt2 1
    bool come_back = false;
    if((exp_type != static_cast<int>(DT::expect::primitive_))
    && (dt2) )
        compiler->genError("invalid 2nd datatype \n");

    if(exp_type == static_cast<int>(DT::expect::primitive_))
    {
        auto tmp1 = dt1;
        auto tmp2 = this; 
        auto dtDt2 = new datatype();

        if(flag)
        if(((strcmp(tmp1->stringVal, "long") == 0) || 
            (strcmp(tmp1->stringVal, "short") == 0) || 
            (strcmp(tmp1->stringVal, "double") == 0) || 
            (strcmp(tmp1->stringVal, "float") == 0) 
        ) && (dt2))
            compiler->genError("can't have %s as sec token\n", tmp1->stringVal); // not for 64, some other

        recForDt2:;
            tmp2->typeStr = tmp1->stringVal;
        if(strcmp(tmp1->stringVal, "void") == 0)
        {
            ifdp if(flag) parserDebugger << "\tdt2 type void \n"; 
                else     parserDebugger << "\tdt1 type void \n";
            tmp2->type = static_cast<int>(DT::type::void_);
            tmp2->size = 0;
        
        }
        else if(strcmp(tmp1->stringVal, "char") == 0)
        {
            ifdp if(flag) parserDebugger << "\tdt2 type char \n"; 
                else     parserDebugger << "\tdt1 type char \n";
            tmp2->type = static_cast<int>(DT::type::char_);
            tmp2->size = 1;
        }
        else if(strcmp(tmp1->stringVal, "short") == 0)
        {
            ifdp if(flag) parserDebugger << "\tdt2 type short \n"; 
                else     parserDebugger << "\tdt1 type short \n";
            tmp2->type = static_cast<int>(DT::type::short_);
            tmp2->size = 2;
        }
        else if(strcmp(tmp1->stringVal, "int") == 0)
        {
            ifdp if(flag) parserDebugger << "\tdt2 type int \n"; 
                else     parserDebugger << "\tdt1 type int \n";
            tmp2->type = static_cast<int>(DT::type::int_);
            tmp2->size = 4;
        }
        else if(strcmp(tmp1->stringVal, "long") == 0)
        {
            ifdp if(flag) parserDebugger << "\tdt2 type long \n"; 
                else     parserDebugger << "\tdt1 type long \n";
            tmp2->type = static_cast<int>(DT::type::long_);
            tmp2->size = 4;
        }
        else if(strcmp(tmp1->stringVal, "float") == 0)
        {
            ifdp if(flag) parserDebugger << "\tdt2 type flt \n"; 
                else     parserDebugger << "\tdt1 type flt \n";
            tmp2->type = static_cast<int>(DT::type::void_);
            tmp2->size = 4;
        }
        else if(strcmp(tmp1->stringVal, "double") == 0)
        {
            ifdp if(flag) parserDebugger << "\tdt2 type dble \n"; 
                else     parserDebugger << "\tdt1 type dble \n";
            tmp2->type = static_cast<int>(DT::type::void_);
            tmp2->size = 4;
        }
        else 
            compiler->genError("FATAL ERROR: not a valid primate \n");

        if(come_back)
            goto come_back_here;
        if(dt2 && !flag)
        {
            tmp1 = dt2;
            tmp2 = dtDt2;

            flag = true;
            come_back = true;
            goto recForDt2;
            come_back_here:;
            ifdpm("\tdt2 attaching to dt1 \n");
            this->size += dtDt2->size;
            this->sec = dtDt2; 
            setFlag(DT::flag::IS_SECONDARY);
        }
    }
    else if ((exp_type == static_cast<int>(DT::expect::struct_)) || (exp_type == static_cast<int>(DT::expect::union_)))
        compiler->genError("structs/unions unsupported for now \n");
    else 
        compiler->genError("FATAL ERROR: unnsuported datatyp exp \n");
    
    //this->typeStr = dt1->stringVal; moving to switch blocks
    if(
        (strcmp(dt1->stringVal,"long")==0)
        &&
        (strcmp(dt2->stringVal,"long")==0)
    )
        compiler->genError("64bits unsupported for now \n");
}   

// int  
// struct x 
// long long
static uint32_t name_giver ;
void DT::datatype::parseDatatypeType(compilation* compiler)
{
    auto dt_tok1 = compiler->tokenAt();
    compiler->tokenPtr++;
    auto dt_tok2 = compiler->tokenAt();
    ifdpm("\tat parse dt type (010) with tokens: \n");
    ifdp if(dt_tok1) dt_tok1->print(parserDebugger);
    ifdp if(dt_tok2) dt_tok2->print(parserDebugger); else parserDebugger<<"No sec token \n";

    

   // if(! dt_tok2) return ;
    if(dt_tok2 && dt_tok2->type == static_cast<int>(Token::type::KW) &&
        (strcmp(dt_tok2->stringVal, "void") == 0 ||
        strcmp(dt_tok2->stringVal, "char") == 0 ||
        strcmp(dt_tok2->stringVal, "short") == 0 ||
        strcmp(dt_tok2->stringVal, "int") == 0 ||
        strcmp(dt_tok2->stringVal, "long") == 0 ||
        strcmp(dt_tok2->stringVal, "float") == 0 ||
        strcmp(dt_tok2->stringVal, "double") == 0)
    )
        compiler->tokenPtr++;
    else dt_tok2 = nullptr;

    ifdp if(dt_tok2) parserDebugger << "\tWe got a valid 2nd dt type \n"; 
        else parserDebugger << "\tWe DIDNOT got a valid 2nd dt type \n";

    int expected_type = static_cast<int>(DT::expect::primitive_) ;
    bool flag = false; // set true in the IF condition itself @details not idiomatic
    if( std::string(dt_tok1->stringVal) == "union" && (flag = true))
        expected_type = static_cast<int>(DT::expect::union_);
    else if( std::string(dt_tok1->stringVal) == "struct" && (flag = true) )
        expected_type = static_cast<int>(DT::expect::struct_);
    if(flag)
    {
        ifdpm("\tIts a struct/union, checking name byy checking tokne: \n");
        auto name = compiler->tokenAt();        // we now have the data of struct or union in expected type so we can store the string in token (id of sttruct/union)
        ifdp name->print(parserDebugger);
        if(name->type == static_cast<int>(Token::type::ID)) compiler->tokenPtr++;
        else 
        {
            // struct {} abc; // nameless struct with direct instance. we generate name for our conveniance
            name = new Token::token();
            name->type == static_cast<int>(Token::type::ID);
            std::string nameStr = "_nameless_" + std::to_string(name_giver++);
            name->stringVal = strdup(nameStr.c_str());
            setFlag(DT::flag::STRUCT_UNION_NO_NAME);
        }
    }
    ifdpm("\tchecking star count, check token first: \n");
    // int** // struct abc*** // long long ** 
    int starCnt = 0 ;
    auto tok = compiler->tokenAt();
    ifdp tok->print(parserDebugger);
    while (
        (tok) && (tok->type == static_cast<int>(Token::type::OP)) && 
    ( strcmp(tok->stringVal,"*")==0 ))
    {   
        tok = compiler->tokenAt(++compiler->tokenPtr); 
        ifdpm("\tSTAR++ ");
        starCnt++;
    }

    ifdp parserDebugger << "\n\tstar cnt: "<< starCnt<< std::endl;
    parserDatatypeInit(compiler,dt_tok1, dt_tok2,starCnt,expected_type);
}
void DT::datatype::parse(compilation* compiler)
{
    memset(this,0,sizeof(datatype));
    setFlag(DT::flag::IS_SIGNED); // default datatype is generally signed
    ifdpm("\tentered DT::datatype::parse. Starting series mod-type-mod \n");
    // static int const a = 5 ;
    parseDatatypeModifiers(compiler);
    ifdpm("in series mod-type-mod, first mod done \n");

    ifdp 
    {
        parserDebugger<<"_\n";
        hasDatatypeChanged(this);
        parserDebugger<<"_\n";
    }
    parseDatatypeType(compiler);
    ifdpm("in series mod-type-mod, type done \n");

    ifdp 
    {
        parserDebugger<<"_\n";
        hasDatatypeChanged(this);
        parserDebugger<<"_\n";
    }

    parseDatatypeModifiers(compiler);
    ifdpm("in series mod-type-mod, second mod done \n");


    ifdp 
    {
        parserDebugger<<"_\n";
        hasDatatypeChanged(this);
        parserDebugger<<"_\nFinally the dt: \n";
        print(1);
    }

}

/*void record::parseExressionable_root()
{
    ParsePotentialExpressions();
    struct node* result_node = nodePop();
    nodePush(result_node);
}*/

void DT::datatype::parseVar(compilation* compiler, Token::token* tok, 
record* rec)
{
    ifdp parserDebugger << "entered parseVar with tok: \n" ;
    tok->print(parserDebugger); 
    node* val = nullptr; 
    // TODO check for brackets
    if(tok->type== static_cast<int>(Token::type::OP) && !strcmp(tok->stringVal,"=") )
    {
        // parseExpressionable_root
        tok = compiler->tokenAt(++compiler->tokenPtr);
        ifdp parserDebugger << "dealing with token: \n";
        ifdp tok->print(parserDebugger);
            auto valueNode = new Node::node();
            valueNode->pushInto(compiler->vecNodes);
            valueNode->type = Node::id_;
            valueNode->val.stringVal = tok->stringVal; 
            compiler->tokenPtr++;
        auto varNode = new Node::node();

        varNode->type = Node::var_ ;
        varNode->expVarUnion.var.name = valueNode->val.stringVal;
        varNode->expVarUnion.var.type = this ;
        varNode->expVarUnion.var.val = valueNode;
    
        // TODO CALC STACK OFFSET
        varNode->pushInto(compiler->vecNodes);
    }
}

void record::parseDeclaration()
{
    //std::cout<<"pvfsu entry \n";
    ifdpm("\tentered parseDeclaration to parse potential VFSU \n");
    auto dt = new DT::datatype(); 
    dt->setFlag(DT::flag::IS_SIGNED);
    ifdp hasDatatypeChanged(dt);
    dt->parse(compiler);
    ifdp parserDebugger<<"After parse, here is token currently pointed at: \n";
    if(compiler->tokenPtr < compiler->vecTokens[0].size())
    {
        auto tok = compiler->tokenAt(compiler->tokenPtr);
        ifdp tok->print(parserDebugger);
        // dt here ?
        //std::cout<<"pvfsu exit \n";
        dt->parseVar(compiler,tok,this);
    }
}

// the function is responsble for parsing Keyword
void record::parseKw(Token::token* tok)
{
    ifdpm("\tentered parseKw with token: \n");
    ifdp tok->print(parserDebugger);
    if( DT::isTypeModifier(tok->stringVal) ||
        DT::isDatatype(tok->stringVal) )
    {
        ifdpm("\tthe token is a type modifier or datatype, we can expect form of (typeMod)*(datatype)+(typemod)* {* means 0+, + means 1+} \n");
        parseDeclaration();
        return ;
    }
    std::cout<<"parsing kw exit1 \n";
}


int getPriorityFor(const char* op )
{
    for(int i = 0 ; i < opPrecedence.size(); i++)
        for(auto j: opPrecedence[i].operators )
            if(op == j)
                return i;
    return -1;
}
bool shouldWeEvalLeft(const char* left, const char* right)
{
    if(left==right) return false; 
    int left_rank = getPriorityFor(left);
    int right_rank = getPriorityFor(right);
    
    if( opPrecedence[left_rank].direction == rightToLeft )
        return false;

    return left_rank <= right_rank;
}

void Node::node::nodeShiftChildrenLeft()
{
    //ip num1 * (num2 + num3)
    //op (num1 * num2) + num3
    if(type != Node::exp_ || (expVarUnion.exp.right->type != Node::exp_))
    {
        ifdpm("can't shift left: invalid op \n");
        exit(-1);
    }
    const char* rightOp = expVarUnion.exp.right->expVarUnion.exp.op;
    Node::node* newLeftchild = expVarUnion.exp.left; 
    Node::node* new_rightchild = expVarUnion.exp.right->expVarUnion.exp.left; 
    
    Node::node* newLeftnode = new Node::node();
    newLeftnode->type= Node::exp_ ; 
    newLeftnode->expVarUnion.exp.left = newLeftchild;
    newLeftnode->expVarUnion.exp.right = new_rightchild;
    newLeftnode->expVarUnion.exp.op = expVarUnion.exp.op;

    Node::node* new_rightnode = expVarUnion.exp.right->expVarUnion.exp.right;
    
    expVarUnion.exp.left = newLeftnode;
    expVarUnion.exp.right = new_rightnode;
    expVarUnion.exp.op = rightOp;
}

void Node::node::reorderExpression(int lev)
{
    gapd(lev);
    ifdpm("called reorder exp \n");
    // check if node is exp, and check if there's an exp in atleast one child
    if (type != Node::exp_)
    {
        gapd(lev);
        ifdpm("it is not exp, returning \n");
        return;
    }

    auto left = expVarUnion.exp.left;
    auto right = expVarUnion.exp.right;

    bool leftIsExp = left && (left->type == Node::exp_);
    bool rightIsExp = right && (right->type == Node::exp_);

    if (!leftIsExp && !rightIsExp)
    {
        gapd(lev);
        ifdpm("no child is exp, so returning \n");
        return;
    }

    if (!leftIsExp && rightIsExp) 
    {
        // left op, right op  a*(b+c)
        if(shouldWeEvalLeft(expVarUnion.exp.op, expVarUnion.exp.right->expVarUnion.exp.op))
        {
            gapd(lev);
            ifdpm("Node Shift required \n");  
            nodeShiftChildrenLeft();
            gapd(lev);
            ifdpm("Node after shift: \n");
            printNode(gaper+lev,1);
            if (expVarUnion.exp.left)
            {
                gapd(lev);
                ifdpm("left child also calling reorder exp \n");
                expVarUnion.exp.left->reorderExpression(lev+1);
            } 
            if (expVarUnion.exp.right)
            {
                gapd(lev);
                ifdpm("right child also calling reorder exp \n");
                expVarUnion.exp.right->reorderExpression(lev+1);
            } 
        }
    }
}

record* record::clone(int flags)
{
    auto r = new record(this->compiler, flags);
    return r;
}
void record::parseExpressionableForOp(const char* op)
{
    ifdpm("in parseexpressionableforop() \n");
    ParsePotentialExpressions();
}
void record::checkAndMakeExp(Token::token* tok)
{
    ifdpm("checking and making Expression \n");
    const char* op = tok->stringVal;
    Node::node* left ;
    if(compiler->vecNodes[0].size())
        left = compiler->vecNodes[0].back();
    switch (left->type)
    {
        // the IS_EXPRESSION_ABLE means the node can be part of an expression - a number, another expression, a variable
        IS_EXPRESSION_ABLE:
            ifdpm("left node is an expression element!\n");
            left->printNode(gaper,true);
        break;
        default:
            left = nullptr;
            ifdpm("left node is not expression element! \n");
            return;
        break;
    }
    ifdpm("token pointer moved forward \n");
    compiler->tokenPtr++;
    Node::popFrom(compiler->vecNodes);
    left->flags |= NODE_FLAG_INSIDE_EXPRESSION; 
    
    auto rec1 = this->clone(this->flags);
    rec1->parseExpressionableForOp(op);
    Node::node* right = Node::popFrom(compiler->vecNodes);
    if(right)
        right->flags |= NODE_FLAG_INSIDE_EXPRESSION ;
    else {
        std::cout<<"FATAL ERROR \n";
        exit(1);
    }
    ifdpm("right node: \n");
    right->printNode(gaper,true);
    auto exp = new Node::node();
    exp->type = Node::exp_ ;
    //exp->expVarUnion.exp
    exp->expVarUnion.exp.left = left;
    exp->expVarUnion.exp.right = right ;
    exp->expVarUnion.exp.op = op;
    exp->reorderExpression();
    exp->pushInto(compiler->vecNodes);
}

// The node around this operator will be an expression
// based on operation different functions
int record::dealWithOp(Token::token* tok)
{
    ifdpm("Dealing with Op \n");
    // for binary operations which have left and right operands, we check and make an expression node
    checkAndMakeExp(tok);
    return 0;
}
static int sucide;
// a number node or experession node or keyword node
int record::makeOneNode()
{
    ifdpm("entered makeOneNode with token\n");
    // peek next 
    Token::token* tok = compiler->tokenAt();
    if(!tok)
        // EOF
        return 0;
    ifdp tok->print(parserDebugger);

        // the flag can be used in debugging
    flags |= NODE_FLAG_INSIDE_EXPRESSION;
    int res=0;
    Node::node* n ; 
    switch(tok->type)
    {
        // token is a number, make a node out of it 
        case static_cast<int>(Token::type::Num):
            n = new Node::node();
            n->pushInto(compiler->vecNodes);
            ifdpm("number node made and pusehd to vecNodes \n");
            n->type = Node::number_;
            n->val.ullVal = tok->ullVal; 
            ifdpm("token pointer moved forward 1 position \n");
            compiler->tokenPtr++;
            res=1;
        break;

        // identifier- straightforwadly a node
        case static_cast<int>(Token::type::ID):
            // parse identifier
            n = new Node::node();
            n->pushInto(compiler->vecNodes);
            ifdpm("Identifier node made and pusehd to vecNodes \n");
            n->type = Node::id_;
            n->val.stringVal = tok->stringVal;
            ifdpm("token pointer moved 1 position \n");
            compiler->tokenPtr++;
            res=1;
        break;

        case static_cast<int>(Token::type::OP):
            dealWithOp(tok);
            res=1;
        break;

        case static_cast<int>(Token::type::KW):
            parseKw(tok); 
            res=1;
        break;

        default:
            std::cout<<"not handled this yet! \n";
            exit(-1);
        break;
    }
    
    ifdpm("returning from MakeOneNode: ");
    ifdp parserDebugger<< res <<std::endl;
    return res;
    // at this point we are pointing to a valid token 
    // ptr is not yet incremented which is to be noted

}

void record::ParsePotentialExpressions()
{
    ifdpm("In Parsing potential Expressions.. \n");

    // tokens are read, as long as they get along with the expression, reads.
    // for example a+(b+c(...))
    gaper++;
    while(record::makeOneNode());
    // it fails when nodes are pushed, and maybe there was a token not part of the expression
    gaper--;
}

// this function specifically for global scope keywords
void record::parseGlobalKeyword()
{
    ifdpm("\tentered parseGlobalKeyword\n");
    parseKw(compiler->tokenAt(compiler->tokenPtr));
    // at this point we have a node ?
}

// will read 1 or more of tokens to make a Node 
int parser::parseNextNode()
{
    gaper++;
    // if there is no more token to proces, We are done
    if(compiler->tokenPtr >= compiler->vecTokens[0].size())
    {
        ifdpm("No more tokens, parsing done ! \n");
        gaper--;
        return 0;
    }
    ifdp parserDebugger << "In parse_next() with tok \n";
    auto tok = compiler->tokenAt();
    ifdp tok->print(parserDebugger);
    if(!tok)
    {
        gaper-- ;
        return 0;
    }
    int res = 0;
    auto rec = new record(0);
    rec->compiler = compiler;

    switch(tok->type)
    {
        // for Numbers, identifiers, and strings in general, they are part of expressions.
        case static_cast<int>(Token::type::Num): 
        case static_cast<int>(Token::type::ID):
        case static_cast<int>(Token::type::Str):

            rec->ParsePotentialExpressions();
            break; 

        case static_cast<int>(Token::type::KW):
            rec->parseGlobalKeyword();
            break;

        default:   
            compiler->genError("this token cant be conv to node (or for now) \n");
        break;        
    }
    gaper--;
    return 1;
}

// The main function which handles all parsing functionalities
int parser::parse()
{
    gaper = 0;
    ifdp parserDebugger<< "started parsing \n";
    Node::node* node= nullptr;
    Token::token* last = nullptr;
    compiler->tokenPtr=0; // it will point to the next token that is unprocessed
    name_giver = 0;
    sucide = 0;
    while(parseNextNode())
    {
        std::cout<< "Next token : \n";

        // commenting to test
        if(!compiler->vecNodes[0].size())
        {
            break;
        }

        node = compiler->vecNodes[0].back();
        compiler->vecTree->push_back(node);
    }
    return 1;
}
void Node::node::pushInto(std::vector<Node::node*>* v)
{
    v->push_back( this );
}
Node::node* Node::topOf(std::vector<Node::node*>* v)
{
    if(v->size())
        return v->back();
    return nullptr; 
}
Node::node* Node::popFrom(std::vector<Node::node*>* v)
{
    if(v->size())
    {
        auto x = v->back();
        v->pop_back();
        return x; 
    }
    return nullptr;
}
void Node::node::printNode(int level, bool isDebug)
{
    if(isDebug)
    {
        if(type == exp_)
        {
            gapd(level);
            parserDebugger<<"It is an expression Node with op: "<< expVarUnion.exp.op << std::endl ;
            gapd(level);
            parserDebugger<< "Left node: \n";
            expVarUnion.exp.left->printNode(level+1,1);
            gapd(level);
            parserDebugger<<"right node: \n";
            expVarUnion.exp.right->printNode(level+1,1);
        }
        else if(type == number_)
        {
            gapd(level);
            parserDebugger<<"number val: "<< val.ullVal << std::endl;
        }
        return ;
    }
    if(type == exp_)
    {
        gap(level);
        std::cout<<"It is an expression Node with op: "<< expVarUnion.exp.op << std::endl ;
        gap(level);
        std::cout<< "Left node: \n";
        expVarUnion.exp.left->printNode(level+1);
        gap(level);
        std::cout<<"right node: \n";
        expVarUnion.exp.right->printNode(level+1);
    }
    else if(type == number_)
    {
        gap(level);
        std::cout<<"number val: "<< val.ullVal << std::endl;
    }
}