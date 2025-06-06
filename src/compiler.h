#ifndef _almond
#define _almond

#include "token.h"
#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <inttypes.h> // for PRIu16, PRIu64
#include <iostream>
#include <memory.h>
#include <string>
#include <vector>

#define dot std::cout << "."

extern bool LEXER_DEBUG;
extern std::ofstream lexerDebugger;
#define ifdl if (LEXER_DEBUG)
#define ifdlm(msg)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    if (LEXER_DEBUG)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
        lexerDebugger << msg;

// forward declarations to fix cross-reference issues
struct compilation;
namespace Node
{
struct node;
}
typedef std::vector<Node::node *> arrSS; // array static sizes aka array brackets
namespace DT
{
struct datatype;
}

// namespace for 'symbol' used in symbol table operations
namespace SYM
{
// a symbol can be one of the three.
// TODO: why flags when mutually exclusive ?
enum class type
{
    node = 0b0001,
    nativeF = 0b0010,
    undef = 0b0100
};
// the structure of the actual symbol
struct symbol
{
    std::string name;
    int type;
    void *metadata;
};
struct symbolResolver
{
    compilation *compiler;
    void init ();
    void push (symbol *s);
    void newTable ();
    void endTable ();
    symbol *get (std::string &name);
    symbol *getForNF (std::string &name);
    symbol *makeSymbol (const char *name, int type, void *content);
    Node::node *node (symbol *sym);

    /*
        building for variables, functions, structures, unions
        pending
    */
    void buildForNode (Node::node *node);
};
};

struct scope;
struct compilation
{
    int lineNo, colNo, flags;
    int tokenPtr;

    int scopePtr;
    scope *rootScope, *activeScope;

    FILE *ifile, *ofile;
    const char *path;
    std::vector<Token::token *> *vecTokens;
    std::vector<Node::node *> *vecNodes, *vecTree;

    std::vector<SYM::symbol *> *symTable;
    std::vector<std::vector<SYM::symbol *> *> *symTableTable;

    SYM::symbolResolver *symResolver;
    int symPtr;

    compilation () : lineNo (1), colNo (1), flags (0), tokenPtr (0), scopePtr (0), rootScope (nullptr), activeScope (nullptr), ifile (nullptr), ofile (nullptr), path (nullptr), symTable (nullptr)
    // vec_t(new std::vector<Token::token*>()),
    // vecNodes(new std::vector<Node::node*>()),
    // vecTree(new std::vector<Node::node*>()),
    // sym_table_table(new std::vector<std::vector<symbol*>*>())
    {
    }

    ~compilation ()
    {
        if (ifile)
            fclose (ifile);
        if (ofile)
            fclose (ofile);
    }
    int compileFile (const char *inputFile, const char *outputFile, int flags);
    void genError (const char *msg, ...);
    void genWarning (const char *msg, ...);
    struct Token::token *tokenAt ();
    struct Token::token *tokenAt (int ptr);
    // lexer* sandbox(std::string& custom);

    scope *rootScopeCreate (bool create);
    scope *newScope (int flags);
    void finishScope ();
    void pushScope (void *address, size_t size);
    void *scopeFromTo (scope *s, scope *e);
    void *scopeLastInstance ();
    void printTokensFromCurPointer (std::ofstream &wr);
    void printTokensFromCurPointer ();
    void skipCharOrError (char c);
};

struct scope
{
    bool dec; // may remove;
    int flags, ptr;
    std::vector<void *> *instances;
    size_t size;
    scope *parent;
    compilation *compiler;

    scope () : ptr (0), flags (0), size (0), parent (nullptr), compiler (nullptr)
    {
        instances = new std::vector<void *> ();
        dec = true; // LOOK HERE
    }

    ~scope ()
    {
        // delete instances;
    }

    void iterate (bool start);
    void *instanceAt (int index);
    void *top ();
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
    const char *path;
    /*struct nodeBinded
    {
        node *head = nullptr; // body
        node *func = nullptr; // function

        nodeBinded () = default;
        ~nodeBinded () = default;
    } binded; // as a pointer ?*/
    union
    {
        struct
        {
            Node::node *left, *right;
            const char *op;
        } expression;

        struct
        {
            DT::datatype *type;
            const char *name;
            node *val;
        } variable;
        std::vector<Node::node *> *VariableList;
        Node::node *staticSize; // bracket eg. NUMTYPE,3 in int a[3];

    } expVarUnion;
    union
    {
        char charVal;
        char *stringVal;
        unsigned int integerVal;
        unsigned long longVal;
        unsigned long long ullVal;
        void *any;
    } val; // val not in peach
    node () : type (0), flags (0), rowNo (0), colNo (0), path (nullptr)
    {
        // binding here required
        // push node ? to where ?
        val.any = nullptr; // safely init union
    }

    // Destructor
    ~node ()
    {
        // If you use string_val or dynamically allocated memory, free it here
        // delete[] value.string_val; // Uncomment only if memory is owned by node
    }

    // implementatios of these can be found in parser.cpp
    void pushInto (std::vector<node *> *v);
    void printNode (int, bool isdebug = false);
    void nodeShiftChildrenLeft ();
    void reorderExpression (int lev = 0);

    // idk what this nodeSet_vector is trying to do
};
node *topOf (std::vector<node *> *v);
node *popFrom (std::vector<node *> *v);
};

struct record
{
    compilation *compiler;
    int flags;
    record (compilation *c, int f = 0)
    {
        compiler = c;
        flags = f;
    }
    int makeOneNode ();
    void ParsePotentialExpressions ();
    void parseExpressionableForOp (const char *op);
    void checkAndMakeExp (Token::token *tok);
    int dealWithOp (Token::token *tok);
    void parseKw (Token::token *tok);
    void parseDeclaration ();
    void parseGlobalKeyword ();
    // void parseExressionable_root();
    record *clone (int flags);
    arrSS *parseArraySS ();
};

namespace DT
{
enum class flag : uint16_t
{
    IS_SIGNED = 0x01,
    IS_STATIC = 0x02,
    IS_CONST = 0x04,
    IS_POINTER = 0x08,
    IS_ARRAY = 0x10,
    IS_EXTERN = 0x20,
    IS_RESTRICT = 0x40,
    IGNORE_TYPE_CHECKING = 0x80,
    IS_SECONDARY = 0x100,
    STRUCT_UNION_NO_NAME = 0x200,
    IS_LITERAL = 0x400
};
struct datatype
{
    int pdepth, type; // can be int or float or long
    uint16_t flags;   // refer enum class DT
    size_t size;
    struct datatype *sec; // long long
    const char *typeStr;
    union
    {
        struct node *structNode, *unionNode;
    };
    struct array
    {
        arrSS *multiDimSizes; // array
        size_t size;

        size_t getSizeFromIndex (int index = 0);
        int getTotIndicies (DT::datatype *);

    } array;
    void print (bool isdebug = false);
    void parse (compilation *c);

    bool checkFlag (DT::flag f);
    void setFlag (DT::flag f);
    void unsetFlag (DT::flag f);

    void parseDatatypeModifiers (compilation *c);
    void parseDatatypeType (compilation *c);
    void parserDatatypeInit (compilation *c, Token::token *dt1, Token::token *dt2, int stars, int expectation);

    void parseVar (compilation *c, Token::token *tok, record *h);
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
bool isDatatype (const char *str);
bool isTypeModifier (const char *str);
bool hasDatatypeChanged (struct datatype *current);
};

struct parser
{

    compilation *compiler;

    parser () { compiler = nullptr; }

    ~parser ()
    {
        if (compiler)
            {
                for (Node::node *n : *compiler->vecNodes)
                    delete n;
                for (Node::node *n : *compiler->vecTree)
                    delete n;
            }

        delete compiler->vecNodes;
        delete compiler->vecTree;
    }

    int parse ();
    int parseNextNode ();
};

const bool leftToRight = 0;
const bool rightToLeft = 1;

struct parsePriority
{
    std::vector<std::string> operators;
    bool direction;
};

#endif