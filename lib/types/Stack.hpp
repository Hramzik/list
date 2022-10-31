#ifndef STACK_TYPE_HPP_INCLUDED
#define STACK_TYPE_HPP_INCLUDED









#include "Element.hpp"
//-------------------- SETTINGS --------------------
#define ON_STACK_ERROR_DUMPING
#define ON_STACK_AFTER_OPERATION_DUMPIN
#define ON_CANARY_PROTECTION
#define ON_HASH_PROTECTION

#define  log_file_name "logs.txt"
#define dump_file_name "dump.txt"

const double stack_resize_coefficient = 2;
//--------------------------------------------------


#ifdef ON_CANARY_PROTECTION
    #define IF_CANARY_PROTECTED(x) x;
#else
    #define IF_CANARY_PROTECTED(x)  ;
#endif


#ifdef ON_HASH_PROTECTION
    #define IF_HASH_PROTECTED(x) x;
#else
    #define IF_HASH_PROTECTED(x)  ;
#endif


typedef struct Stack_structure      Stack;
typedef struct Stack_info_structure Stack_info;
IF_CANARY_PROTECTED (typedef unsigned long long canary_t);
IF_HASH_PROTECTED   (typedef unsigned long long   hash_t);


struct  Stack_info_structure  {

    const  char*  name;
    Stack*        adress;
    const  char*  birth_file;
    const  char*  birth_func;
    int           birth_line;
};

struct  Stack_structure  {

    IF_CANARY_PROTECTED (canary_t FIRST_CANARY);


    Element* elements;
    size_t   size;
    size_t   capacity;

    Stack_info debug_info;

    IF_HASH_PROTECTED (hash_t hash;);


    IF_CANARY_PROTECTED (canary_t SECOND_CANARY);
};


const size_t   STACK_SIZE = sizeof (Stack);









#endif