#ifndef debug_
#define debug_

const bool debug_lex = false ;
const bool debug_parse = false ;

#define ifdl if(debug_lex)
#define ifdl_m(msg) if(debug_lex) std::cout<<msg; 
#define ifdp1

#endif
