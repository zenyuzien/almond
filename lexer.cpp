
#include "lexer.h"
#include <stdlib.h>
#include <iostream>
#include <cstring>


char lexer::next_char(bool mode)
{
    compiler->col_no++;
    char c = getc( compiler->ifile );
    if( expressions>0 )
        bracket_content += c;
    
    if (c == '\n')
    {
        compiler->line_no++;
        compiler->col_no++;
    } 
    return c; 
}
char lexer::top_char(bool mode ) // peeks next char
{
    char c = getc(compiler->ifile);
    ungetc(c,compiler->ifile);
    return c;
}
void lexer::push_char(char c, bool mode)
{
    ungetc(c,compiler->ifile);
}

struct token* lexer::read_next_token()
{
    auto tok = new token();
    if(expressions>0)
        tok->bracket_grp = strdup(bracket_content.c_str()) ;
    std::string tmp= "";
    char c = top_char(); 
    //std::cout << "case '"<<c<<"'\n";
    char end_delimeter; 

    // 1b1 ? 
    switch( c )
    {
        case '/':
        {
            next_char();
            c = top_char();
            if(c == '/') // single line comm 
            {
                next_char();
                c = top_char();
                while(c != EOF && c!= '\n')
                {
                    tmp+=c;
                    next_char();
                    c = top_char();
                }
                tok->type = TT_C ;
                tok->string_val = strdup(tmp.c_str());
                return tok;
            }
            else if( c == '*') /* multiline */
            {
                next_char();
                c = top_char();
                while(c!= EOF )
                {
                    if( c == '*')
                    {
                        next_char();
                        if(top_char()=='/')
                        {
                            next_char();
                            break;
                        }
                        push_char('*');
                    } 
                    
                    tmp+=c ; 
                    next_char();
                    c = top_char();
                }
                tok->type = TT_C;
                tok->string_val = strdup(tmp.c_str());
                return tok;
            }
            push_char('/');
            goto op_case;
            break;
        }

        NUMBER_CASE:
        {
            while( c >= '0' && c <= '9' )
            {
                tmp += c;
                next_char();
                c = top_char();
            }
            unsigned long long val = std::stoull(tmp);
            c = top_char();
            int type = DEFAULT_NUM;
            if(c == 'L')
                type= LONG_NUM;
            else if(c== 'f')
                type = FLOAT_NUM;
            if(type)
                next_char();
            tok->num_type= type;
            
            //std::cout<< val <<" \n";
            tok->type = TT_Num;
            tok->ull_val = val; 
            break;
        }

        case 'x': // hexa 
        {
            token* t ;
            if((*vec_t).size())
                t = (*vec_t)[(*vec_t).size()-1];
            if( t->type != TT_Num && t->ull_val != 0 )
            {
                goto id_kw ; 
            }
            (*vec_t).pop_back(); // remove 0 
            next_char();
            c = top_char();
            while( isdigit(c) || (c >= 'a' && c <='f') || (c <= 'F' && c>='A') )
            {
                tmp+=c;
                next_char();
                c = top_char();
            }
            unsigned long long num = std::stoull(tmp, nullptr, 16);
            tok->type = TT_Num; 
            tok->ull_val = num;
            break;
        }
        case 'b': // bin 
        {
            token* t ;
            if((*vec_t).size())
                t = (*vec_t)[(*vec_t).size()-1];
            if( t->type != TT_Num && t->ull_val != 0 )
            {
                goto id_kw ; 
            }
            (*vec_t).pop_back(); // remove 0 
            next_char();
            c = top_char();
            //std::cout<<"-> "<<c;
            while( c == '1' || c =='0' )
            {
                tmp+=c;
                next_char();
                c = top_char();
            }
           // std::cout << "Binary string to convert: \"" << tmp << "\"" << std::endl;

            unsigned long long num = std::stoull(tmp, nullptr, 2);
            tok->type = TT_Num; 
            tok->ull_val = num;
            break;
        }
        case '\'':
        {
            next_char();
            // now top_char is the char next to ' 
            if( top_char() == '\'')
            {
                compiler->error_msg("empty quote error \n");
            }
            c = top_char();
            if(c == '\\')
            {
                // escape char
                c = next_char();
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
            tok->type = TT_Sym ; // TODO quote? 
            tok->char_val = c ; 
            next_char();
            c = next_char();
            if( c != '\'')
            {
                compiler->error_msg("quote multi-char error \n");
            }
            break;
        }
        OPERATOR_CASE:
        {
            if(c == '<')
            {
                ; // #include <string.h> case
                auto x = (*vec_t)[vec_t->size()-1];
                if((x->type == TT_KW) && (x->string_val == "include"))
                {
                    end_delimeter = '>';
                    goto string_case ;
                }

            }
            op_case :;
            tok->type = TT_OP; 
            c = next_char(); // c is first operator
            tmp += c ; 
            //std::cout<<"entered operator case \n";
            if(!( c == '(' || c == '[' || c == ',' || c == '.' || c == '*' || c == '?' ))
            {
                //std::cout<<"checking 2nd op \n";
                c = top_char(); //SECOND operator
                if( c == '(' || c == '[' || c == ',' || c == '.' || c == '*' || c == '?' )
                {
                    goto jump1;
                }
                switch(c)
                {
                    OPERATOR_CASE:
                    case '/':
                    {
                        // INSIDE
                        c = next_char(); // still second
                        tmp+=c;
                        //std::cout<<tmp<<" we got 2nd op \n";
                        if(
                            ( tmp == ".."                 && ('.' == top_char())) ||
                            ( tmp == "<<" || tmp == ">>") && ('=' == top_char())
                        )
                            tmp += next_char(); // third
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
                            compiler->error_msg("operator %s is not valid! \n",strdup(tmp.c_str()));
                        
                        break;
                    }
                }
            }
            jump1:;

            tok->string_val = strdup(tmp.c_str()) ;
            //std::cout<< "tok: "<< tok->string_val<< std::endl;
            if(c == '(')
            {
                if(!expressions++)
                    bracket_content = "";
            } 

            break;
        }
        SYMBOL_CASE:
        // opening bracket is op, closing is sym ? 
            next_char();
            if( c == ')')
            {
                if(--expressions <0)
                {
                    compiler->error_msg("extra close bracket error \n");
                }
            }
            tok->type = TT_Sym;
            tok->char_val =  c; 
            break;

        case '\n':
            next_char();
            tok->type= TT_Newl;
            break;

        case '"':
        {
            end_delimeter = '"';
            string_case:;
            next_char();
            c = next_char();
            //std::cout<<"the next char is "<<c<<std::endl;
            while(c!= end_delimeter && c!=EOF )
            {
                if(c == '\\')
                {
                    // handle escape sequence
                    continue;
                }
                tmp+=c;
                c= next_char();
            }
            tok->type = TT_Str;
            tok->string_val = strdup(tmp.c_str());
            break;
        }
        case ' ':
        case '\t':
        {
            if (!vec_t->empty())
            {
                token* last = (*vec_t)[vec_t->size() - 1];
                last->space_next = true;
            }
            next_char();  // Consume the whitespace
            delete tok;   // Avoid leaking unused token
            return read_next_token();  // Continue lexing
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
                    next_char();
                    c = top_char();
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
                    tok->type = TT_KW ;
                else 
                    tok->type = TT_ID ; 
                tok->string_val = strdup(tmp.c_str());
                break;
            }

            compiler->error_msg("Unexpected token \n");
    }
    return tok;
}

int lexer::lex()
{
    expressions=0;
    bracket_content = ""; 
    // globalise the isntance ? TODO 
    filename = compiler->path;
    struct token *token = read_next_token();
    while(token)
    {
        //std::cout<<"walhalla";
        (*vec_t).push_back(token);
        //std::cout<<"size: "<< (*vec_t).size() << std::endl;
        token = read_next_token();
    }

    compiler->vec_t = vec_t;
    return LEXER_SUCCESS;
}


