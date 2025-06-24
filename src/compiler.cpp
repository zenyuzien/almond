#include "compiler.h"
#include "lexer.h"

#include <memory>
#include <stdlib.h>
#include <stdarg.h>
#include <cstring>
#include <iostream>

extern std::ofstream parserDebugger;


Token::token* compilation::tokenAt()
{
    
    return tokenAt(tokenPtr);
}
struct Token::token* compilation::tokenAt(int token_ptr)
{
    Token::token* tok= nullptr;
    if(token_ptr < (vecTokens[0].size()))
        tok = vecTokens[0][token_ptr];

    if(!tok)
    {
        ifdlm("tok invalid from current pointer, eof \n");
        return nullptr; // EOF
    }

    ifdlm("printing the Token::token in single()\n");
    ifdl vecTokens[0][tokenPtr]->print(lexerDebugger);

    
    while(tok)
    {
        if( tok->type == static_cast<int>(Token::type::Newl)||
            tok->type == static_cast<int>(Token::type::C)  ||
          ( tok->type == static_cast<int>(Token::type::Sym) && tok->charVal == '\\' )
        )// check newline comment and // 
           if(token_ptr < (vecTokens[0].size()-1)){
                tokenPtr++;
                tok = vecTokens[0][++token_ptr]; 
                ifdlm("INC PTR++ \n");
           }
           else // EOF
                return nullptr;
        else break;
    }
    // peek next end
    return tok;
}
void
compilation::skipCharOrError (char c)
{
    auto tok = tokenAt ();
    tokenPtr++;
    if (!tok || tok->type != static_cast<int> (Token::type::Sym) || tok->charVal != c)
        genError ("Char %c is expected, %c not allowed \n",tok->charVal, c );
}
void compilation::genError(const char* msg, ...)
{
    parserDebugger.close();
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr," on line %i, col %i in file %s\n", lineNo, colNo, path);
    exit(-1);
}
void compilation::genWarning(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr," on line %i, col %i in file %s\n", lineNo, colNo, path);
}
int compilation::compileFile
(
        const char* input_file, 
        const char* output_file,
        int flags
)
{    
    ifile = fopen(input_file,"r");
    ofile = fopen(output_file,"w");
    if(!ifile)
    {
        std::cout << "Couldn't open input file "<<input_file<<"\n";
        exit(-1);
    }
    if(!ofile)
    {
        std::cout << "Couldn't open output file "<<output_file<<"\n";
        exit(-1);        
    }
    this->flags = flags;

    if(!ofile)
        return 0;

    // lexical
    auto lex_process = new lexer(this);

    if(!lex_process || (!lex_process->lex()))
        return 0;
    
            ifdl lexerDebugger<< "\n\nSummary: Token count is " <<(*vecTokens).size() << std::endl;
            for(auto x : (*vecTokens))
                ifdl x->print(lexerDebugger);

    std::cout<<"\nLexed successfully ! \n";
    lexerDebugger.close();
    // parsing 

    symResolver = new SYM::symbolResolver();
    symResolver->compiler = this;
    symResolver->init();
    symResolver->newTable();
    
    parserActiveBody = nullptr;
    auto parse_process = new parser();
    parse_process->compiler = this;
    vecNodes = new std::vector<Node::node*>();
    vecTree = new std::vector<Node::node*>();
    if(!parse_process)
        return 0;
    if(parse_process->parse() != 1)
        return 0;
    
    std::cout<< "\nParsed successfully !\n\n";

    std::cout<< "Nodes summary: size: "<< vecNodes[0].size()<<std::endl;
    for(auto x : vecNodes[0])
    {
        std::cout<<"_________________________\n";
        x->printNode(0);
        
    }
        std::cout<<"_________________________\n";


    // code gen

    return 1;
}
/*
lexer* compilation::sandbox(std::string& custom)
{
    std::string tmp = custom;
    auto lexp = new lexer(this, strdup(custom.c_str()));
    if(!lexp)
        return NULL; 
    if( lexp->lex() != LEXER_SUCCESS )
        return NULL;
    return lexp;
}*/
scope* compilation::rootScopeCreateFree(bool create) // 1 create, 0 free
    {
        if(create)
        {
            rootScope = activeScope = new scope();
            return rootScope;
        }
        //rootScope->de_alloc();
        rootScope = activeScope = nullptr;
        return nullptr;
    }
    scope* compilation::newScope(int flags)
    {
        auto new_scope = new scope();
        new_scope->flags = flags;
        new_scope->parent = activeScope;
        activeScope = new_scope;
        return new_scope;
    }
    void compilation::finishScope()
    {
         // delete activeScope ; can't do it yet, so we do this instead :
        activeScope = activeScope->parent;
        if(!activeScope) rootScope = nullptr;
    }
    void compilation::pushScope(void* address, size_t size)
    {
        activeScope->instances->push_back(address);
        activeScope->size += size;
    }
    void* compilation::scopeFromTo(scope* s, scope* e)
    {
        if( s == e ) return nullptr; 
        auto last = s->top();
        if(last) return last; 
        auto parent = s->parent;
        if(parent) return scopeFromTo(parent,e);
        return nullptr;
    }
    void* compilation::scopeLastInstance()
    {
        return scopeFromTo( activeScope, nullptr );
    }

    void scope::iterate(bool start) // 0- start , 1-end // dec true means ptr decreases
    {
        if(start)
        {
            ptr = dec ? instances->size()-1 : 0 ;
            return ;
        }
    }
    void* scope::instanceAt(int index)
    {
        if(instances->size()==0) return nullptr; 
        return instances[0][index];
    }
    void* scope::top()
    {
        if(instances->size()==0) return nullptr; 
        return instances[0][instances->size()-1];
    }

    Node::node* SYM::symbolResolver::nodeFromSym(SYM::symbol* sym)
    {
        if(sym->type != static_cast<int>(SYM::type::node) )
            return NULL;
        return (Node::node*)sym->metadata;
    }
    Node::node* SYM::symbolResolver::nodeFromSymbol(std::string& name)
    {
        struct SYM::symbol* sym ; 
        sym = get( name );
        if(!sym)
            return nullptr; 
        return nodeFromSym(sym);
    }
    Node::node* SYM::symbolResolver::nodeForName(std::string& name)
    {
        Node::node* n ; 
        n = nodeFromSymbol(name);
        if(!n || (n->type != Node::struct_))
            return nullptr; 
        return n;
    }

        void SYM::symbolResolver::init()
        {
            compiler->symTableTable = new std::vector< std::vector<SYM::symbol*>* >();
            //new std::vector<SYM::symbol*>() ; 
        }
        void SYM::symbolResolver::push(SYM::symbol* s)
        {
            compiler->symTable->push_back(s);
        }
        void SYM::symbolResolver::newTable()
        {
            if(compiler->symTable)
                compiler->symTableTable[0].push_back(compiler->symTable);
            compiler->symTable = new std::vector<SYM::symbol*>();
        }
        void SYM::symbolResolver::endTable()
        {
            compiler->symTable = compiler->symTableTable[0].back();
            compiler->symTableTable[0].pop_back();
            /*
        std::vector<SYM::symbol *> *symTable;
        std::vector<std::vector<SYM::symbol *>> *symTableTable;
            */
        }
        SYM::symbol* SYM::symbolResolver::get(std::string& name) 
        {
            compiler->symPtr =0 ;
            auto tmp = compiler->symTable[0];
            SYM::symbol* s;
            if(tmp.size())
                s = compiler->symTable[0][0];
            while(s)
            {
                //std::cout<<compiler->symTable[0].size()<<" ";
                if( s->name == name )
                    return s;
                ++compiler->symPtr;
                if( compiler->symTable[0].size() > compiler->symPtr)
                s = compiler->symTable[0][compiler->symPtr];
                else break;
            }
            return nullptr; 
        }
        SYM::symbol* SYM::symbolResolver::getForNF(std::string& name)
        {
            auto s = get(name);
            if(!s)
                return nullptr; 
            if(s->type & static_cast<int>(SYM::type::nativeF))
                return nullptr; 
            return s;
        }
        SYM::symbol* SYM::symbolResolver::makeSymbol(const char* name, int type, void* content)
        {
            std::string n;
            if(name) n = std::string(name);
            if( SYM::symbolResolver::get(n) )
                return nullptr;
            auto sym = new SYM::symbol();
            sym->name = n ;
            sym->metadata = content; 
            sym->type = 0 | type;
            return nullptr; 
        }
        Node::node* SYM::symbolResolver::node(SYM::symbol* sym)
        {
            return (sym->type & static_cast<int>(SYM::type::node)) ? nullptr : static_cast<Node::node*>(sym->metadata);
        }

        /*
            building for variables, functions, structures, unions 
            pending
        */
        void build_for_node(Node::node* node)
        {
            switch(node->type)
            {
                case Node::var_ : 
                    break; 
                case Node::func_:
                    break;
                case Node::struct_:
                    break;
                case Node::union_:
                    break;
                default:
                    std::cout<<"FATAL ERROR \n";
                    exit(-1);
            }
        }
void compilation::printTokensFromCurPointer(int count)
{
    if(!count ||  ((tokenPtr +  count) >= vecTokens[0].size()) )
        count = vecTokens[0].size() ;

    std::cout<< "Tokens from cur_pointer: " << tokenPtr << std::endl;
    for(int i = tokenPtr ;i < count; i++ )
        vecTokens[0][i]->print();
}
void compilation::printTokensFromCurPointer(std::ofstream& wr, int count)
{
    /*
        Let's say there are 3 tokenn in total
        the pointer is at 0th 
        and count is 1. 
        then we check if 0th + 1 is a valid one within bounds 
        ptr + count < size is the condition 
        if count is 3, 0th + 3 is not existant    
    */
    wr<<" size is "<< vecTokens[0].size() << " and cnt is "<< count << " with ptr: "<< tokenPtr  <<std::endl;
    if(!count ||  ((tokenPtr +  count) >= vecTokens[0].size()) )
        count = vecTokens[0].size() - (tokenPtr +1) ; // if size 3, ptr at 1, cnt 3, then count should become 1.  

    wr<< "_____Toks from cur_pointer: " << tokenPtr << " _________ cnt " << count << std::endl;
    for(int i = tokenPtr ; i < (tokenPtr+count); i++ )
        vecTokens[0][i]->print(wr);
    wr << "______________________________________\n";
}
int padding(int val, int to)
{
    if(( to <=0 ) || !(val%to) ) // reduncant TODO
        return 0;
    return to - (val%to) % to ;
}
int align(int val, int to)
{
    if(val%to)
        val += padding(val,to);
    return val;
}
int alignPositive(int val, int to) // negatives included for offsets
{
    if(! to>=0)
        return -1;
    if(val<0)
        to = -to;
    return align(val,to);
}
int computeSumPadding( std::vector<Node::node*> *list )
{
    int padding =0; 
    int lastType = -1; 
    bool mixedTypes= false ;
    int ptr = 0; 
    Node::node* cur = list[0][ptr++],
    *last  = nullptr;
    while(cur)
    {
        if(cur->type != Node::var_)
        {
            cur = list[0][ptr++];
            continue;
        }
        padding += cur->expVarUnion.variable.padding;
        lastType = cur->expVarUnion.variable.type->type;
        last = cur; // why use last ? 
        cur = list[0][ptr++];
    }
    return padding;
}

void compilation::symresolverBuildForNode(Node::node* n)
{
    switch( n->type)
    {
        case Node::struct_:

            if(n->flags & static_cast<int>(Node::flags::forwardDeclaration) )
                return;
            symResolver->makeSymbol(n->expVarUnion.structure.name,  static_cast<int>( SYM::type::node), n);

        break;

        default:
        break;
    }
}
