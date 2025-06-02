#ifndef _token
#define _token

#include <cstdint>
#include <cstdlib>
#include <string>
/*
*
*
*/
namespace Token
{
    enum class type // token classes
    {
        ID, // identifier
        KW, // keyword
        OP,  // operator 
        Sym,  // symbol 
        Num, // number
        Str, // string
        C,   // comment 
        Newl   // new line
    };  
    enum class numType
    {
        DEFAULT ,
        LONG, 
        FLOAT,
        DOUBLE
    };
    struct token
    {
        int type, flags, numType, rowNo, colNo ;
        union 
        {
            char charVal; 
            char* stringVal;
            uint32_t intVal ; 
            unsigned long longVal; 
            unsigned long long ullVal ;
            void* any; 
        };
        bool spaceNext;
        const char* bracketGroup ;
        token()
            : type(0), numType(0), flags(0), rowNo(0), colNo(0),
            ullVal(0), 
            spaceNext(false), bracketGroup(nullptr) {
            }

        ~token() {
            // If using string_val, delete if allocated
            // Assumes string_val was allocated with new or strdup
            if (type == static_cast<int>(Token::type::Str) && stringVal) {
                free(stringVal);  // or delete[] string_val if you used new[]
                stringVal = nullptr;
            }
        }
        std::string type_and_val_to_str();
        void print();
    };
};


#endif