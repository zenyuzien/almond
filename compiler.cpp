#include "compiler.h"

#include <stdlib.h>

int compilation::compile_file
(
        const char* input_file, 
        const char* output_file,
        int flags
)
{
    struct compilation* process = (struct compilation*) calloc(1,sizeof(struct compilation));
    process->ifile = fopen(input_file,"r");
    process->ofile = fopen(output_file,"w");
    process->flags = flags;  

    if(!process->ofile)
        return COMPILATION_FAILED;


    // lexical
    // parsing 
    // code gen

    return COMPILATION_SUCCESS;
}