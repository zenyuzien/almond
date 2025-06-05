
#include "lexer.h"
#include "token.h"
#include <cstring>
#include <iostream>
#include <stdlib.h>
// check header file for the function overviews in this file
/*

BLUEPRINT OF LEXER:

check chars one by one
switch the char to multiple cases

    case /
        can be division or single or multi line comment
        if not comments, JUMP to operator case
    case number
        fill the number
        check type (letter after number)
    case x/b
        check 0 in before token
        fill number as bin/hex
    case ' (quote)
        if next \ : escape sequence
        if not \, just store as symbol
        check closing '
    case operator without division
        if < check for include and go to string case if applicable
        if one of ([,.*? , then single token
        else they can include more symbols as operator so check
    case symbol
        if ) check if allowed
    case \n
        straightforward token
    case "
        check for delimeter and continue checking, can include escape char
    case \t or space
        set prev token as next_space = true
    default case
        check isalpha/underscore as first char
            check isalnum and underscore from 2nd

    the goal is to fill the std::vector<Token::token*> *vecTokens
    and return 0/1 based on verdict of success

*/

std::string
Token::token::type_and_val_to_str ()
{
    switch (type)
        {
        // casting is done to convert enum class into type
        case static_cast<int> (Token::type::Num):
            return "Number " + std::to_string (ullVal);
        case static_cast<int> (Token::type::Str):
            return "String " + std::string (stringVal);
        case static_cast<int> (Token::type::OP):
            return "Operator " + std::string (stringVal);
        case static_cast<int> (Token::type::Sym):
            return std::string ("Symbol ") + charVal;
        case static_cast<int> (Token::type::ID):
            return "Identifier " + std::string (stringVal);
        case static_cast<int> (Token::type::KW):
            return "Keyword " + std::string (stringVal);
        case static_cast<int> (Token::type::Newl):
            return "New Line";
        case static_cast<int> (Token::type::C):
            return "Comment " + std::string (stringVal);
        default:
            return "Unknown";
        }
}
void
Token::token::print ()
{
    // calling the utility function defined above
    std::cout << type_and_val_to_str () << std::endl;
}
void
Token::token::print (std::ofstream &wr)
{
    wr << type_and_val_to_str () << std::endl;
}

char
lexer::nextCharInSourceFile ()
{
    compiler->colNo++;
    char c = getc (compiler->ifile);
    if (expressions > 0)
        parenContent += c;

    if (c == '\n')
        {
            compiler->lineNo++;
            compiler->colNo++;
        }
    return c;
}
char
lexer::peekCharInSourceFile () // peeks next char
{
    char c = getc (compiler->ifile);
    ungetc (c, compiler->ifile);
    return c;
}
void
lexer::pushCharToSourceFile (char c)
{
    ungetc (c, compiler->ifile);
}

struct Token::token *
lexer::getNextToken ()
{
    ifdlm ("Getting next Token \n");

    // declaraing and constructing a new empty token to fill based on reading char stream of source file
    auto tok = new Token::token ();

    // debugging purpose
    if (expressions > 0)
        tok->bracketGroup = strdup (parenContent.c_str ());

    // the tmp will store the string value, using std::string for ease in concatenation
    std::string tmp = "";
    char c = peekCharInSourceFile (), end_delimeter;

    // 1b1 TODO, the syntax should not be allowed, it compiles.

    ifdl if (c != EOF) lexerDebugger << "Switch --> " << c << std::endl;
    else lexerDebugger << "EOF case \n";
    // the switch takes a char and decides what it can be based on conditions
    switch (c)
        {
        // a stream when starting with / can mean a comment, or division
        case '/':
            {
                // checking the next char followed immedietly by /
                nextCharInSourceFile ();
                c = peekCharInSourceFile ();
                if (c == '/') // single line comment if 2 /'s consecutive
                    {
                        ifdlm ("second / : single comment begin \n");
                        // so in this case, read entire line unless its EOF
                        nextCharInSourceFile ();
                        c = peekCharInSourceFile ();
                        while (c != EOF && c != '\n')
                            {
                                tmp += c;
                                nextCharInSourceFile ();
                                c = peekCharInSourceFile ();
                            }
                        // this type is obviously a comment, and the comment data will be stored in heap, returns pointer to token
                        tok->type = static_cast<int> (Token::type::C);
                        tok->stringVal = strdup (tmp.c_str ());
                        return tok;
                    }
                else if (c == '*') /* multiline */
                    {
                        ifdlm ("/* : multi comment begin \n");
                        // now * followed by a / means the begin of multi line comment,
                        // we read until EOF or */ but not erroring if not found */
                        nextCharInSourceFile ();
                        c = peekCharInSourceFile ();
                        while (c != EOF)
                            {
                                if (c == '*')
                                    {
                                        nextCharInSourceFile ();
                                        if (peekCharInSourceFile () == '/')
                                            {
                                                // now that the chars for comments over, next chat to proceed to swith()
                                                ifdlm ("multi Line end \n");
                                                nextCharInSourceFile ();
                                                break;
                                            }
                                        // we went to next char, but since it was false alarm, we push it back
                                        pushCharToSourceFile ('*');
                                    }

                                tmp += c;
                                nextCharInSourceFile ();
                                c = peekCharInSourceFile ();
                            }
                        tok->type = static_cast<int> (Token::type::C);
                        tok->stringVal = strdup (tmp.c_str ());
                        return tok;
                    }
                // we tried looking at char next to / for comments but we need to push it back for operator processing because we can confirm it to be division
                pushCharToSourceFile ('/');
                ifdlm ("It is not the comments, so it is division \n");
                goto op_case;
                break;
            }

        NUMBER_CASE:
            {
                // if a char is number, we read till we get a non number.
                while (c >= '0' && c <= '9')
                    {
                        tmp += c;
                        nextCharInSourceFile ();
                        c = peekCharInSourceFile ();
                    }
                unsigned long long val = std::stoull (tmp);
                ifdl lexerDebugger << "Got Number " << val << " with numTYpe: ";
                // after number, we may encounter a numtype as like eg. a = 12f indicateing float
                c = peekCharInSourceFile ();
                int type = static_cast<int> (Token::numType::DEFAULT);
                if (c == 'L')
                    type = static_cast<int> (Token::numType::LONG);
                else if (c == 'f')
                    type = static_cast<int> (Token::numType::FLOAT);
                if (type) // not default as default is zero
                    nextCharInSourceFile ();
                ifdl lexerDebugger << type << std::endl;
                tok->numType = type;
                tok->type = static_cast<int> (Token::type::Num);
                tok->ullVal = val;
                break;
            }

        case 'x': // hexa
            {
                Token::token *t;
                if ((*vecTokens).size ())
                    t = (*vecTokens)[(*vecTokens).size () - 1];
                if (t->type != static_cast<int> (Token::type::Num) && t->ullVal != 0)
                    {
                        ifdlm ("ID/kw starting with x (ruled out hexadecimal number) \n");
                        goto id_kw;
                    }
                ifdlm ("Hexa number to be expected as 0 preceeding x \n");
                (*vecTokens).pop_back (); // remove 0
                nextCharInSourceFile ();
                c = peekCharInSourceFile ();
                while (isdigit (c) || (c >= 'a' && c <= 'f') || (c <= 'F' && c >= 'A'))
                    {
                        tmp += c;
                        nextCharInSourceFile ();
                        c = peekCharInSourceFile ();
                    }
                unsigned long long num = std::stoull (tmp, nullptr, 16);
                ifdl lexerDebugger << "Hex: " << num << std::endl;
                tok->type = static_cast<int> (Token::type::Num);
                tok->ullVal = num;
                break;
            }
        case 'b': // bin
            {
                Token::token *t;
                if ((*vecTokens).size ())
                    t = (*vecTokens)[(*vecTokens).size () - 1];
                if (t->type != static_cast<int> (Token::type::Num) && t->ullVal != 0)
                    {
                        ifdlm ("ID/kw starting with x (ruled out binary number) \n");
                        goto id_kw;
                    }
                ifdlm ("bin number to be expected as 0 preceeding x \n");
                (*vecTokens).pop_back (); // remove 0
                nextCharInSourceFile ();
                c = peekCharInSourceFile ();
                // std::cout<<"-> "<<c;
                while (c == '1' || c == '0')
                    {
                        tmp += c;
                        nextCharInSourceFile ();
                        c = peekCharInSourceFile ();
                    }
                // std::cout << "Binary string to convert: \"" << tmp << "\"" << std::endl;

                unsigned long long num = std::stoull (tmp, nullptr, 2);
                ifdl lexerDebugger << "Bin: " << num << std::endl;
                tok->type = static_cast<int> (Token::type::Num);
                tok->ullVal = num;
                break;
            }
        case '\'':
            {
                nextCharInSourceFile ();
                // now peekCharInSourceFile is the char next to '
                if (peekCharInSourceFile () == '\'')
                    {
                        compiler->genError ("empty quote error \n");
                    }
                c = peekCharInSourceFile ();
                if (c == '\\')
                    {
                        ifdlm ("escape char: ");
                        c = nextCharInSourceFile ();
                        if (c == 'n')
                            {
                                c = '\n';
                            }
                        else if (c == '\\')
                            {
                                c = '\\';
                            }
                        else if (c == 't')
                            {
                                c = '\t';
                            }
                        else if (c == '\'')
                            {
                                c = '\'';
                            }
                        ifdl lexerDebugger << c << std::endl;
                    }
                tok->type = static_cast<int> (Token::type::Sym); // TODO quote? 'c' instead of c
                tok->charVal = c;
                nextCharInSourceFile ();
                c = nextCharInSourceFile ();
                if (c != '\'')
                    {
                        compiler->genError ("quote multi-char error \n");
                    }
                break;
            }

        OPERATOR_CASE_WITHOUT_DIVISION:
            {
                if (c == '<')
                    {
                        ; // #include <string.h> case
                        auto x = (*vecTokens)[vecTokens->size () - 1];
                        if (x->type == static_cast<int> (Token::type::KW) && (x->stringVal == "include"))
                            {
                                ifdlm ("< while having include earlier => to expect string, not treating as operator \n");
                                end_delimeter = '>';
                                goto string_case;
                            }
                    }
            op_case:;
                tok->type = static_cast<int> (Token::type::OP);
                c = nextCharInSourceFile (); // c is first operator
                tmp += c;
                if (!(c == '(' || c == '[' || c == ',' || c == '.' || c == '*' || c == '?'))
                    {
                        ifdl lexerDebugger << c << " can expect more chars to the operator \n";
                        c = peekCharInSourceFile (); // SECOND operator
                        if (c == '(' || c == '[' || c == ',' || c == '.' || c == '*' || c == '?')
                            {
                                ifdl lexerDebugger << "But not for " << c << std::endl;
                                goto jump1;
                            }
                        switch (c)
                            {
                            OPERATOR_CASE_WITHOUT_DIVISION:
                            case '/':
                                {
                                    // INSIDE
                                    c = nextCharInSourceFile (); // still second
                                    tmp += c;
                                    // std::cout<<tmp<<" we got 2nd op \n";
                                    if ((tmp == ".." && ('.' == peekCharInSourceFile ())) || (tmp == "<<" || tmp == ">>") && ('=' == peekCharInSourceFile ()))
                                        tmp += nextCharInSourceFile (); // third
                                    else if (!((tmp == "++") || (tmp == "--") || (tmp == "->") || (tmp == "==") || (tmp == "!=") || (tmp == ">=") || (tmp == "<=") || (tmp == "&&") || (tmp == "||") || (tmp == "+=") || (tmp == "-=") || (tmp == "*=") || (tmp == "/=") || (tmp == "%=") || (tmp == "<<") || (tmp == ">>") || (tmp == "&=") || (tmp == "|=") || (tmp == "^=") || (tmp == "->")))
                                        compiler->genError ("operator %s is not valid! \n", strdup (tmp.c_str ()));

                                    break;
                                }
                            }
                    }
            jump1:;

                tok->stringVal = strdup (tmp.c_str ());
                // std::cout<< "tok: "<< tok->stringVal<< std::endl;
                if (c == '(')
                    {
                        if (!expressions++)
                            parenContent = "";
                    }

                break;
            }
        SYMBOL_CASE:
            {
                ifdl lexerDebugger << "In Symbol case with " << c << std::endl;
                // opening bracket is op, closing is sym ?
                nextCharInSourceFile ();
                if (c == ')')
                    {
                        if (--expressions < 0)
                            compiler->genError ("extra close bracket error \n");
                    }
                tok->type = static_cast<int> (Token::type::Sym);
                tok->charVal = c;
                ifdl lexerDebugger << "token symbol: " << c << std::endl;
                break;
            }
        case '\n':
            nextCharInSourceFile ();
            tok->type = static_cast<int> (Token::type::Newl);
            break;

        case '"':
            {
                end_delimeter = '"';
                ifdlm ("will check string with end_delimeter \"\n");
            string_case:;
                nextCharInSourceFile ();
                c = nextCharInSourceFile ();
                // std::cout<<"the next char is "<<c<<std::endl;
                while (c != end_delimeter && c != EOF)
                    {
                        if (c == '\\')
                            {
                                // handle escape sequence
                                continue;
                            }
                        tmp += c;
                        c = nextCharInSourceFile ();
                    }
                tok->type = static_cast<int> (Token::type::Str);
                tok->stringVal = strdup (tmp.c_str ());
                break;
            }
        case ' ':
        case '\t':
            {
                // TODO this IF condition, the spaceNext overall doesnt seem to be useful
                if (!vecTokens->empty ())
                    {
                        Token::token *last = (*vecTokens)[vecTokens->size () - 1];
                        last->spaceNext = true;
                    }
                nextCharInSourceFile (); // Consume the whitespace
                delete tok;              // Avoid leaking unused token
                return getNextToken ();  // Continue lexing
            }
        case EOF:
            delete tok;
            return nullptr;
            break;

        default:
        id_kw:;
            // can't have case for all alphabet so default
            if (isalpha (c) || c == '_')
                {
                    while (isalnum (c) || c == '_')
                        {
                            tmp += c;
                            nextCharInSourceFile ();
                            c = peekCharInSourceFile ();
                        }
                    if (tmp == "unsigned" || tmp == "signed" || tmp == "char" || tmp == "short" || tmp == "int" || tmp == "long" || tmp == "float" || tmp == "double" || tmp == "void" || tmp == "struct" || tmp == "union" || tmp == "static" || tmp == "__ignore_typecheck" || tmp == "return" || tmp == "include" || tmp == "sizeof" || tmp == "if" || tmp == "else" || tmp == "while" || tmp == "for" || tmp == "do" || tmp == "break" || tmp == "continue" || tmp == "switch" || tmp == "case"
                        || tmp == "default" || tmp == "goto" || tmp == "typedef" || tmp == "const" || tmp == "extern" || tmp == "restrict")
                        tok->type = static_cast<int> (Token::type::KW);
                    else
                        tok->type = static_cast<int> (Token::type::ID);
                    tok->stringVal = strdup (tmp.c_str ());
                    break;
                }

            compiler->genError ("Unexpected token \n");
        }
    ifdl lexerDebugger << "\tToken to be returned : \n";
    ifdl tok->print (lexerDebugger);
    return tok;
}

int
lexer::lex ()
{
    ifdlm ("***LEXER STARTED***\n");
    expressions = 0;
    parenContent = "";
    // globalise the isntance ? TODO
    fileName = compiler->path;
    struct Token::token *token = getNextToken ();
    while (token)
        {
            (*vecTokens).push_back (token);
            token = getNextToken ();
        }

    compiler->vecTokens = vecTokens;
    ifdlm ("***LEXER ENDED***\n");
    return 1;
}
