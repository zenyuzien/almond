
#include "lexer.h"
#include "token.h"
#include <stdlib.h>
#include <iostream>
#include <cstring>
std::string Token::token::type_and_val_to_str()
{
    switch(type)
    {
        case static_cast<int>(Token::type::Num):
            return "Number "+ std::to_string(ullVal) ;
        case static_cast<int>(Token::type::Str):
            return "String " + std::string(stringVal) ;
        case static_cast<int>(Token::type::OP):
            return "Operator "+ std::string(stringVal);
        case static_cast<int>(Token::type::Sym):
            return "Symbol " + charVal;
        case static_cast<int>(Token::type::ID):
            return "Identifier "+ std::string(stringVal);
        case static_cast<int>(Token::type::KW):
            return "Keyword "+ std::string(stringVal);
        case static_cast<int>(Token::type::Newl):
            return "New Line";
        case static_cast<int>(Token::type::C):
            return "Comment "+ std::string(stringVal);
        default:
            return "Unknown";
    }
}
void Token::token::print()
{
    std::cout << type_and_val_to_str() << std::endl;
}
char lexer::nextCharInSourceFile()
{
    compiler->colNo++;
    char c = getc( compiler->ifile );
    if( expressions>0 )
        parenContent += c;
    
    if (c == '\n')
    {
        compiler->lineNo++;
        compiler->colNo++;
    } 
    return c; 
}
char lexer::peekCharInSourceFile() // peeks next char
{
    char c = getc(compiler->ifile);
    ungetc(c,compiler->ifile);
    return c;
}
void lexer::pushCharToSourceFile(char c)
{
    ungetc(c,compiler->ifile);
}

struct Token::token* lexer::getNextToken()
{
    auto tok = new Token::token();
    if(expressions>0)
        tok->bracketGroup = strdup(parenContent.c_str()) ;
    std::string tmp= "";
    char c = peekCharInSourceFile(),
    end_delimeter; 

    // 1b1 ? 
    switch( c )
    {
        case '/':
        {
            nextCharInSourceFile();
            c = peekCharInSourceFile();
            if(c == '/') // single line comm 
            {
                nextCharInSourceFile();
                c = peekCharInSourceFile();
                while(c != EOF && c!= '\n')
                {
                    tmp+=c;
                    nextCharInSourceFile();
                    c = peekCharInSourceFile();
                }
                tok->type = static_cast<int>(Token::type::C) ;
                tok->stringVal = strdup(tmp.c_str());
                return tok;
            }
            else if( c == '*') /* multiline */
            {
                nextCharInSourceFile();
                c = peekCharInSourceFile();
                while(c!= EOF )
                {
                    if( c == '*')
                    {
                        nextCharInSourceFile();
                        if(peekCharInSourceFile()=='/')
                        {
                            nextCharInSourceFile();
                            break;
                        }
                        pushCharToSourceFile('*');
                    } 
                    
                    tmp+=c ; 
                    nextCharInSourceFile();
                    c = peekCharInSourceFile();
                }
                tok->type = static_cast<int>(Token::type::C);
                tok->stringVal = strdup(tmp.c_str());
                return tok;
            }
            pushCharToSourceFile('/');
            goto op_case;
            break;
        }

        NUMBER_CASE:
        {
            while( c >= '0' && c <= '9' )
            {
                tmp += c;
                nextCharInSourceFile();
                c = peekCharInSourceFile();
            }
            unsigned long long val = std::stoull(tmp);
            c = peekCharInSourceFile();
            int type = static_cast<int>(Token::numType::DEFAULT);
            if(c == 'L')
                type= static_cast<int>(Token::numType::LONG);
            else if(c== 'f')
                type = static_cast<int>(Token::numType::FLOAT);
            if(type) // not default as default is zero
                nextCharInSourceFile();
            tok->numType= type;
            tok->type = static_cast<int>(Token::type::Num);
            tok->ullVal = val; 
            break;
        }

        case 'x': // hexa 
        {
            Token::token* t ;
            if((*vecTokens).size())
                t = (*vecTokens)[(*vecTokens).size()-1];
            if( t->type != static_cast<int>(Token::type::Num) && t->ullVal != 0 )
            {
                goto id_kw ; 
            }
            (*vecTokens).pop_back(); // remove 0 
            nextCharInSourceFile();
            c = peekCharInSourceFile();
            while( isdigit(c) || (c >= 'a' && c <='f') || (c <= 'F' && c>='A') )
            {
                tmp+=c;
                nextCharInSourceFile();
                c = peekCharInSourceFile();
            }
            unsigned long long num = std::stoull(tmp, nullptr, 16);
            tok->type = static_cast<int>(Token::type::Num); 
            tok->ullVal = num;
            break;
        }
        case 'b': // bin 
        {
            Token::token* t ;
            if((*vecTokens).size())
                t = (*vecTokens)[(*vecTokens).size()-1];
            if( t->type != static_cast<int>(Token::type::Num) && t->ullVal != 0 )
            {
                goto id_kw ; 
            }
            (*vecTokens).pop_back(); // remove 0 
            nextCharInSourceFile();
            c = peekCharInSourceFile();
            //std::cout<<"-> "<<c;
            while( c == '1' || c =='0' )
            {
                tmp+=c;
                nextCharInSourceFile();
                c = peekCharInSourceFile();
            }
           // std::cout << "Binary string to convert: \"" << tmp << "\"" << std::endl;

            unsigned long long num = std::stoull(tmp, nullptr, 2);
            tok->type = static_cast<int>(Token::type::Num); 
            tok->ullVal = num;
            break;
        }
        case '\'':
        {
            nextCharInSourceFile();
            // now peekCharInSourceFile is the char next to ' 
            if( peekCharInSourceFile() == '\'')
            {
                compiler->genError("empty quote error \n");
            }
            c = peekCharInSourceFile();
            if(c == '\\')
            {
                // escape char
                c = nextCharInSourceFile();
                if(c == 'n')
                {
                    c = '\n';
                }
                else if( c== '\\') 
                {
                    c = '\\';
                }
                else if(c == 't')
                {
                    c = '\t';
                }
                else if(c == '\'')
                {
                    c = '\'';
                }
            }
            tok->type = static_cast<int>(Token::type::Sym) ; // TODO quote? 
            tok->charVal = c ; 
            nextCharInSourceFile();
            c = nextCharInSourceFile();
            if( c != '\'')
            {
                compiler->genError("quote multi-char error \n");
            }
            break;
        }
        
        OPERATOR_CASE_WITHOUT_DIVISION:
        {
            if(c == '<')
            {
                ; // #include <string.h> case
                auto x = (*vecTokens)[vecTokens->size()-1];
                if(x->type == static_cast<int>(Token::type::KW) && (x->stringVal == "include"))
                {
                    end_delimeter = '>';
                    goto string_case ;
                }

            }
            op_case :;
            tok->type = static_cast<int>(Token::type::OP); 
            c = nextCharInSourceFile(); // c is first operator
            tmp += c ; 
            //std::cout<<"entered operator case \n";
            if(!( c == '(' || c == '[' || c == ',' || c == '.' || c == '*' || c == '?' ))
            {
                //std::cout<<"checking 2nd op \n";
                c = peekCharInSourceFile(); //SECOND operator
                if( c == '(' || c == '[' || c == ',' || c == '.' || c == '*' || c == '?' )
                {
                    goto jump1;
                }
                switch(c)
                {
                    OPERATOR_CASE_WITHOUT_DIVISION:
                    case '/':
                    {
                        // INSIDE
                        c = nextCharInSourceFile(); // still second
                        tmp+=c;
                        //std::cout<<tmp<<" we got 2nd op \n";
                        if(
                            ( tmp == ".."                 && ('.' == peekCharInSourceFile())) ||
                            ( tmp == "<<" || tmp == ">>") && ('=' == peekCharInSourceFile())
                        )
                            tmp += nextCharInSourceFile(); // third
                        else if (!((tmp == "++") || (tmp == "--") ||
                                (tmp == "->") || (tmp == "==") ||
                                (tmp == "!=") || (tmp == ">=") ||
                                (tmp == "<=") || (tmp == "&&") ||
                                (tmp == "||") || (tmp == "+=") ||
                                (tmp == "-=") || (tmp == "*=") ||
                                (tmp == "/=") || (tmp == "%=") ||
                                (tmp == "<<") || (tmp == ">>") ||
                                (tmp == "&=") || (tmp == "|=") ||
                                (tmp == "^=") || (tmp == "->")
                        ))
                            compiler->genError("operator %s is not valid! \n",strdup(tmp.c_str()));
                        
                        break;
                    }
                }
            }
            jump1:;

            tok->stringVal = strdup(tmp.c_str()) ;
            //std::cout<< "tok: "<< tok->stringVal<< std::endl;
            if(c == '(')
            {
                if(!expressions++)
                    parenContent = "";
            } 

            break;
        }
        SYMBOL_CASE:
        // opening bracket is op, closing is sym ? 
            nextCharInSourceFile();
            if( c == ')')
            {
                if(--expressions <0)
                    compiler->genError("extra close bracket error \n");
                
            }
            tok->type = static_cast<int>(Token::type::Sym);
            tok->charVal =  c; 
            break;

        case '\n':
            nextCharInSourceFile();
            tok->type= static_cast<int>(Token::type::Newl);
            break;

        case '"':
        {
            end_delimeter = '"';
            string_case:;
            nextCharInSourceFile();
            c = nextCharInSourceFile();
            //std::cout<<"the next char is "<<c<<std::endl;
            while(c!= end_delimeter && c!=EOF )
            {
                if(c == '\\')
                {
                    // handle escape sequence
                    continue;
                }
                tmp+=c;
                c= nextCharInSourceFile();
            }
            tok->type = static_cast<int>(Token::type::Str);
            tok->stringVal = strdup(tmp.c_str());
            break;
        }
        case ' ':
        case '\t':
        {
            // TODO this IF condition, the spaceNext overall doesnt seem to be useful
            if (!vecTokens->empty())
            {
                Token::token* last = (*vecTokens)[vecTokens->size() - 1];
                last->spaceNext = true;
            }
            nextCharInSourceFile();  // Consume the whitespace
            delete tok;   // Avoid leaking unused token
            return getNextToken();  // Continue lexing
        }
        case EOF:  
            delete tok;
            return nullptr;
            break;
        
        default: 
            id_kw:;
            // can't have case for all alphabet so default 
            if( isalpha(c) || c =='_' )
            {
                while( isalnum(c) || c =='_' )
                {
                    tmp+= c; 
                    nextCharInSourceFile();
                    c = peekCharInSourceFile();
                }
                if (tmp == "unsigned" || tmp == "signed" || tmp == "char" ||
                    tmp == "short" || tmp == "int" || tmp == "long" ||
                    tmp == "float" || tmp == "double" || tmp == "void" ||
                    tmp == "struct" || tmp == "union" || tmp == "static" ||
                    tmp == "__ignore_typecheck" || tmp == "return" || tmp == "include" ||
                    tmp == "sizeof" || tmp == "if" || tmp == "else" ||
                    tmp == "while" || tmp == "for" || tmp == "do" ||
                    tmp == "break" || tmp == "continue" || tmp == "switch" ||
                    tmp == "case" || tmp == "default" || tmp == "goto" ||
                    tmp == "typedef" || tmp == "const" || tmp == "extern" ||
                    tmp == "restrict"
                )
                    tok->type = static_cast<int>(Token::type::KW) ;
                else 
                    tok->type = static_cast<int>(Token::type::ID) ; 
                tok->stringVal = strdup(tmp.c_str());
                break;
            }

            compiler->genError("Unexpected token \n");
    }
    return tok;
}

int lexer::lex()
{
    expressions=0;
    parenContent = ""; 
    // globalise the isntance ? TODO 
    fileName = compiler->path;
    struct Token::token *token = getNextToken();
    while(token)
    {
        (*vecTokens).push_back(token);
        token = getNextToken();
    }

    compiler->vecTokens = vecTokens;
    return 1;
}


