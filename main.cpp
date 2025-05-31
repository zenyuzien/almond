#include "compiler.h"
#include <iostream>
#include <vector>
#include <cstring>

bool debug_parse = false;
bool custom_parse = false;
int main(int argc, char *argv[]) {

    debug_parse= false;
    // Check command line arguments
    for (int i = 0; i < argc ; i++) {
    
        if (strcmp(argv[i], "-dp") == 0 ) {
            debug_parse = true;
            //custom_parse = true;
            break;
        }        
        if (strcmp(argv[i], "-cp") == 0 ) {
            //debug_parse = true;
            custom_parse = true;
            break;
        }

    }

    auto c = new compilation(); 
    int res = c->compile_file("test.c","test",0);
    if(res == COMPILATION_SUCCESS)
    {
        std::cout<< "Compiled succesfully! \n" ; 
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