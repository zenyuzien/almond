#ifndef _almond 
#define _almond 

#include <string>
#include <stdio.h>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "token.h"
#include <iostream>
#include <memory.h>
#include <vector>
#include <string>
#include <bitset>
#include <inttypes.h> // for PRIu16, PRIu64



extern bool debugParse ;
extern bool customParse;
#define ifd if(debugParse)
#define ifc if(customParse)
#define ifdm(msg) do { if (debugParse) std::cout << msg ; } while (0)
#define ifcm(msg) do { if (customParse) std::cout << msg ; } while (0)

struct compilation;
namespace Node { struct node ;}
namespace DT { struct datatype ;}

namespace SYM 
{
    enum class type 
    {
        node     = 0b0001,
        native_f = 0b0010,
        undef    = 0b0100
    };
    struct symbol
    {
        std::string name; 
        int type; 
        void* metadata;
    };
    struct symbolResolver
    {
        compilation* compiler; 
        void init();
        void push(symbol* s);
        void newTable();
        void endTable();
        symbol* get(std::string& name);
        symbol* getForNF(std::string& name);
        symbol* makeSymbol(const char* name, int type, void* content);
        Node::node* node(symbol* sym);

        /*
            building for variables, functions, structures, unions 
            pending
        */
        void buildForNode(Node::node* node);
    };
};



struct scope;
struct compilation
{
    int lineNo , colNo , flags; 
    int tokenPtr; 

    int scopePtr; 
    scope* rootScope, *activeScope; 

    FILE *ifile, *ofile;
    const char* path; 
    std::vector<Token::token*>* vecTokens;
    std::vector<Node::node*> *vecNodes, *vecTree;

    std::vector<SYM::symbol*>* symTable; 
    std::vector< std::vector<SYM::symbol*>* >* symTableTable; 

    SYM::symbolResolver* symResolver; 
    int symPtr;

    compilation() : 
        lineNo(1),
        colNo(1),
        flags(0),
        tokenPtr(0),
        scopePtr(0),
        rootScope(nullptr),
        activeScope(nullptr),
        ifile(nullptr),
        ofile(nullptr),
        path(nullptr),
        symTable(nullptr)
        //vec_t(new std::vector<Token::token*>()),
        //vecNodes(new std::vector<Node::node*>()),
        //vecTree(new std::vector<Node::node*>()),
        //sym_table_table(new std::vector<std::vector<symbol*>*>())
    {}


    ~compilation() {
        if (ifile) fclose(ifile);
        if (ofile) fclose(ofile);
    }
    int compileFile(
        const char* inputFile, 
        const char* outputFile,
        int flags
    );
    void genError(const char* msg, ...);
    void genWarning(const char *msg, ...);
    struct Token::token* tokenAt();
    struct Token::token* tokenAt(int ptr);
    //lexer* sandbox(std::string& custom);

    scope* rootScopeCreate(bool create);
    scope* newScope(int flags);
    void finishScope();
    void pushScope(void* address, size_t size);
    void* scopeFromTo(scope* s, scope* e);
    void* scopeLastInstance();

};

struct scope
{
    bool dec; // may remove;
    int flags, ptr; 
    std::vector<void*>* instances;
    size_t size;  
    scope* parent;
    compilation* compiler;

    scope()
        : ptr(0), flags(0), size(0), parent(nullptr), compiler(nullptr)
    {
        instances = new std::vector<void*>();
        dec = true; // LOOK HERE
    }

    ~scope() {
       // delete instances;
    }
  
    void iterate(bool start);
    void* instanceAt(int index);
    void* top();
};



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
        int rowNo, colNo;
        const char* path; 
        struct nodeBinded
        {
            node* head = nullptr; // body
            node* func = nullptr; // function

            nodeBinded() = default;
            ~nodeBinded() = default;
        }binded;// as a pointer ?
        union 
        {
            struct expression
            {
                Node::node *left,*right;
                const char*op;
            }exp;

            struct variable 
            {
                DT::datatype* type;
                const char* name; 
                node* val;
            }var;


        }expVarUnion;
        union 
        {
            char charVal; 
            char* stringVal;
            unsigned int integerVal ; 
            unsigned long longVal; 
            unsigned long long ullVal ;
            void* any; 
        }val; // val not in peach
        node()
            : type(0), flags(0), rowNo(0), colNo(0), path(nullptr)
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
        void display(int);
        void nodeShiftChildrenLeft();
        void reorderExpression();
        
        // idk what this node_set_vector is trying to do
    };
    node* top(std::vector<node*>* v);
    node* pop(std::vector<node*>* v);
};

struct record 
{
    compilation* compiler;
    int flags;
    record(compilation* c, int f =0)
    {
        compiler = c;
        flags =f ;
    }
    int parse_expressionable_single();
    void parse_expressionable();
    void parse_expressionable_for_op(const char* op);
    void parse_exp_normal(Token::token* tok);
    int parse_exp(Token::token* tok);
    void parse_kw(Token::token* tok);
    void parse_var_func_struct_union();
    void parse_kw_for_global();
    //void parse_exressionable_root();
    record * clone(int flags);
};

namespace DT 
{
    enum class flag : uint16_t {
        IS_SIGNED               = 0x01,
        IS_STATIC               = 0x02,
        IS_CONST                = 0x04,
        IS_POINTER              = 0x08,
        IS_ARRAY                = 0x10,
        IS_EXTERN               = 0x20,
        IS_RESTRICT             = 0x40,
        IGNORE_TYPE_CHECKING    = 0x80,
        IS_SECONDARY            = 0x100,
        STRUCT_UNION_NO_NAME    = 0x200,
        IS_LITERAL              = 0x400
    };
    struct datatype 
    {
        int pdepth,type; // can be int or float or long 
        uint16_t flags; // refer enum class DT 
        size_t size;
        struct datatype* sec; // long long
        const char* type_str ;
        union 
        {
            struct node* struct_node, *union_node ;
        };
        void print();
        void parse(compilation* c);

        bool check_flag(DT::flag f);
        void set_flag(DT::flag f);
        void unset_flag(DT::flag f);

        void parse_datatype_modifiers(compilation* c);
        void parse_datatype_type(compilation* c);
        void parser_datatype_init(compilation* c, Token::token* dt1,Token::token*dt2, int stars, int expectation );
    
        void parse_var(compilation* c, Token::token* tok, record* h);
    };
    enum class type
    {
        void_,
        char_,
        short_,
        int_,
        long_,
        double_,
        struct_,
        union_,
        unknown_
    };
    enum class expect
    {
        primitive_,
        union_,
        struct_
    };
    bool is_datatype(const char* str);
    bool is_var_modifier(const char* str);
    bool has_datatype_changed(struct datatype* current);
};



struct parser {

    compilation* compiler;

    parser() {
        compiler = nullptr;
    }

    ~parser() {
        if(compiler)
        {
            for (Node::node* n : *compiler->vecNodes) delete n;
            for (Node::node* n : *compiler->vecTree) delete n;
        }

        delete compiler->vecNodes;
        delete compiler->vecTree;
    }

    int parse();
    int parse_next();
};

const int left_to_right = 0;
const int right_to_left = 1;

struct parse_priority {
    std::vector<std::string> operators;
    int direction;
};




#endif