#ifndef LIST_HPP_INCLUDED
#define LIST_HPP_INCLUDED









#include <sys\stat.h>
#include <locale.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "../lib/types/Return_code.hpp"
#include "../lib/types/Element.hpp"
#include "../lib/types/Stack.hpp"


//-------------------- SETTINGS --------------------
#define ON_LIST_ERROR_DUMPING
#define ON_LIST_AFTER_OPERATION_DUMPIN

#define  log_file_name "logs.txt"
#define dump_file_name "dump.txt"

const double list_resize_coefficient = 1.2;
//--------------------------------------------------


#define LOG_ERROR(code) _log_error (code, __FILE__, __PRETTY_FUNCTION__, __LINE__)

#define  LIST_CTOR(x)  _list_ctor (x, #x + (#x[0] == '&'), __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define FLIST_DUMP(x) _flist_dump (x, dump_file_name,      __FILE__, __PRETTY_FUNCTION__, __LINE__)

#define LIST_POP_RESIZE(x)   list_resize (x, (int) fmin ( ceil ( (double) x->capacity / list_resize_coefficient), x->capacity - 1) )
#define LIST_PUSH_RESIZE(x)  list_resize (x, (int) fmax ( ceil ( (double) x->capacity * list_resize_coefficient), x->capacity + 1) )

#ifdef ON_LIST_ERROR_DUMPING

    #define ASSERT_LIST_OK(x) if (list_damaged (x)) { FLIST_DUMP (x); LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    #define ASSERT_LIST_OK_FOR_LIST_POP(x)\
            if (list_damaged (x)) { FLIST_DUMP (x); LOG_ERROR (BAD_ARGS); if (return_code_ptr) { *return_code_ptr = BAD_ARGS; } return Element {NAN, true}; }

    #define LIST_ERROR_DUMP(x) FLIST_DUMP(x)

#else

    #define ASSERT_LIST_OK(x)\
    if (list_damaged (x)) {                 LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    #define ASSERT_LIST_OK_FOR_LIST_POP(x)\
            if (list_damaged (x)) {                  LOG_ERROR (BAD_ARGS); if (return_code_ptr) { *return_code_ptr = BAD_ARGS; } return Element {NAN, true}; }

    #define LIST_ERROR_DUMP(x)

#endif


#ifdef ON_LIST_AFTER_OPERATION_DUMPING
    #define LIST_AFTER_OPERATION_DUMPING(x) FLIST_DUMP (x)
#else
    #define LIST_AFTER_OPERATION_DUMPING(x)
#endif


typedef int List_state;
enum        List_state_flags  {

    LS_OK                            = 0, //LS - short for List_state
    LS_NULLPTR                       = 1,
    LS_SIZE_GREATER_THAN_CAPACITY    = 2,
    LS_NULLPTR_NODES                 = 4,
    LS_INCORRECT_POISON_DISTRIBUTION = 8,
};



typedef struct Node_structure Node;
struct         Node_structure  {

    Element element;
    int     prev;
    int     next;
};



typedef struct List_structure      List;

typedef struct List_info_structure List_info;
struct         List_info_structure  {

    const  char* name;
    List*        adress;
    const  char* birth_file;
    const  char* birth_func;
    int          birth_line;
};

struct  List_structure  {

    int size;
    int capacity;
    bool   is_linearized;

    Node* eternal;

    int top_free_ind;


    List_info debug_info;
};



const size_t NODE_SIZE    = sizeof (Node);
const size_t LIST_SIZE    = sizeof (List);



Return_code _list_ctor          (List* list, const char* name, const char* file, const char* func, int line);
Return_code  list_dtor          (List* list);
Return_code _list_resize        (List* list, int new_capacity);
Return_code _list_canary_resize (List* list, int new_capacity);
Return_code  list_push          (List* list, Element_value new_element_value);
Element      list_pop           (List* list, Return_code* return_code_ptr = nullptr);
Return_code  list_linearize     (List* list);

List_state  list_damaged (List* list);
void       _flist_dump   (List* list, const char* file_name, const char* file, const char* function, int line);

Return_code _list_fill_with_poison (List* list, int from,  int to);
Return_code _list_free_stack_fill  (List* list, int first, int last);
Return_code _list_free_stack_push  (List* list, int freed);










#endif