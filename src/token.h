#ifndef _token
#define _token

#include <cstdint>
#include <cstdlib>
#include <string>
/*
 * the namespace Token contains the token structure and 2 enum blocks for token types and number types
 *
 */
namespace Token {
enum class type // token classes
{
    ID,  // identifier
    KW,  // keyword
    OP,  // operator
    Sym, // symbol
    Num, // number
    Str, // string
    C,   // comment
    Newl // new line
};
enum class numType // the number can be specified with a symbol specifying type eg. a = 1255f
{
    DEFAULT,
    LONG,
    FLOAT,
    DOUBLE
};
struct token {
    // 'type' will store the value from enum class type
    // flags will contain useful information that may be used in future.
    // numType as mentioned earlier, will specify the type like float or double when mentioned like eg. mynum = 100f
    int type, flags, numType, rowNo, colNo; // rowNo, colNo together specify position in source input file where the token is, used for errors/ general debugging
    // union is required a value can be any of char, string, number, etc
    // but just to be open-minded, we have void* as well.
    union {
        char charVal;
        char *stringVal;
        uint32_t intVal;
        unsigned long longVal;
        unsigned long long ullVal;
        void *any;
    };
    // this may come handy but not for certain, this flag indicates if there exists a space after this token
    bool spaceNext;

    // purely for debugging helping
    const char *bracketGroup;
    token() : type(0), numType(0), flags(0), rowNo(0), colNo(0), ullVal(0), spaceNext(false), bracketGroup(nullptr) {}

    ~token() {
        // If using string_val, delete if allocated
        // Assumes string_val was allocated with new or strdup
        if (type == static_cast<int>(Token::type::Str) && stringVal) {
            free(stringVal); // or delete[] string_val if you used new[]
            stringVal = nullptr;
        }
    }
    // this function returns a string with type and value in string format, useful for printing
    std::string type_and_val_to_str();

    // prints the token in human readable format, if debug, writes to log file
    void print(std::ofstream &wr);
    void print();
};
}; // namespace Token

#endif