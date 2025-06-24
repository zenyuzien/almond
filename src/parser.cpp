// #include "parser.h"
#include "compiler.h"
#include <iomanip>
#include <string>
#include <iostream>
#include <memory.h>

int gaper;

extern bool PARSER_DEBUG;
extern std::ofstream parserDebugger;

#define gap(level)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    for (int i = 0; i < level; i++)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
        std::cout << "  ";
#define gapd(level)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    for (int i = 0; i < level; i++)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
        parserDebugger << "  ";
#define ifdp                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
    if (PARSER_DEBUG)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              \
        for (int i = 0, lim = (gaper > 15 ? 15 : gaper); i < lim; i++)                                                                                                                                                                                                                                                                                                                                                                                                                                             \
            parserDebugger << "  ";                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    if (PARSER_DEBUG)
#define ifdpm(msg)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    if (PARSER_DEBUG)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              \
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
            for (int i = 0, lim = (gaper > 15 ? 15 : gaper); i < lim; i++)                                                                                                                                                                                                                                                                                                                                                                                                                                         \
                parserDebugger << "  ";                                                                                                                                                                                                                                                                                                                                                                                                                                                                            \
            parserDebugger << msg;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
        }

#define IS_EXPRESSION_ABLE                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         \
    case Node::exp_:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    case Node::exp_bracket_:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
    case Node::unary_:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             \
    case Node::id_:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    case Node::number_:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            \
    case Node::string_

size_t sizeOfStruct( const char* structName, compilation* compiler)
{
    struct SYM::symbol* sym ; 
    auto str = std::string(structName);

    sym = compiler->symResolver->get( str);
    if(!sym)
        return 0;
    Node::node* data = (Node::node*) sym->metadata ; 

    return data->expVarUnion.structure.bodyNode->expVarUnion.body.size;
    
}

// a small gdb- like feature to track changes to struct data_type, purely for debugging purpose
bool
DT::hasDatatypeChanged (struct datatype *current)
{
    parserDebugger << "[datatype tracker called] \n";
    static struct datatype last = { 0 };
    static bool firstCall = true;

    bool changed = false;

    if (firstCall)
        {
            memcpy (&last, current, sizeof (struct datatype));
            firstCall = false;
            ifdpm ("[datatype tracker] First-time initialization.\n");
            return true;
        }

#define CHECK_AND_REPORT(field, fmt)                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    if (last.field != current->field)                                                                                                                                                                                                                                                                                                                                                                                                                                                                              \
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          \
            parserDebugger << "[datatype changed] " << std::left << std::setw (12) << #field << ": " << last.field << " -> " << current->field << "\n";                                                                                                                                                                                                                                                                                                                                                            \
            changed = true;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        \
        }

    CHECK_AND_REPORT (pdepth, "%d");
    CHECK_AND_REPORT (type, "%d");
    CHECK_AND_REPORT (flags, "%" PRIu16);
    CHECK_AND_REPORT (size, "%" PRIu64);
    CHECK_AND_REPORT (sec, "%p");
    CHECK_AND_REPORT (structNode, "%p"); // applies to both union_node and struct_node
    if (last.typeStr != current->typeStr)
        {
            parserDebugger << "[datatype changed] " << std::left << std::setw (12) << "typeStr" << ": '" << (last.typeStr ? last.typeStr : "(null)") << "' -> '" << (current->typeStr ? current->typeStr : "(null)") << "'\n";
            changed = true;
        }

    if (changed)
        ifdpm ("[datatype tracker] Updated internal copy.\n") else ifdpm ("[datatype tracker] UNCHANGED.\n");

    memcpy (&last, current, sizeof (struct datatype));
    return changed;
}


bool
DT::isDatatype (const char *str)
{
    return strcmp (str, "void") == 0 || strcmp (str, "char") == 0 || strcmp (str, "int") == 0 || strcmp (str, "short") == 0 || strcmp (str, "float") == 0 || strcmp (str, "double") == 0 || strcmp (str, "long") == 0 || strcmp (str, "struct") == 0 || strcmp (str, "union") == 0;
}
bool
DT::isTypeModifier (const char *str)
{
    return strcmp (str, "unsigned") == 0 || strcmp (str, "signed") == 0 || strcmp (str, "static") == 0 || strcmp (str, "const") == 0 || strcmp (str, "extern") == 0 || strcmp (str, "__ignore_typecheck__") == 0;
}

void
DT::datatype::setFlag (DT::flag f)
{
    flags |= static_cast<uint16_t> (f);
}
void
DT::datatype::unsetFlag (DT::flag f)
{
    flags &= ~static_cast<uint16_t> (f);
}
bool
DT::datatype::checkFlag (DT::flag f)
{
    return (flags & static_cast<uint16_t> (f)) != 0;
}

void
DT::datatype::print (bool isdebug)
{
    if (isdebug)
        {
            ifdp parserDebugger << "pdepth: " << pdepth << " type: " << type << " \n";
            std::bitset<16> binary (flags);
            ifdp parserDebugger << "flags: " << binary << std::endl;
            ifdp parserDebugger << "size: " << size << " \n";
            try 
            {
                if (typeStr != nullptr)
                    ifdp parserDebugger << "typeStr: " << std::string (typeStr) << " \n";
                if (sec != nullptr)
                    {
                        ifdp parserDebugger << "secondary: \n\n";
                        sec->print (isdebug);
                    }
                else
                    ifdp parserDebugger << "No secondary: \n\n";
            } catch (const std::logic_error& e) {
        ifdp parserDebugger << "Caught logic_error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        // Catches other std exceptions
        ifdp parserDebugger << "Caught exception: " << e.what() << std::endl;
    }

            return;
        }

    std::cout << "pdepth: " << pdepth << " type: " << type << " \n";
    std::bitset<16> binary (flags);
    std::cout << "flags: " << binary << std::endl;
    std::cout << "size: " << size << " \n";
    if (typeStr)
        std::cout << " typeStr: " << std::string (typeStr) << " \n";
    if (sec != nullptr)
        {
            std::cout << "secondary: \n\n";
            sec->print ();
        }
    else
        std::cout << "No secondary: \n\n";
}

void
DT::datatype::parseDatatypeModifiers (compilation *compiler)
{
    auto tok = compiler->tokenAt ();

    ifdpm ("entered parseMod with token: ");
    ifdp tok->print (parserDebugger);
    // static const
    ifdpm ("checking flags for the consecutive mods \n");
    while (tok && tok->type == static_cast<int> (Token::type::KW))
        {
            if (!DT::isTypeModifier (tok->stringVal))
                break;
            if (strcmp (tok->stringVal, "signed") == 0)
                {
                    ifdpm ("applied sign \n");
                    setFlag (DT::flag::IS_SIGNED);
                }
            else if (strcmp (tok->stringVal, "unsigned") == 0)
                {
                    ifdpm ("applied unsign \n");
                    unsetFlag (DT::flag::IS_SIGNED);
                }
            else if (strcmp (tok->stringVal, "static") == 0)
                {
                    ifdpm ("applied static \n");
                    setFlag (DT::flag::IS_STATIC);
                }
            else if (strcmp (tok->stringVal, "const") == 0)
                {
                    ifdpm ("applied const \n");
                    setFlag (DT::flag::IS_CONST);
                }
            else if (strcmp (tok->stringVal, "extern") == 0)
                {
                    ifdpm ("applied extern \n");
                    setFlag (DT::flag::IS_EXTERN);
                }
            else if (strcmp (tok->stringVal, "__ignore_typecheck__") == 0)
                {
                    ifdpm ("applied IGNORE_TYPE_CHECKING \n");
                    setFlag (DT::flag::IGNORE_TYPE_CHECKING);
                }

            tok = compiler->tokenAt (++compiler->tokenPtr);
            ifdpm ("checking next token while also moving ptr, tok: \n");
            ifdp tok->print (parserDebugger);
        }
}
size_t DT::datatype::dtSizeArrayAccess()
{
    // for structs and unions, the size is of structs
    if( 
        (type == static_cast<int>(DT::type::struct_) ||
        type == static_cast<int>(DT::type::struct_))
        && flags & static_cast<int> (DT::flag::IS_POINTER)
        && (pdepth == 1 )
     )
        return size;
    return dtSize();
}
size_t DT::datatype::dtElementSize()
{
    if( flags & static_cast<int>(DT::flag::IS_POINTER) )
        return static_cast<size_t>( DT::sizes::pointerSize );
    return size;
}
size_t DT::datatype::dtPointingSize() //datatype_size_no_ptr
{
    if( flags & static_cast<int>(DT::flag::IS_ARRAY) )
        return array.size; 
    return size;
}
size_t DT::datatype::dtSize()
{
    if( 
        (flags & static_cast<int>(DT::flag::IS_POINTER)) &&
        pdepth
    )
        return  static_cast<size_t>( DT::sizes::pointerSize ); // 
    if(flags & static_cast<int>(DT::flag::IS_POINTER) )
        return array.size;
    return size;
}

void
DT::datatype::parserDatatypeInit (compilation *compiler, Token::token *dt1, Token::token *dt2, int stars, int exp_type)
{
    pdepth = stars; // I ADDED
    ifdpm ("in dt init ");
    bool flag = false; // dt1 0 dt2 1
    bool come_back = false;
    if ((exp_type != static_cast<int> (DT::expect::primitive_)) && (dt2))
        compiler->genError ("invalid 2nd datatype \n");

    if (exp_type == static_cast<int> (DT::expect::primitive_))
        {
            auto tmp1 = dt1;
            auto tmp2 = this;
            auto dtDt2 = new datatype ();

            if (flag)
                if (((strcmp (tmp1->stringVal, "long") == 0) || (strcmp (tmp1->stringVal, "short") == 0) || (strcmp (tmp1->stringVal, "double") == 0) || (strcmp (tmp1->stringVal, "float") == 0)) && (dt2))
                    compiler->genError ("can't have %s as sec token\n", tmp1->stringVal); // not for 64, some other

        recForDt2:;
            tmp2->typeStr = tmp1->stringVal;
            if (strcmp (tmp1->stringVal, "void") == 0)
                {
                    ifdp if (flag) parserDebugger << "dt2 type void \n";
                    else parserDebugger << "dt1 type void \n";
                    tmp2->type = static_cast<int> (DT::type::void_);
                    tmp2->size = 0;
                }
            else if (strcmp (tmp1->stringVal, "char") == 0)
                {
                    ifdp if (flag) parserDebugger << "dt2 type char \n";
                    else parserDebugger << "dt1 type char \n";
                    tmp2->type = static_cast<int> (DT::type::char_);
                    tmp2->size = 1;
                }
            else if (strcmp (tmp1->stringVal, "short") == 0)
                {
                    ifdp if (flag) parserDebugger << "dt2 type short \n";
                    else parserDebugger << "dt1 type short \n";
                    tmp2->type = static_cast<int> (DT::type::short_);
                    tmp2->size = 2;
                }
            else if (strcmp (tmp1->stringVal, "int") == 0)
                {
                    ifdp if (flag) parserDebugger << "dt2 type int \n";
                    else parserDebugger << "dt1 type int \n";
                    tmp2->type = static_cast<int> (DT::type::int_);
                    tmp2->size = 4;
                }
            else if (strcmp (tmp1->stringVal, "long") == 0)
                {
                    ifdp if (flag) parserDebugger << "dt2 type long \n";
                    else parserDebugger << "dt1 type long \n";
                    tmp2->type = static_cast<int> (DT::type::long_);
                    tmp2->size = 4;
                }
            else if (strcmp (tmp1->stringVal, "float") == 0)
                {
                    ifdp if (flag) parserDebugger << "dt2 type flt \n";
                    else parserDebugger << "dt1 type flt \n";
                    tmp2->type = static_cast<int> (DT::type::void_);
                    tmp2->size = 4;
                }
            else if (strcmp (tmp1->stringVal, "double") == 0)
                {
                    ifdp if (flag) parserDebugger << "dt2 type dble \n";
                    else parserDebugger << "dt1 type dble \n";
                    tmp2->type = static_cast<int> (DT::type::void_);
                    tmp2->size = 4;
                }
            else
                compiler->genError ("FATAL ERROR: not a valid primate \n");

            if (come_back)
                goto come_back_here;
            if (dt2 && !flag)
                {
                    tmp1 = dt2;
                    tmp2 = dtDt2;

                    flag = true;
                    come_back = true;
                    goto recForDt2;
                come_back_here:;
                    ifdpm ("dt2 attaching to dt1 \n");
                    this->size += dtDt2->size;
                    this->sec = dtDt2;
                    setFlag (DT::flag::IS_SECONDARY);
                }
        }
    else if ((exp_type == static_cast<int> (DT::expect::struct_)) || (exp_type == static_cast<int> (DT::expect::union_)))
    {
        this->typeStr = dt1->stringVal; 
        this->type = static_cast<int>(DT::type::struct_) ; 
        this->size = sizeOfStruct(dt1->stringVal, compiler);
        std::string name = (dt1->stringVal) ? std::string( dt1->stringVal): "";

        this->structNode = compiler->symResolver->nodeForName( name );
    }
    else
        compiler->genError ("FATAL ERROR: unnsuported datatyp exp \n");

    // this->typeStr = dt1->stringVal; moving to switch blocks
    if ((strcmp (dt1->stringVal, "long") == 0) && dt2 && (strcmp (dt2->stringVal, "long") == 0))
        compiler->genError ("64bits unsupported for now \n");
}

// int
// struct x
// long long
static uint32_t name_giver;
void
DT::datatype::parseDatatypeType (compilation *compiler)
{
    auto dt_tok1 = compiler->tokenAt ();
    compiler->tokenPtr++;
    //Token::token* name;
    auto dt_tok2 = compiler->tokenAt ();
    ifdpm ("at parse dt type with tokens: ");
    ifdp if (dt_tok1) dt_tok1->print (parserDebugger);
    ifdp if (dt_tok2) dt_tok2->print (parserDebugger);
    else parserDebugger << "No sec token \n";

    // if(! dt_tok2) return ;
    if (dt_tok2 && dt_tok2->type == static_cast<int> (Token::type::KW) && (strcmp (dt_tok2->stringVal, "void") == 0 || strcmp (dt_tok2->stringVal, "char") == 0 || strcmp (dt_tok2->stringVal, "short") == 0 || strcmp (dt_tok2->stringVal, "int") == 0 || strcmp (dt_tok2->stringVal, "long") == 0 || strcmp (dt_tok2->stringVal, "float") == 0 || strcmp (dt_tok2->stringVal, "double") == 0))
        compiler->tokenPtr++;
    else
        dt_tok2 = nullptr;

    ifdp if (dt_tok2) parserDebugger << "We got a valid 2nd dt type \n";
    else parserDebugger << "We DIDNOT got a valid 2nd dt type \n";
    if (dt_tok1->type == static_cast<int> (Token::type::KW))
        if (strcmp(dt_tok1->stringVal, "float") == 0 ||
            strcmp(dt_tok1->stringVal, "long") == 0  ||
            strcmp(dt_tok1->stringVal, "double") == 0)
            if(dt_tok2)
            if (dt_tok2->type == static_cast<int> (Token::type::KW))
                if ( !strcmp(dt_tok2->stringVal,"int"))
                    {
                        parserDebugger << "Ignoring 2nd int as its valueless \n";
                        dt_tok2 = nullptr;
                    }

    int expected_type = static_cast<int> (DT::expect::primitive_);
    bool flag = false; // set true in the IF condition itself @details not idiomatic
    if (std::string (dt_tok1->stringVal) == "union" && (flag = true))
        expected_type = static_cast<int> (DT::expect::union_);
    else if (std::string (dt_tok1->stringVal) == "struct" && (flag = true))
        expected_type = static_cast<int> (DT::expect::struct_);
    if (flag)
        {
            ifdpm ("Its a struct/union, checking name by checking token: \n");
            
            dt_tok1 = compiler->tokenAt (); // we now have the data of struct or union in expected type so we can store the string in token (id of sttruct/union)
            ifdp dt_tok1->print (parserDebugger);
            if (dt_tok1->type == static_cast<int> (Token::type::ID))
            {
                compiler->tokenPtr++;

            }
            else
                {
                    // struct {} abc; // nameless struct with direct instance. we generate name for our conveniance
                    
                    dt_tok1->type == static_cast<int> (Token::type::ID);
                    std::string nameStr = "_nameless_" + std::to_string (name_giver++);
                    dt_tok1->stringVal = strdup (nameStr.c_str ());
                    setFlag (DT::flag::STRUCT_UNION_NO_NAME);
                }
        }
    // int** // struct abc*** // long long **
    int starCnt = 0;
    auto tok = compiler->tokenAt ();
    while ((tok) && (tok->type == static_cast<int> (Token::type::OP)) && (strcmp (tok->stringVal, "*") == 0))
        {
            tok = compiler->tokenAt (++compiler->tokenPtr);
            ifdpm ("STAR++ ");
            starCnt++;
        }

    ifdp parserDebugger << "star cnt: " << starCnt << std::endl;
    
    parserDatatypeInit (compiler, dt_tok1, dt_tok2, starCnt, expected_type);
}
void
DT::datatype::parse (compilation *compiler)
{
    memset (this, 0, sizeof (datatype));
    setFlag (DT::flag::IS_SIGNED); // default datatype is generally signed
    ifdpm ("entered DT::datatype::parse. Starting series mod-type-mod \n");
    // static int const a = 5 ;
    parseDatatypeModifiers (compiler);
    ifdpm ("in series mod-type-mod, first mod done \n");

    /*ifdp
    {
        parserDebugger<<"_\n";
        hasDatatypeChanged(this);
        parserDebugger<<"_\n";
    }*/
    parseDatatypeType (compiler);
    ifdpm ("in series mod-type-mod, type done \n");
        
    /*ifdp
    {
        parserDebugger<<"_\n";
        hasDatatypeChanged(this);
        parserDebugger<<"_\n";
    }*/

    parseDatatypeModifiers (compiler);
    ifdpm ("in series mod-type-mod, second mod done \n");


    ifdp
    {
        // parserDebugger<<"_\n";
        // hasDatatypeChanged(this);
        parserDebugger << "Finally the dt: \n";
        print(1);
    }        
}

/*void record::parseExressionable_root()
{
    ParsePotentialExpressions();
    struct node* result_node = nodePop();
    nodePush(result_node);
}*/

void record::makeVarAndReg(DT::datatype * dt, Token::token* name, Node::node* valueNode)
{
    auto varNode = new Node::node ();
    varNode->type = Node::var_;
    varNode->expVarUnion.variable.name = name->stringVal;
    varNode->expVarUnion.variable.type = dt;
    varNode->expVarUnion.variable.val = valueNode;
    
    // parserScopeOffset
    if(flags & static_cast<int>(recordFlags::globalScope) )
    {
        // scopeOffsetGlobal
        return ;
    }
    if(flags & static_cast<int>(recordFlags::insideStructure) )
    {
        //scopeOffsetForStructure
        int offset = 0 ;
        parserScope* last = (parserScope*)compiler->scopeLastInstance();
        if(last)
        {
            offset += (last->stackOffset + last->node->expVarUnion.variable.type->size );
            if(varNode->type != Node::struct_ && varNode->type != Node::union_)
            {
                varNode->expVarUnion.variable.padding = padding(offset, 
                varNode->expVarUnion.variable.type->size);
            }
            varNode->expVarUnion.variable.allignedOffset = offset + 
            varNode->expVarUnion.variable.padding;
        }
        return;
    }
    // parserScopeOffsetForStack 

    parserScope* last =
    (parserScope*)compiler->scopeFromTo(compiler->activeScope, compiler->rootScope);

    bool upward = flags & static_cast<int>(recordFlags::upwardStack);
    int offset = -varNode->varSize();
    if(upward)
    {
        // TODO HANDLE UPWARD
        std::cout<<"FATAL BUG \n";
        exit(-1);
    }
    if(last)
    {
        offset += last->node->extractListOrVarNode()
        ->expVarUnion.variable.allignedOffset;
    }

    // parserScopeOffsetForStack end 
    // parserScopeOffset end
    auto tmpParserScope = new parserScope(varNode, varNode->expVarUnion.variable.allignedOffset,0);
    compiler->pushScope(
        tmpParserScope, varNode->expVarUnion.variable.type->size
    );
    varNode->pushInto (compiler->vecNodes);
}

void
DT::datatype::parseVar (compilation *compiler, Token::token *name, record *rec)
{
    // we anyway stored the token in name, so we can proceed to next token
    compiler->tokenPtr++;
    ifdp parserDebugger << "Entered parseVar with name: " << name->stringVal << " and currently pointing tok: \n";
    auto tok = compiler->tokenAt () ;
    if (!tok)
        return;
    tok->print (parserDebugger);
    Node::node *val = nullptr;
    // TODO check for brackets
    Node::node *valueNode = nullptr;
    if (tok->type == static_cast<int> (Token::type::OP) && !strcmp (tok->stringVal, "="))
        {
            ifdpm("= found, so parsing potential expressions now \n");
            compiler->tokenPtr++;
            rec->ParsePotentialExpressions ();
            valueNode = Node::popFrom (compiler->vecNodes,1);
            /*// parseExpressionable_root
            tok = compiler->tokenAt(++compiler->tokenPtr); // value sotred in tok
            ifdp parserDebugger << "dealing with token: ";
            ifdp tok->print(parserDebugger);
                auto valueNode = new Node::node();
                valueNode->type = Node::id_;
                ifdp parserDebugger<<"valueNode/varNode val: "<< tok->ullVal << std::endl;
                valueNode->val.ullVal = tok->ullVal;
                compiler->tokenPtr++; // token after value*/
        }
    record* rec2 = new record(compiler, 0);
    rec2->makeVarAndReg(this, name, valueNode );


}

void record::parseSymbol()
{
        size_t varSize =0 ; 
        auto recNew = new record(compiler,static_cast<int> (recordFlags::globalScope));
        recNew->parseBody(&varSize);
        //Node::node* bodyNode = Node::popFrom(compiler->vecNodes,1);
        //bodyNode->pushInto(compiler->vecNodes, true);
}

void record::parseContentNode( record* rec )
{
    ifdp compiler->printTokensFromCurPointer(parserDebugger, 5);
    auto tok = compiler->tokenAt();
    if(tok->type == static_cast<int>(Token::type::KW) )
    {
        rec->parseKw(tok);
        return ;
    }
    // if not declaration, maybe an expresison 
    rec->ParsePotentialExpressions();

    tok = compiler->tokenAt();
    if(
        tok->type == static_cast<int>(Token::type::Sym)
        &&  !tok->charVal==';'
    )
    {
        // parse symbol 
        parseSymbol();
    }

    compiler->skipCharOrError(';');
}


void record::appendSizeNode(size_t* varSize, Node::node* n)
{
    if(!n)
        return ;
    if(n->type == Node::var_)
    {
        if(
            n->expVarUnion.variable.type->type 
                == static_cast<int>(DT::type::struct_) ||
            n->expVarUnion.variable.type->type 
                == static_cast<int>(DT::type::union_)
        )
        {
            //parser_append_size_for_node_struct_union
            *varSize += n->varSize();
            if( n->expVarUnion.variable.type->flags 
                & static_cast<int>( DT::flag::IS_POINTER )  )
                    return;
            
            Node::node* largestNode =nullptr;
            // variable_struct_or_union_body_node
            if(n->expVarUnion.variable.type->type 
                == static_cast<int>(DT::type::struct_) )
            {
                largestNode = 
                    n->expVarUnion.variable.type
                     ->structNode->expVarUnion.structure.bodyNode;
            }

            // unions not implemented 
            std::cout<<"FATAL ERROR \n";
            exit(-1);
            
            if(largestNode)
                *varSize += align( *varSize, 
                    largestNode->expVarUnion.variable.type->size );
            return ;
        }
        *varSize += n->varSize();
    }
    else if(n->type == Node::varlist_)
    {
        int ptr = 0; 
        Node::node* varNode = n->expVarUnion.VariableList[0][ptr++];
        while(varNode)
        {
            appendSizeNode( varSize, varNode );
            varNode = n->expVarUnion.VariableList[0][ptr++];
        }
    }

}

void record::finalize(
        Node::node* bodyNode,
        std::vector<Node::node*>* content,
        size_t* varSize,  
        Node::node* largestVarNodeInContent, Node::node* largestPrimitiveNode
)
{

    ifdp parserDebugger << "finalizing with largest node: " ;
    ifdp largestVarNodeInContent->printNode(gaper,1) ;
    ifdp parserDebugger << " and largest primitive node: " ;
    ifdp largestPrimitiveNode->printNode(gaper,1);
    ifdp parserDebugger << " and size of body without padding: " << *varSize << std::endl;


            if(flags & static_cast<int>(recordFlags::insideUnion))
            if(largestVarNodeInContent) // always true if single body  node has atleast one 
                *varSize = largestVarNodeInContent->varSize();
        
        int padding = computeSumPadding(content);
        *varSize += padding ;

        if(largestPrimitiveNode)
        {
            *varSize = align(*varSize , 
            largestPrimitiveNode->expVarUnion.variable.type->size );
        }
        bool padded = (padding!=0);
        bodyNode->expVarUnion.body.content = content;
        bodyNode->expVarUnion.body.padded = padded ;
        ifdp parserDebugger<< "After padding: " << *varSize << std::endl;
        bodyNode->expVarUnion.body.size = *varSize; 
        bodyNode->expVarUnion.body.largestVarNodeInContent = largestVarNodeInContent;

}

void record::parseBody(size_t* varSize)
{
    ifdpm("Parsing body now \n");
    compiler->newScope(0);
    size_t tmp = 0 ;
    if(!varSize)
        varSize = &tmp;
    auto content = new std::vector<Node::node*>();
    auto tok = compiler->tokenAt();
    if(!( tok->type == static_cast<int>(Token::type::Sym) 
            && tok->charVal == '{' 
    ))
    {
       
    ifdpm("Single line body ");
        // body of single statement 

        auto node=  new Node::node();
        node->type = Node::body_;

        node->binded.head = compiler->parserActiveBody;
        compiler->parserActiveBody = node; 

        parseContentNode( clone(flags) ); // this is the only statenent of body
        
        auto contentNode = Node::popFrom(compiler->vecNodes,1);

        ifdpm("and pushed into content node \n");
        contentNode->pushInto(content,1);

        // change var size, i believve we should cummulate all statements into varSIze
        // varSize -> bodySize ? 
        appendSizeNode(varSize, contentNode);

        Node::node* larNode = nullptr; 
        if(contentNode->type == Node::var_)
            larNode = contentNode;

        // finalize body

        Node::node* largestPrimitiveNode = larNode;
        Node::node* largestNode = larNode; 

        finalize:; // from multiparse
        finalize( node ,content, varSize, largestNode, largestPrimitiveNode  );
        
        compiler->parserActiveBody = node->binded.head;
        node->pushInto(compiler->vecNodes, 1);
        compiler->finishScope();
        return ;
    }


    ifdpm("Multi-line body ");

    // multi-statement parse
    Node::node* bodyNode = new Node::node();
    bodyNode->type = Node::body_;
    bodyNode->expVarUnion.body.content = nullptr; 
    bodyNode->expVarUnion.body.size = 0;
    bodyNode->expVarUnion.body.padded = false; 
    bodyNode->expVarUnion.body.largestVarNodeInContent = nullptr;

    Node::node* statement = nullptr; 
    Node::node* largestPossibleVarNode = nullptr;
    Node::node* LargestPrimitiveVarNode = nullptr;

    compiler->skipCharOrError('{');
    Token::token* tmpToken = compiler->tokenAt();
    
    while( ! ((tmpToken->type == static_cast<int>(Token::type::Sym) )
    &&
    ( tmpToken->charVal == '}' )) )
    {
        ifdpm("Begenning to parse a statement now: ");
        parseContentNode( clone(flags) );
        auto contentNode = Node::popFrom(compiler->vecNodes,1);

        if(contentNode->type == Node::var_)
        {
            if(! largestPossibleVarNode || 
            ( largestPossibleVarNode->expVarUnion.variable.type->size <= 
            contentNode->expVarUnion.variable.type->size ) )
            {
                ifdpm("this is the largest possible varNode so far \n");
                largestPossibleVarNode = contentNode;
            }
            if( 
                contentNode->expVarUnion.variable.type->type != static_cast<int>(DT::type::union_) &&
                 contentNode->expVarUnion.variable.type->type != static_cast<int>(DT::type::struct_) 
              )
              {
                    if(!LargestPrimitiveVarNode || (
                        LargestPrimitiveVarNode->expVarUnion.variable.type->size
                        <= contentNode->expVarUnion.variable.type->size
                    ))
                    {
                        ifdpm("this is the largest primitive varNode so far \n");
                        LargestPrimitiveVarNode = contentNode;
                    }
              }
        }

        ifdpm("and pushed into content node \n");

        content->push_back(contentNode);
        appendSizeNode(varSize, contentNode->extractListOrVarNode());
        tmpToken = compiler->tokenAt();
    }
    ifdpm("Exited from the block doing multi line parsing, the content: \n\n");
    for(auto n : content[0])
    {
        n->printNode(1,true);
    }
    compiler->skipCharOrError('}');
    finalize( bodyNode, content, varSize, largestPossibleVarNode, LargestPrimitiveVarNode );

    compiler->parserActiveBody = bodyNode->binded.head;

    ifdpm("_PUSHING the multiline chunk as a body type node \n");
    bodyNode->pushInto(compiler->vecNodes);

    compiler->finishScope();
    // TODO function stack size adjustment
    return ;

}

void
record::parseDeclaration ()
{
    // std::cout<<"pvfsu entry \n";
    ifdpm ("entered parseDeclaration to parse potential VFSU \n");
    auto dt = new DT::datatype ();
    dt->parse (compiler);

    // at this point datatype is parsed, now identifier parsing needed , happens at parseVar
    ifdp parserDebugger << "DT parse done, parsing struct/var now ";

    if(( dt->type == static_cast<int>(DT::type::struct_) ||
        dt->type == static_cast<int>(DT::type::union_ )   )
        //&&( compiler->tokenAt()->type == static_cast<int> (Token::type::Sym)
      //  && compiler->tokenAt()->charVal == '{'
      //  ) TODO PLEASE
    )
    {


    //parseStructOrUnion
        if( dt->type == static_cast<int>(DT::type::struct_))
        {
            //parseStruct 
            bool forward = true; 
            auto x = compiler->tokenAt();
            if(x->type == static_cast<int>(Token::type::Sym) 
            && x->charVal == '{')
                forward = false; 
            if(!forward)
                compiler->newScope(0);
            
            //parse_struct_no_new_scope begin
            Node::node* bodyNode = nullptr ;
            size_t bodyVarSize = 0 ;
            record* tmprec = new record(compiler,
            static_cast<int>(recordFlags::insideStructure) ); 
            if(!forward)
            {
                tmprec->parseBody(&bodyVarSize);
                ifdpm("retrieving to wrap with a struct Node \n");
                bodyNode = Node::popFrom(compiler->vecNodes,1);
            }
            // make struct node
                Node::node* structNode = new Node::node();
                int flags = 0;
                if(!bodyNode) flags |= static_cast<int> (Node::flags::forwardDeclaration);
                structNode->type = Node::struct_; 
                structNode->expVarUnion.structure.bodyNode = bodyNode;
                structNode->expVarUnion.structure.name = dt->typeStr ; 
                structNode->flags = flags;
            if(bodyNode)
            {
                dt->size = bodyNode->expVarUnion.body.size;
            }
            dt->structNode = structNode;
            auto tmpToken = compiler->tokenAt();
            if(tmpToken->type == static_cast<int>(Token::type::ID) )
            {
                ifdpm("Also declared a variable within the struct definition \n");
                structNode->flags |= static_cast<int>( Node::flags::varCombined );

                if(dt->flags & static_cast<int>(DT::flag::STRUCT_UNION_NO_NAME))
                {
                    dt->typeStr = tmpToken->stringVal;
                    dt->unsetFlag(DT::flag::STRUCT_UNION_NO_NAME);
                    structNode->expVarUnion.structure.name = tmpToken->stringVal;
                    ifdpm("the nameless struct has an instance which makes it overwritten ");
                    ifdp parserDebugger << "with "<< tmpToken->stringVal << std::endl;
                    // overwrite strucutre's given name to this instance name direcly
                }
                //make_variable_node_and_register(history_begin(0), dtype, var_name, NULL); 
                makeVarAndReg(dt, tmpToken, nullptr);
                structNode->expVarUnion.structure.var = Node::popFrom(compiler->vecNodes);

                compiler->tokenPtr++;
            }
            compiler->skipCharOrError(';');
            structNode->pushInto(compiler->vecNodes,1);
            ifdpm("wrap and push complete \n");
            //parse_struct_no_new_scope end

            // actual body parsing now 
            if(!forward)
                compiler->finishScope();
        }
        else if(dt->type == static_cast<int>(DT::type::union_ ))
        {
            ;
        }
        else 
            compiler->genError("FATAL ERROR: should be union/struct \n");
        
        ifdpm("for SYMRESOLVER \n");
        Node::node* suNode = Node::popFrom(compiler->vecNodes,1);
        compiler->symresolverBuildForNode(suNode);
        suNode->pushInto(compiler->vecNodes,1);
        ifdpm("for SYMRESOLVER done \n");
    }

    // this tok will hold the identifier
    auto tok = compiler->tokenAt ();
    if (!tok)
        {
            ifdpm ("EOF \n");
            return;
        }

    dt->parseVar (compiler, tok, this);
    ifdpm("After parseVar variable node in vecNodes now \n");

    tok = compiler->tokenAt ();
    if (!tok)
        {
            ifdpm ("EOF \n");
            return;
        }

    // int a compltedparseVar
    // can be a =10 or a[10] or a; simply
    if (tok->type == static_cast<int> (Token::type::OP) && !strcmp (tok->stringVal, "["))
        {
            ifdpm("[ got, so parsing static sizes of the array : ");
            auto arrss = parseArraySS ();
            ifdpm("after parseaass tok : ");
            ifdp tok->print(parserDebugger);
            dt->array.multiDimSizes = arrss;
            dt->array.size = dt->array.getSizeFromIndex(0);
            dt->flags |= static_cast<int> (DT::flag::IS_ARRAY);
        }

    if (tok->type == static_cast<int> (Token::type::OP) && !strcmp (tok->stringVal, ","))
        {
            ifdpm ("found comma, doing multivar node \n");
            auto varListVec = new std::vector<Node::node *> ();
            Node::node *singleVarNode;
            do
                {
                    compiler->tokenPtr++;
                    //////dt->parseVar (compiler, tok, this);
                    singleVarNode = Node::popFrom (compiler->vecNodes,1);
                    varListVec->push_back (singleVarNode);
                }
            while (compiler->tokenAt ()->type == static_cast<int> (Token::type::OP) && tok->charVal == ',');
            auto varListNode = new Node::node ();
            varListNode->type = Node::varlist_;
            varListNode->expVarUnion.VariableList = varListVec;
            varListNode->pushInto (compiler->vecNodes,1);
        }

    ifdpm ("Expecting semicolong now \n");
    compiler->skipCharOrError(';');
}

// the function is responsble for parsing Keyword
void
record::parseKw (Token::token *tok)
{
    ifdpm ("entered parseKw with token: ");
    ifdp tok->print (parserDebugger);
    if (DT::isTypeModifier (tok->stringVal) || DT::isDatatype (tok->stringVal))
        {
            ifdpm ("the token is a type modifier or datatype, we can expect form of (typeMod)*(datatype)+(typemod)* {* means 0+, + means 1+} \n");
            parseDeclaration ();
            return;
        }
    std::cout << "parsing kw exit1 \n";
}

record *
record::clone (int flags)
{
    auto r = new record (this->compiler, flags);
    return r;
}
void
record::parseExpressionableForOp (const char *op)
{
    ifdpm ("in parseexpressionableforop() \n");

    ParsePotentialExpressions ();
}
void
record::checkAndMakeExp (Token::token *tok)
{
    ifdpm ("checking and making Expression \n");
    const char *op = tok->stringVal;
    Node::node *left;
    if (compiler->vecNodes[0].size ())
        left = compiler->vecNodes[0].back();
    if(!left)
    {
        std::cout<<"FATAL ERROR MAYBE \n";
        return ;
    }
    switch (left->type)
        {
        // the IS_EXPRESSION_ABLE means the node can be part of an expression - a number, another expression, a variable
        IS_EXPRESSION_ABLE:
            ifdpm ("left node is a valid expression param\n");
            left->printNode (gaper, true);
            break;
        default:
            left = nullptr;
            ifdpm ("left node is not expression element! \n");
            return;
            break;
        }
    ifdpm ("token pointer moved forward \n");
    compiler->tokenPtr++;
    Node::popFrom (compiler->vecNodes,1);
    left->flags |= static_cast<int>(Node::flags::insideExpression);

    auto rec1 = this->clone (this->flags);
    rec1->parseExpressionableForOp (op);
    Node::node *right = Node::popFrom (compiler->vecNodes);
    ifdp parserDebugger << "NODE POPPED with type: " << right->type << std::endl;
    if (right)
        right->flags |= static_cast<int>(Node::flags::insideExpression);
    else
        {
            std::cout << "FATAL ERROR \n";
            exit (1);
        }
    ifdpm ("right node: \n");
    right->printNode (gaper, true);
    auto exp = new Node::node ();
    exp->type = Node::exp_;
    // exp->expVarUnion.expression
    exp->expVarUnion.expression.left = left;
    exp->expVarUnion.expression.right = right;
    exp->expVarUnion.expression.op = op;
    exp->reorderExpression ();
    exp->pushInto (compiler->vecNodes,1);
}

// The node around this operator will be an expression
// based on operation different functions
int
record::dealWithOp (Token::token *tok)
{

    ifdpm ("Dealing with Op \n");
    // for binary operations which have left and right operands, we check and make an expression node
    checkAndMakeExp (tok);
    return 0;
}
static int sucide;
// a number node or experession node or keyword node
int
record::makeOneNode ()
{
    Token::token *tok = compiler->tokenAt ();
    if (!tok)
        // EOF
        return 0;
    ifdpm ("entered makeOneNode with token ");
    ifdp tok->print (parserDebugger);

    // the flag can be used in debugging
    flags |= static_cast<int>(Node::flags::insideExpression);
    int res = 0;
    Node::node *n;
    switch (tok->type)
        {
        // token is a number, make a node out of it
        case static_cast<int> (Token::type::Num):
            n = new Node::node ();
            n->type = Node::number_;
            //ifdp parserDebugger << "NUMBER NODE INSERTED with val "<< tok->ullVal <<"\n";
            n->pushInto (compiler->vecNodes, 1);
            ifdpm ("number node made and pusehd to vecNodes \n");
            n->val.ullVal = tok->ullVal;
            ifdpm ("token pointer moved forward 1 position \n");
           // std::cout<<"\n\n-> "<< compiler->tokenPtr << ": "; 
            //compiler->tokenAt()->print();
            compiler->tokenPtr = compiler->tokenPtr+1;
            //std::cout<<"-> "<< compiler->tokenPtr << ": "; 
            //compiler->tokenAt()->print();
            res = 1;
            break;

        // identifier- straightforwadly a node
        case static_cast<int> (Token::type::ID):
            // parse identifier
            n = new Node::node ();
            n->type = Node::id_;
            ifdp parserDebugger << "ID NODE INSERTED with val "<< tok->stringVal <<"\n";
            n->pushInto (compiler->vecNodes);
            ifdpm ("Identifier node made and pusehd to vecNodes \n");
            
            n->val.stringVal = tok->stringVal;
            ifdpm ("token pointer moved 1 position \n");
            compiler->tokenPtr++;
            res = 1;
            break;

        case static_cast<int> (Token::type::OP):
            dealWithOp (tok);
            res = 1;
            break;

        case static_cast<int> (Token::type::KW):
            parseKw (tok);
            res = 1;
            break;

        default:
            //std::cout << "not handled this yet! \n";
            ifdpm("semicolon got in makeonenode so returning 0 \n");
            res = 0;
            break;
        }

    ifdpm ("returning from MakeOneNode: ");
    ifdp parserDebugger << res << std::endl;
    return res;
    // at this point we are pointing to a valid token
    // ptr is not yet incremented which is to be noted
}

void
record::ParsePotentialExpressions ()
{

    ifdpm ("In Parsing potential Expressions.. \n");
    // tokens are read, as long as they get along with the expression, reads.
    // for example a+(b+c(...))
    gaper++;
    while (record::makeOneNode ())
        ;
    // it fails when nodes are pushed, and maybe there was a token not part of the expression
    gaper--;
}

// this function specifically for global scope keywords
void
record::parseGlobalKeyword ()
{
    ifdpm ("entered parseGlobalKeyword ");
    parseKw (compiler->tokenAt (compiler->tokenPtr));
    // at this point we have a node ?
}

// will read 1 or more of tokens to make a Node
int
parser::parseNextNode ()
{
    gaper++;
    // if there is no more token to proces, We are done
    if (compiler->tokenPtr >= compiler->vecTokens[0].size ())
        {
            ifdpm ("No more tokens, parsing done ! \n");
            gaper--;
            return 0;
        }
    ifdp parserDebugger << "In parse_next() with tok \n";
    auto tok = compiler->tokenAt ();
    ifdp tok->print (parserDebugger);
    if (!tok)
        {
            gaper--;
            return 0;
        }
    int res = 0;
    auto rec = new record (0);
    rec->compiler = compiler;

    switch (tok->type)
        {
        // for Numbers, identifiers, and strings in general, they are part of expressions.
        case static_cast<int> (Token::type::Num):
        case static_cast<int> (Token::type::ID):
        case static_cast<int> (Token::type::Str):

            rec->ParsePotentialExpressions ();
            break;

        case static_cast<int> (Token::type::KW):
            rec->parseGlobalKeyword ();
            break;
        
        case static_cast<int>( Token::type::Sym ):
            rec->parseSymbol();
            break;

        default:
            if(tok->type ==  static_cast<int>(Token::type::Sym) )
                if(tok->charVal == ';')
                {
                    ifdpm("; to exit parsenextnode with 1\n");
                    compiler->tokenPtr++;
                    gaper--;
                    return 1;
                }
            compiler->genError ("this token cant be conv to node (or for now) \n");
            break;
        }
    gaper--;
    return 1;
}

// The main function which handles all parsing functionalities
int
parser::parse ()
{
    gaper = 0;
    ifdp parserDebugger << "started parsing \n";
    Node::node *node = nullptr;
    Token::token *last = nullptr;
    compiler->tokenPtr = 0; // it will point to the next token that is unprocessed
    name_giver = 0;
    sucide = 0;
    compiler->rootScopeCreateFree(1);
    compiler->printTokensFromCurPointer (parserDebugger);
    while (parseNextNode ())
        {
            compiler->printTokensFromCurPointer (parserDebugger);
            //std::cout << "Next token : \n";

            // commenting to test
            if (!compiler->vecNodes[0].size ())
                {
                    break;
                }

            node = compiler->vecNodes[0].back ();
            compiler->vecTree->push_back (node);
        }
    parserDebugger.close ();
    return 1;
}

size_t
DT::datatype::array::getSizeFromIndex (int index)
{
    if (index > multiDimSizes[0].size ()) // char* abc; return abc
        return size;
    int ptr = index;
    auto sizeNode = multiDimSizes[0][ptr++];
    if (!sizeNode)
        return 0;
    while (sizeNode)
        {
            if (sizeNode->type != static_cast<int> (Node::number_))
                return 0;
            size *= (sizeNode->expVarUnion.staticSize->val.longVal);
            sizeNode = multiDimSizes[0][ptr++];
        }
    return size;
}
int
DT::datatype::array::getTotIndicies (DT::datatype *dt)
{
    if (dt->flags & static_cast<int> (DT::flag::IS_ARRAY))
        return dt->array.multiDimSizes[0].size ();

    return -1;
}

arrSS *
record::parseArraySS ()
{
    ifdpm("Got [ so Enterred parseArraySS \n");
    auto arss = new arrSS ();
    auto tok = compiler->tokenAt ();
    while (tok->type == static_cast<int> (Token::type::OP) && !strcmp (tok->stringVal, "["))
        {

            ifdpm("pointing to [ ");
            compiler->tokenPtr++; // points to the size now  or it could also be closed brackets directly as it can be sizeless declaration
            tok = compiler->tokenAt ();
            if (tok->type == static_cast<int> (Token::type::Sym) && tok->charVal == ']')
                {
                    ifdpm("then pointing to ], so exit with pointer forward after ] \n");
                    compiler->tokenPtr++;
                    break;
                }
            ifdpm("got exp content so parsepotential exp \n");

            // int a[ 5+(7+8) ]
            ParsePotentialExpressions ();
            compiler->skipCharOrError (']');

            ifdpm("skipped ] after parse exp content, tok now: ");
            tok = compiler->tokenAt(); 
            ifdp tok->print(parserDebugger);

            auto node = Node::popFrom (compiler->vecNodes,1);
            auto newNode = new Node::node ();
            newNode->type = Node::bracket_;
            newNode->expVarUnion.staticSize = node;

            arss[0].push_back (newNode);
        }
    return arss;
}

