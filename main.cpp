#include "compiler.h"
#include <iostream>
#include <vector>

int main()
{
    auto c = new compilation(); 
    int res = c->compile_file("test.c","test",0);
    if(res == COMPILATION_SUCCESS)
    {
        std::cout<< "All ok ! \n" ; 
    }
    else if( res == COMPILATION_FAILED )
    {
        std::cout << "Comp failed ! \n";
    }
    else 
    {
        std::cout<<res <<" idk what happened \n";
    }
    return 0;
}