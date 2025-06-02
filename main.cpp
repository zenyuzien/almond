#include "compiler.h"
#include <iostream>
#include <vector>
#include <cstring>

bool debugParse = false;
bool customParse = false;
int main(int argc, char *argv[]) {

    debugParse= false;
    // Check command line arguments
    for (int i = 0; i < argc ; i++) {
    
        if (strcmp(argv[i], "-dp") == 0 ) {
            debugParse = true;
            customParse = true;
            break;
        }        
        if (strcmp(argv[i], "-cp") == 0 ) {
            debugParse = true;
            customParse = true;
            break;
        }

    }

    auto c = new compilation(); 
    int res = c->compileFile("test.c","test",0);
    if(res == 1)
    {
        std::cout<< "Compiled succesfully! \n" ; 
    }
    else if( res == 0 )
    {
        std::cout << "Comp failed ! \n";
    }
    else 
    {
        std::cout<<res <<" idk what happened \n";
    }
    return 0;
}