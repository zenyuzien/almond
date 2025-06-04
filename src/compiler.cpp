#include "compiler.h"
#include "lexer.h"
#include "parser.h"

#include <memory>
#include <stdlib.h>
#include <stdarg.h>
#include <cstring>
#include <iostream>

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
    ifdl vecTokens[0][tokenPtr]->print();

    
    while(tok)
    {
        if( tok->type == static_cast<int>(Token::type::Newl)||
            tok->type == static_cast<int>(Token::type::C)  ||
          ( tok->type == static_cast<int>(Token::type::Sym) && tok->charVal == '\\' )
        )// check newline comment and // 
           if(token_ptr < (vecTokens[0].size()-1)){
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

void compilation::genError(const char* msg, ...)
{
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
    
            std::cout<< "size: " <<(*vecTokens).size() << std::endl;
            for(auto x : (*vecTokens))
                x->print();
            

    std::cout<<"Lexed successfully ! \n";
    lexerDebugger.close();
    // parsing 
    
    
    auto parse_process = new parser();
    parse_process->compiler = this;
    vecNodes = new std::vector<Node::node*>();
    vecTree = new std::vector<Node::node*>();
    if(!parse_process)
        return 0;
    if(parse_process->parse() != 1)
        return 0;
    
    std::cout<< "Parsing success \n";

    std::cout<< "Nodes summary: size: "<< vecNodes[0].size()<<std::endl;
    for(auto x : vecNodes[0])
    {
        std::cout<<"_________________________\n";
        x->printNode(0);
        std::cout<<"_________________________\n";
        
    }


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

scope* compilation::rootScopeCreate(bool create) // 1 create, 0 free
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

        void SYM::symbolResolver::init()
        {
            compiler->symTableTable = new std::vector<std::vector<SYM::symbol*>*>();
            //new std::vector<SYM::symbol*>() ; 
        }
        void SYM::symbolResolver::push(SYM::symbol* s)
        {
            compiler->symTable->push_back(s);
        }
        void SYM::symbolResolver::newTable()
        {
            if(compiler->symTable)
                compiler->symTableTable->push_back(compiler->symTable);
            compiler->symTable = new std::vector<SYM::symbol*>();
        }
        void SYM::symbolResolver::endTable()
        {
            compiler->symTable = compiler->symTableTable[0].back();
            compiler->symTableTable[0].pop_back();
        }
        SYM::symbol* SYM::symbolResolver::get(std::string& name) 
        {
            compiler->symPtr =0 ;
            auto s = compiler->symTable[0][0];
            while(s)
            {
                if( s->name == name )
                    return s;
                s = compiler->symTable[0][++compiler->symPtr];
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
            auto n = std::string(name);
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