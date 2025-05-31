#include "compiler.h"
#include "lexer.h"
#include "parser.h"

#include <memory>
#include <stdlib.h>
#include <stdarg.h>
#include <cstring>
#include <iostream>
struct token* compilation::token_at()
{
    return token_at(token_ptr);
}
struct token* compilation::token_at(int token_ptr)
{
    token* tok= nullptr;
    if(token_ptr < (vec_t[0].size()))
        tok = vec_t[0][token_ptr];

    if(!tok)
    {
        ifdm("tok invalid from current pointer, eof \n");
        return nullptr; // EOF
    }

    ifdm("printing the token in single()\n");
    ifd vec_t[0][token_ptr]->print();

    while(tok)
    {
        if( tok->type == TT_Newl||
            tok->type == TT_C   ||
          ( tok->type == TT_Sym && tok->char_val == '\\' )
        )// check newline comment and // 
           if(token_ptr < (vec_t[0].size()-1)){
                tok = vec_t[0][++token_ptr]; 
                ifdm("INC PTR++ \n");
           }
           else // EOF
                return nullptr;
        else break;
    }
    // peek next end
    return tok;
}

void compilation::error_msg(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr," on line %i, col %i in file %s\n", line_no, col_no, path);
    exit(-1);
}
void compilation::warn_msg(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr," on line %i, col %i in file %s\n", line_no, col_no, path);
}
int compilation::compile_file
(
        const char* input_file, 
        const char* output_file,
        int flags
)
{    
    ifile = fopen(input_file,"r");
    ofile = fopen(output_file,"w");
    this->flags = flags;

    if(!ofile)
        return COMPILATION_FAILED;

    // lexical
    auto lex_process = new lexer(this,std::string{});



    if(!lex_process)
        return COMPILATION_FAILED;
    
    if(lex_process->lex() != LEXER_SUCCESS)
        return COMPILATION_FAILED;
    
            std::cout<< "size: " <<(*vec_t).size() << std::endl;
            for(auto x : (*vec_t))
                x->print();
            

    std::cout<<"Lexed successfully ! \n";
    // parsing 
    
    auto parse_process = new parser();
    parse_process->compiler = this;
    vec_n = new std::vector<Node::node*>();
    vec_tree = new std::vector<Node::node*>();
    if(!parse_process)
        return COMPILATION_FAILED;
    if(parse_process->parse() != PARSE_SUCCESS)
        return COMPILATION_FAILED;
    
    std::cout<< "Parsing success \n";

    std::cout<< "Nodes summary: size: "<< vec_n[0].size()<<std::endl;
    for(auto x : vec_n[0])
    {
        std::cout<<"______________\n";
        x->display(0);
        std::cout<<"______________\n";
        
    }


    // code gen

    return COMPILATION_SUCCESS;
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

scope* compilation::root_scope_create(bool create) // 1 create, 0 free
    {
        if(create)
        {
            root_scope = active_scope = new scope();
            return root_scope;
        }
        //root_scope->de_alloc();
        root_scope = active_scope = nullptr;
        return nullptr;
    }
    scope* compilation::new_scope(int flags)
    {
        auto new_scope = new scope();
        new_scope->flags = flags;
        new_scope->parent = active_scope;
        active_scope = new_scope;
        return new_scope;
    }
    void compilation::finish_scope()
    {
         // delete active_scope ; can't do it yet, so we do this instead :
        active_scope = active_scope->parent;
        if(!active_scope) root_scope = nullptr;
    }
    void compilation::push_scope(void* address, size_t size)
    {
        active_scope->instances->push_back(address);
        active_scope->size += size;
    }
    void* compilation::scope_from_to(scope* s, scope* e)
    {
        if( s == e ) return nullptr; 
        auto last = s->top();
        if(last) return last; 
        auto parent = s->parent;
        if(parent) return scope_from_to(parent,e);
        return nullptr;
    }
    void* compilation::scope_last_instance()
    {
        return scope_from_to( active_scope, nullptr );
    }

    void scope::iterate(bool start) // 0- start , 1-end // dec true means ptr decreases
    {
        if(start)
        {
            ptr = dec ? instances->size()-1 : 0 ;
            return ;
        }
    }
    void* scope::instance_at(int index)
    {
        if(instances->size()==0) return nullptr; 
        return instances[0][index];
    }
    void* scope::top()
    {
        if(instances->size()==0) return nullptr; 
        return instances[0][instances->size()-1];
    }

        void SYM::symbol_resolver::init()
        {
            compiler->sym_table_table = new std::vector<std::vector<symbol*>*>();
            //new std::vector<SYM::symbol*>() ; 
        }
        void SYM::symbol_resolver::push(symbol* s)
        {
            compiler->sym_table->push_back(s);
        }
        void SYM::symbol_resolver::new_table()
        {
            if(compiler->sym_table)
                compiler->sym_table_table->push_back(compiler->sym_table);
            compiler->sym_table = new std::vector<SYM::symbol*>();
        }
        void SYM::symbol_resolver::end_table()
        {
            compiler->sym_table = compiler->sym_table_table[0].back();
            compiler->sym_table_table[0].pop_back();
        }
        SYM::symbol* SYM::symbol_resolver::get(std::string& name) 
        {
            compiler->sym_ptr =0 ;
            auto s = compiler->sym_table[0][0];
            while(s)
            {
                if( s->name == name )
                    return s;
                s = compiler->sym_table[0][++compiler->sym_ptr];
            }
            return nullptr; 
        }
        SYM::symbol* SYM::symbol_resolver::get_for_nf(std::string& name)
        {
            auto s = get(name);
            if(!s)
                return nullptr; 
            if(s->type & static_cast<int>(SYM::type::native_f))
                return nullptr; 
            return s;
        }
        SYM::symbol* SYM::symbol_resolver::make_symbol(const char* name, int type, void* content)
        {
            auto n = std::string(name);
            if( SYM::symbol_resolver::get(n) )
                return nullptr;
            auto sym = new SYM::symbol();
            sym->name = n ;
            sym->metadata = content; 
            sym->type = 0 | type;
            return nullptr; 
        }
        Node::node* SYM::symbol_resolver::node(symbol* sym)
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