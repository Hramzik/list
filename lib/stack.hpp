#ifndef STACK_HPP_INCLUDED
#define STACK_HPP_INCLUDED









#include <sys\stat.h>
#include <locale.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "logs.hpp"
#include "types/Stack.hpp"



#define COMMA ,


#define  STACK_CTOR(x)  _stack_ctor (x, #x + (#x[0] == '&'), __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define  STACK_DUMP(x) _fstack_dump (x, nullptr,             __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define FSTACK_DUMP(x) _fstack_dump (x, stack_dump_file_name,      __FILE__, __PRETTY_FUNCTION__, __LINE__)

#define STACK_POP_RESIZE(x)   stack_resize (x, (size_t) fmin ( ceil ( (double) x->capacity / stack_resize_coefficient), x->capacity - 1) )
#define STACK_PUSH_RESIZE(x)  stack_resize (x, (size_t) fmax ( ceil ( (double) x->capacity * stack_resize_coefficient), x->capacity + 1) )

#ifdef ON_STACK_ERROR_DUMPING

    #define ASSERT_STACK_OK(x) if (stack_damaged (x)) { FSTACK_DUMP (x); LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    #define ASSERT_STACK_OK_FOR_STACK_POP(x)\
            if (stack_damaged (x)) { FSTACK_DUMP (x); LOG_ERROR (BAD_ARGS); if (return_code_ptr) { *return_code_ptr = BAD_ARGS; } return Element {NAN, true}; }

    #define STACK_ERROR_DUMP(x) FSTACK_DUMP(x)

#else

    #define ASSERT_STACK_OK(x)\
    if (stack_damaged (x)) {                 LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    #define ASSERT_STACK_OK_FOR_STACK_POP(x)\
            if (stack_damaged (x)) {                  LOG_ERROR (BAD_ARGS); if (return_code_ptr) { *return_code_ptr = BAD_ARGS; } return Element {NAN, true}; }

    #define STACK_ERROR_DUMP(x)

#endif


#ifdef ON_STACK_AFTER_OPERATION_DUMPING
    #define STACK_AFTER_OPERATION_DUMPING(x) FSTACK_DUMP (x)
#else
    #define STACK_AFTER_OPERATION_DUMPING(x)
#endif


#ifdef ON_CANARY_PROTECTION

    #define IF_CANARY_PROTECTED(x) x;
    #define CANARY_SIZE sizeof (canary_t)
    #define FIRST_CANARY_VALUE  0xDEADBEEF
    #define SECOND_CANARY_VALUE 0xDEADBEEF
    #define stack_resize(x,y)         _stack_canary_resize       (x, y)

#else

    #define IF_CANARY_PROTECTED(x)  ;
    #define stack_resize(x,y)     _stack_resize       (x, y)

#endif


#ifdef ON_HASH_PROTECTION
    #define IF_HASH_PROTECTED(x) x;
    #define HASH_SIZE sizeof (hash_t)
    #define HASH_MAX  ( (hash_t) -1)
    #define HASH_SALT ( (hash_t) 0xD1E2A3D4B5E6E7F )
#else
    #define IF_HASH_PROTECTED(x)  ;
#endif



typedef int Stack_state;
enum        Stack_state_flags {

    SS_OK                            = 0, //SS - short for Stack_state
    SS_NULLPTR                       = 1,
    SS_SIZE_GREATER_THAN_CAPACITY    = 2,
    SS_NULLPTR_ELEMENTS              = 4,
    SS_INCORRECT_POISON_DISTRIBUTION = 8,
    SS_STACK_CANARY_CORRUPTED        = 16,
    SS_DATA_CANARY_CORRUPTED         = 32,
    SS_INCORRECT_HASH                = 64,
};



Return_code _stack_ctor          (Stack* stack, const char* name, const char* file, const char* func, int line);
Return_code  stack_dtor          (Stack* stack);
Return_code _stack_resize        (Stack* stack, size_t new_capacity);
Return_code _stack_canary_resize (Stack* stack, size_t new_capacity);
Return_code  stack_push          (Stack* stack, Element_value new_element_value);
Element      stack_pop           (Stack* stack, Return_code* return_code_ptr = nullptr);

Stack_state  stack_damaged       (Stack* stack);
void       _fstack_dump          (Stack* stack, const char* file_name, const char* file, const char* function, int line);

Return_code _stack_fill_with_poison (Stack* stack, size_t from, size_t to);

IF_HASH_PROTECTED (

    hash_t        hash300                      (void* _data_ptr, size_t size);
    Return_code   stack_recount_hash           (Stack* stack);
);



















Return_code  _stack_ctor  (Stack* stack, const char* name, const char* file, const char* func, int line) {

    assert ( (file != nullptr) && (func != nullptr) && (line > 0) );
    if (stack == nullptr || name == nullptr) { LOG_ERROR (BAD_ARGS); STACK_ERROR_DUMP (stack); return BAD_ARGS; }


    stack->elements = nullptr;
    stack->size     = 0;
    stack->capacity = 0;


    stack->debug_info.name = name;
    stack->debug_info.birth_file = file;
    stack->debug_info.birth_func = func;
    stack->debug_info.birth_line = line;
    stack->debug_info.adress     = stack;


    IF_CANARY_PROTECTED (

        stack-> FIRST_CANARY =  FIRST_CANARY_VALUE;
        stack->SECOND_CANARY = SECOND_CANARY_VALUE;

        stack->elements = (Element*) calloc (2 * CANARY_SIZE, 1);
        if (stack->elements == nullptr) { LOG_ERROR (MEMORY_ERR); STACK_ERROR_DUMP (stack); return MEMORY_ERR; }

        ( (canary_t*) stack->elements) [0] =  FIRST_CANARY_VALUE;
        ( (canary_t*) stack->elements) [1] = SECOND_CANARY_VALUE;

        stack->elements = (Element*) ( (char*) stack->elements + CANARY_SIZE );
    );


    IF_HASH_PROTECTED ( stack_recount_hash (stack); );


    STACK_AFTER_OPERATION_DUMPING (stack);


    return SUCCESS;
}


Return_code  stack_dtor  (Stack* stack) {

    ASSERT_STACK_OK (stack);


    if (stack->elements != nullptr) {

        #ifdef ON_CANARY_PROTECTION
            free ( (char*) stack->elements - CANARY_SIZE);
        #else
            free (stack->elements);
        #endif
        stack->elements = nullptr;
    }


    STACK_AFTER_OPERATION_DUMPING (stack);


    return SUCCESS;
}


Return_code  _stack_resize  (Stack* stack, size_t new_capacity) {

    ASSERT_STACK_OK (stack);


    stack->elements = (Element*) realloc (stack->elements, sizeof (Element) * new_capacity);
    if (stack->elements == nullptr && new_capacity != 0) { LOG_ERROR (MEMORY_ERR); STACK_ERROR_DUMP (stack); return MEMORY_ERR; }


    _stack_fill_with_poison (stack, stack->size, new_capacity); //


    stack->capacity = new_capacity;


    if (stack->size > new_capacity) { stack->size = new_capacity; }


    IF_HASH_PROTECTED ( stack_recount_hash (stack); );


    STACK_AFTER_OPERATION_DUMPING (stack);


    return SUCCESS;
}


IF_CANARY_PROTECTED (

    Return_code  _stack_canary_resize  (Stack* stack, size_t new_capacity) {

        ASSERT_STACK_OK (stack); //putc('a', stderr);

        size_t new_size = new_capacity * sizeof (Element) + 2 * CANARY_SIZE;


        if (new_capacity == stack->capacity) { STACK_AFTER_OPERATION_DUMPING (stack); return SUCCESS; }

        if (new_capacity > stack->capacity) {

            stack->elements = (Element*) ( (char*) stack->elements - CANARY_SIZE);


            stack->elements = (Element*) realloc (stack->elements, new_size);
            if (stack->elements == nullptr && new_capacity != 0) { LOG_ERROR (MEMORY_ERR); STACK_ERROR_DUMP (stack); return MEMORY_ERR; }


            stack->elements = (Element*) ( (char*) stack->elements + CANARY_SIZE ) + stack->capacity;
            canary_t second_canary_buffer = *( (canary_t*) stack->elements );


            _stack_fill_with_poison (stack, 0, new_capacity - stack->capacity); //


            *( (canary_t*) (stack->elements + new_capacity - stack->capacity) ) = second_canary_buffer;


            stack->elements -= stack->capacity;
            stack->capacity  = new_capacity;


        IF_HASH_PROTECTED ( stack_recount_hash (stack); );


        STACK_AFTER_OPERATION_DUMPING(stack);


        return SUCCESS;
        }

        stack->elements = (Element*) ( (char*) stack->elements - CANARY_SIZE );


        canary_t second_canary_buffer = *(canary_t*) ( (char*) (stack->elements + stack->capacity) + CANARY_SIZE );


        stack->elements = (Element*) realloc (stack->elements, new_size);
        if (stack->elements == nullptr) { LOG_ERROR (MEMORY_ERR); STACK_ERROR_DUMP (stack); return MEMORY_ERR; }


        *(canary_t*) ( (char*) (stack->elements + new_capacity) + CANARY_SIZE ) = second_canary_buffer;


        stack->elements = (Element*) ( (char*) stack->elements + CANARY_SIZE );
        stack->capacity = new_capacity;


        IF_HASH_PROTECTED ( stack_recount_hash (stack); );


        STACK_AFTER_OPERATION_DUMPING(stack);


        return SUCCESS;
    }
);


Return_code  stack_push  (Stack* stack, Element_value new_element_value) {

    ASSERT_STACK_OK (stack);


    if (stack->size == stack->capacity) {

        Return_code resize_code = SUCCESS;

        if (!stack->capacity) {

            resize_code = stack_resize (stack, 1);
        }
        else {

            resize_code = STACK_PUSH_RESIZE (stack);
        }

        if (resize_code) { LOG_ERROR (resize_code);  STACK_ERROR_DUMP (stack); return resize_code; }
    }
   // STACK_DUMP (stack);

    stack->size += 1;
    stack->elements [stack->size - 1] = Element {new_element_value, false};


    IF_HASH_PROTECTED ( stack_recount_hash (stack); );


    STACK_AFTER_OPERATION_DUMPING (stack);


    return SUCCESS;
}


Element  stack_pop  (Stack* stack, Return_code* return_code_ptr) {

    ASSERT_STACK_OK_FOR_STACK_POP (stack);


    Element return_element = {NAN, true};

    if (stack->size != 0) {

        stack->size -= 1;
        return_element = stack->elements [stack->size];
        stack->elements [stack->size] = Element {NAN, true};

        IF_HASH_PROTECTED ( stack_recount_hash (stack); );
    }


    if ( (double) stack->size * pow (stack_resize_coefficient, 2) <= (double) stack->capacity) {

        Return_code resize_code = STACK_POP_RESIZE (stack);

        if (resize_code) {

            LOG_ERROR (resize_code);
            if (return_code_ptr) { *return_code_ptr = BAD_ARGS; }
            STACK_ERROR_DUMP (stack);
            return Element {NAN, true};
        }
    }


    IF_HASH_PROTECTED ( stack_recount_hash (stack); );


    if (return_code_ptr) { *return_code_ptr = SUCCESS; }


    STACK_AFTER_OPERATION_DUMPING (stack);


    return return_element;
}


Stack_state  stack_damaged  (Stack* stack) {

    Stack_state stack_state = SS_OK;


    if (stack == nullptr) { stack_state |= SS_NULLPTR; return stack_state; }


    if (stack->size > stack->capacity)                  { stack_state |= SS_SIZE_GREATER_THAN_CAPACITY; }
    if (stack->elements == nullptr && stack->size != 0) { stack_state |= SS_NULLPTR_ELEMENTS; }


    for (size_t i = 0; i < stack->capacity; i++) {

        if (i <  stack->size) if ( stack->elements [i].poisoned) { stack_state |= SS_INCORRECT_POISON_DISTRIBUTION; break; }
        if (i >= stack->size) if (!stack->elements [i].poisoned) { stack_state |= SS_INCORRECT_POISON_DISTRIBUTION; break; }
    }

    IF_CANARY_PROTECTED (

        if (stack-> FIRST_CANARY !=  FIRST_CANARY_VALUE) { stack_state |= SS_STACK_CANARY_CORRUPTED;  }
        if (stack->SECOND_CANARY != SECOND_CANARY_VALUE) { stack_state |= SS_STACK_CANARY_CORRUPTED;  }

        if ( *( (canary_t*) stack->elements - 1)                  !=  FIRST_CANARY_VALUE) { stack_state |= SS_DATA_CANARY_CORRUPTED; }
        if ( *( (canary_t*) (stack->elements + stack->capacity) ) != SECOND_CANARY_VALUE) { stack_state |= SS_DATA_CANARY_CORRUPTED; }
    );


    IF_HASH_PROTECTED (

        hash_t old_hash = stack->hash;
        stack_recount_hash (stack);
        if (old_hash != stack->hash) { stack_state |= SS_INCORRECT_HASH; }
    );


    return stack_state;
}


void  _fstack_dump  (Stack* stack, const char* file_name, const char* file, const char* func, int line) {

    assert (file != nullptr && func != nullptr);


    FILE* dump_file;
    if (file_name == nullptr) {

        dump_file = stdout;
    }

    else {

        dump_file = fopen (file_name, "a");
        if (dump_file == nullptr) { LOG_ERROR (FILE_ERR); return; }
    }


    setvbuf (dump_file, NULL, _IONBF, 0);


    fprintf (dump_file, "--------------------\n");
    fprintf (dump_file, "Dumping stack at %s in function %s (line %d)...\n\n", file, func, line);


    if (!stack) { fprintf (dump_file, "Stack pointer is nullptr!\n\n"); return; }


    fprintf (dump_file, "this stack has name ");
    if (stack->debug_info.name != nullptr) { fprintf (dump_file, "%s ", stack->debug_info.name); }
    else                                   { fprintf (dump_file, "UNKNOWN NAME "); }
    fprintf (dump_file, "[%p]\n", stack->debug_info.adress);

    fprintf (dump_file, "it was created in file ");
    if (stack->debug_info.birth_file != nullptr) { fprintf (dump_file, "%s\n", stack->debug_info.birth_file); }
    else                                         { fprintf (dump_file, "UNKNOWN NAME\n"); }

    fprintf (dump_file, "in function ");
    if (stack->debug_info.birth_func != nullptr) { fprintf (dump_file, "%s ", stack->debug_info.birth_func); }
    else                                         { fprintf (dump_file, "UNKNOWN NAME "); }

    fprintf (dump_file, "(line %d)\n\n", stack->debug_info.birth_line);


    IF_HASH_PROTECTED (

        fprintf (dump_file, "stack hash is %llX\n\n", stack->hash);
    );


    fprintf (dump_file, "stack is ");
    Stack_state stack_state = stack_damaged (stack);
    if (stack_state) { fprintf (dump_file, "damaged (damage code %u)\n", stack_state); }
    else             { fprintf (dump_file, "ok\n"); }


    fprintf (dump_file, "size     %zd\n",      stack->size);
    fprintf (dump_file, "capacity %zd\n\n",  stack->capacity);
    if (stack->elements) { fprintf (dump_file, "elements [%p]:\n", stack->elements); }
    else                 { fprintf (dump_file, "elements [nullptr]:\n"); }


    for (size_t i = 0; i < stack->capacity; i++) {

        if (i < stack->size) { fprintf (dump_file, "(in)  "); }
        else                 { fprintf (dump_file, "(out) "); }

        fprintf (dump_file, "[%zd] = %-5lg (", i, stack->elements[i].value);
        if (stack->elements[i].poisoned) { fprintf (dump_file,     "poisoned)\n"); }
        else                             { fprintf (dump_file, "not poisoned)\n"); }
    }
    fprintf (dump_file, "\n");


    IF_CANARY_PROTECTED (
        fprintf (dump_file, "first  stack canary - %llX (", stack->FIRST_CANARY);

        if (stack->FIRST_CANARY == FIRST_CANARY_VALUE) { fprintf (dump_file, "untouched)\n"); }
        else                                           { fprintf (dump_file, "corrupted)\n"); }

        fprintf (dump_file, "second stack canary - %llX (", stack->SECOND_CANARY);

        if (stack->SECOND_CANARY == SECOND_CANARY_VALUE) { fprintf (dump_file, "untouched)\n"); }
        else                                             { fprintf (dump_file, "corrupted)\n"); }


        fprintf (dump_file, "first  data  canary - %llX (", *( (canary_t*) stack->elements - 1));

        if ( *( (canary_t*) stack->elements - 1) == FIRST_CANARY_VALUE) { fprintf (dump_file, "untouched)\n"); }
        else                                           { fprintf (dump_file, "corrupted)\n"); }

        fprintf (dump_file, "second data  canary - %llX (", *(canary_t*)(stack->elements + stack->capacity) );

        if ( *(canary_t*)(stack->elements + stack->capacity) == SECOND_CANARY_VALUE) { fprintf (dump_file, "untouched)\n"); }
        else                                             { fprintf (dump_file, "corrupted)\n"); }
    );

    fprintf (dump_file, "\n");


    fclose (dump_file);
}


IF_HASH_PROTECTED (

    hash_t  hash300  (void* _data_ptr, size_t size) {

        if (!_data_ptr) return 0;


        unsigned char* data_ptr = (unsigned char*) _data_ptr;


        hash_t hash_sum  = HASH_SALT;
        hash_t hash_salt = HASH_SALT; 
        for (size_t i = 0; i < size; i++) {

            hash_sum  ^= hash_sum * (data_ptr[i] * hash_salt);
        }


        return hash_sum;
    }


    Return_code  stack_recount_hash  (Stack* stack) {

        if (!stack) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


        #ifdef ON_CANARY_PROTECTION
            hash_t hash1 = hash300 (stack, STACK_SIZE - HASH_SIZE - CANARY_SIZE);
            hash_t hash2 = hash300 ( (char*) stack->elements - CANARY_SIZE, stack->capacity + 2 * CANARY_SIZE);
            hash_t hash3 = hash300 (&stack->SECOND_CANARY, CANARY_SIZE);
        #else
            hash_t hash1 = hash300 (stack, STACK_SIZE - HASH_SIZE);
            hash_t hash2 = hash300 (stack->elements, stack->capacity * ELEMENT_SIZE);
            hash_t hash3 = 0;
        #endif


        stack->hash = hash1 ^ hash2 ^ hash3;


        return SUCCESS;
    }

);


Return_code _stack_fill_with_poison (Stack* stack, size_t from, size_t to) {

    if (!stack || (to > stack->capacity) ) { LOG_ERROR (BAD_ARGS); STACK_ERROR_DUMP(stack); return BAD_ARGS; }


    for (size_t i = from; i < to; i++) {

        stack->elements[i] = Element {NAN, true};
    }


    STACK_AFTER_OPERATION_DUMPING (stack);


    return SUCCESS;
}









#endif


