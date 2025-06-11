#include "compiler.h"
#include <iomanip>
#include <iostream>
#include <memory.h>

extern int gaper;
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

// This structure stores the priority table of all operators in C
const std::vector<parsePriority> opPrecedence = { { { "++", "--", "()", "[]", "(", "[", ".", "->" }, leftToRight },
                                                  { { "*", "/", "%" }, leftToRight },
                                                  { { "+", "-" }, leftToRight },
                                                  { { "<<", ">>" }, leftToRight },
                                                  { { "<", "<=", ">", ">=" }, leftToRight },
                                                  { { "==", "!=" }, leftToRight },
                                                  { { "&" }, leftToRight },
                                                  { { "^" }, leftToRight },
                                                  { { "|" }, leftToRight },
                                                  { { "&&" }, leftToRight },
                                                  { { "||" }, leftToRight },
                                                  { { "?", ":" }, rightToLeft },
                                                  { { "=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^=", "|=" }, rightToLeft },
                                                  { { "," }, leftToRight } };

int
getPriorityFor (const char *op)
{
    for (int i = 0; i < opPrecedence.size (); i++)
        for (auto j : opPrecedence[i].operators)
            if (op == j)
                return i;
    return -1;
}

bool
shouldWeEvalLeft (const char *left, const char *right)
{
    if (left == right)
        return false;
    int left_rank = getPriorityFor (left);
    int right_rank = getPriorityFor (right);

    if (opPrecedence[left_rank].direction == rightToLeft)
        return false;

    return left_rank <= right_rank;
}

void
Node::node::pushInto (std::vector<Node::node *> *v)
{
    v->push_back (this);
}
Node::node *
Node::topOf (std::vector<Node::node *> *v)
{
    if (v->size ())
        return v->back ();
    return nullptr;
}
Node::node *
Node::popFrom (std::vector<Node::node *> *v)
{
    if (v->size ())
        {
            auto x = v->back ();
            v->pop_back ();
            return x;
        }
    return nullptr;
}
std::string printNodeUtility(Node::node*);
std::string printNodeUtilityDT(DT::datatype* dt)
{
    std::string result ; 
    result += (dt->flags & static_cast<uint16_t>(DT::flag::IS_SIGNED))            ? '1' : '0';
    result += (dt->flags & static_cast<uint16_t>(DT::flag::IS_STATIC))            ? '1' : '0';
    result += (dt->flags & static_cast<uint16_t>(DT::flag::IS_CONST))             ? '1' : '0';
    result += (dt->flags & static_cast<uint16_t>(DT::flag::IS_POINTER))           ? '1' : '0';
    result += (dt->flags & static_cast<uint16_t>(DT::flag::IS_ARRAY))             ? '1' : '0';
    result += (dt->flags & static_cast<uint16_t>(DT::flag::IS_EXTERN))            ? '1' : '0';
    result += (dt->flags & static_cast<uint16_t>(DT::flag::IS_RESTRICT))          ? '1' : '0';
    result += (dt->flags & static_cast<uint16_t>(DT::flag::IGNORE_TYPE_CHECKING)) ? '1' : '0';
    result += (dt->flags & static_cast<uint16_t>(DT::flag::IS_SECONDARY))         ? '1' : '0';
    result += (dt->flags & static_cast<uint16_t>(DT::flag::STRUCT_UNION_NO_NAME)) ? '1' : '0';
    result += (dt->flags & static_cast<uint16_t>(DT::flag::IS_LITERAL))           ? "1 " : "0 ";
    result += dt->typeStr ;
    result += " ";
    if(dt->sec) 
    {
        result += std::string(dt->typeStr);
        result += " ";
    }
    if(dt->pdepth)
    {
        for(int i = 0 ; i < dt->pdepth ; i++) 
            result += '*';
        result += " ";
    }
    if( dt->array.multiDimSizes )
    {
        auto dims = dt->array.multiDimSizes[0];
        for(auto n : dims)
            result = result + "[" + printNodeUtility(n) + "] ";
    }
    return result;
}

std::string printNodeUtility(Node::node* exp)
{
    if (!exp) return "[null]";  // Safety check

    std::string tmp;

    switch (exp->type)
    {
        case Node::bracket_:
            return printNodeUtility(exp->expVarUnion.staticSize);
        break;

        case Node::var_:
            tmp += printNodeUtilityDT(exp->expVarUnion.variable.type);
            tmp += exp->expVarUnion.variable.name;
            if(exp->expVarUnion.variable.val)
            {
                tmp += " = " ;
                tmp += printNodeUtility(exp->expVarUnion.variable.val);
            }
        break;

        case Node::exp_:
            tmp += "(";
            tmp += printNodeUtility(exp->expVarUnion.expression.left);
            tmp += exp->expVarUnion.expression.op;
            tmp += printNodeUtility(exp->expVarUnion.expression.right);
            tmp += ")";
        break;

        case Node::number_:
            return std::to_string(exp->val.ullVal);

        case Node::id_:
            return exp->val.stringVal ? std::string(exp->val.stringVal) : "[null_id]";

        default:
            return "[unknown_node_type]";
    }

    return tmp;
}


void
Node::node::printNode (int level, bool isDebug)
{
    if (isDebug)
        {
            if (type == exp_)
                {
                    gapd(level);
                    parserDebugger << printNodeUtility(this) << std::endl;
                }
            else if (type == number_)
                {
                    gapd (level);
                    parserDebugger << "number val: " << val.ullVal << std::endl;
                }
            else if (type == var_)
                {
                    gapd (level);
                    parserDebugger << "var type, flags: " << flags << " union: \n";
                    gapd (level);
                    parserDebugger << "var name: " << expVarUnion.variable.name << " val: " << expVarUnion.variable.val << std::endl;
                }
            return;
        }

    
    gap(level);
    std::cout << printNodeUtility(this) << std::endl;
    return;
    /*

    if (type == exp_)
        {
            gap(level);
            std::cout << printNodeUtility(this) << std::endl;
        }
    else if (type == number_)
        {
            gap (level);
            std::cout << "number val: " << val.ullVal << std::endl;
        }
    else if (type == var_)
        {
            gap (level);
            std::cout << "var type, flags: " << flags << " union: \n";
            gap (level);
            std::cout << "var name: " << expVarUnion.variable.name << "\n"; // << " val: " << expVarUnion.variable.val->val.ullVal << std::endl;
        }*/
}

void
Node::node::nodeShiftChildrenLeft ()
{
    // ip num1 * (num2 + num3)
    // op (num1 * num2) + num3
    if (type != Node::exp_ || (expVarUnion.expression.right->type != Node::exp_))
        {
            ifdpm ("can't shift left: invalid op \n");
            exit (-1);
        }
    const char *rightOp = expVarUnion.expression.right->expVarUnion.expression.op;
    Node::node *newLeftchild = expVarUnion.expression.left;
    Node::node *new_rightchild = expVarUnion.expression.right->expVarUnion.expression.left;

    Node::node *newLeftnode = new Node::node ();
    newLeftnode->type = Node::exp_;
    newLeftnode->expVarUnion.expression.left = newLeftchild;
    newLeftnode->expVarUnion.expression.right = new_rightchild;
    newLeftnode->expVarUnion.expression.op = expVarUnion.expression.op;

    Node::node *new_rightnode = expVarUnion.expression.right->expVarUnion.expression.right;

    expVarUnion.expression.left = newLeftnode;
    expVarUnion.expression.right = new_rightnode;
    expVarUnion.expression.op = rightOp;
}

void
Node::node::reorderExpression (int lev)
{
    gapd (lev);
    ifdpm ("called reorder exp \n");
    // check if node is exp, and check if there's an exp in atleast one child
    if (type != Node::exp_)
        {
            gapd (lev);
            ifdpm ("it is not exp, returning \n");
            return;
        }

    auto left = expVarUnion.expression.left;
    auto right = expVarUnion.expression.right;

    bool leftIsExp = left && (left->type == Node::exp_);
    bool rightIsExp = right && (right->type == Node::exp_);

    if (!leftIsExp && !rightIsExp)
        {
            gapd (lev);
            ifdpm ("no child is exp, so returning \n");
            return;
        }

    if (!leftIsExp && rightIsExp)
        {
            // left op, right op  a*(b+c)
            if (shouldWeEvalLeft (expVarUnion.expression.op, expVarUnion.expression.right->expVarUnion.expression.op))
                {
                    gapd (lev);
                    ifdpm ("Node Shift required \n");
                    nodeShiftChildrenLeft ();
                    gapd (lev);
                    ifdpm ("Node after shift: \n");
                    printNode (gaper + lev, 1);
                    if (expVarUnion.expression.left)
                        {
                            gapd (lev);
                            ifdpm ("left child also calling reorder exp \n");
                            expVarUnion.expression.left->reorderExpression (lev + 1);
                        }
                    if (expVarUnion.expression.right)
                        {
                            gapd (lev);
                            ifdpm ("right child also calling reorder exp \n");
                            expVarUnion.expression.right->reorderExpression (lev + 1);
                        }
                }
        }
}

size_t Node::node::varSize()// for type var_
{
    if(type == Node::var_)
        return expVarUnion.variable.type->dtSize();
    return 0;
}   
size_t Node::node::varListSize() // for type varlist_
{
    if(type == Node::varlist_ )
    {
        size_t totSize = 0 ; 
        int ptr = 0 ;
        auto varNode = expVarUnion.VariableList[0][ptr++];
        while(varNode)
        {
            totSize += varNode->varSize();
            varNode = expVarUnion.VariableList[0][ptr++];
        }
        return totSize;
    }
    return 0;
}