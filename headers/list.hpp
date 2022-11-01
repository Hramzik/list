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


//-------------------- SETTINGS --------------------
#define ON_LIST_ERROR_DUMPING
#define ON_LIST_AFTER_OPERATION_DUMPIN

#define list_dump_bgclr       "9aabba"
#define list_dump_nodeclr     "8bbfec"
#define list_dump_freeclr     "58758e"
#define list_dump_arrwclr     "0368c0"
#define list_dump_freearrwclr "000000"

#define list_dump_file_name              "work/dump.txt"
#define list_graph_dump_file_name        "work/graph_dump"
#define list_graph_file_name             "work/graph"
#define list_graph_describtion_file_name "work/graph_describtion"

const double list_resize_coefficient = 1.2;
//--------------------------------------------------

#define TOSTR(x) #x

#define LOG_ERROR(code) _log_error (code, __FILE__, __PRETTY_FUNCTION__, __LINE__)

#define  LIST_CTOR(x)       _list_ctor      (x, #x + (#x[0] == '&'),       __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define FLIST_DUMP(x)      _flist_dump      (x, list_dump_file_name,       __FILE__, __PRETTY_FUNCTION__, __LINE__)


#define LIST_GENERATE_GRAPH_DESCRIPTION(x, line) list_generate_graph_describtion (x, #line)
#define LIST_GENERATE_GRAPH(x, line)             list_generate_graph             (#line)
#define _FLIST_GRAPHDUMP(x, line, ...)         _flist_graphdump                 (x, list_graph_dump_file_name, __FILE__, __PRETTY_FUNCTION__, line, #line, __VA_ARGS__)

#define FLIST_GRAPHDUMP(x, ...)\
    LIST_GENERATE_GRAPH_DESCRIPTION (x, __LINE__);\
    LIST_GENERATE_GRAPH             (x, __LINE__);\
    _FLIST_GRAPHDUMP                (x, __LINE__, __VA_ARGS__)

#define LIST_POP_RESIZE(x)   list_resize (x, (int) fmin ( ceil ( (double) (x->capacity - 1) / list_resize_coefficient), x->capacity - 1), true)
#define LIST_PUSH_RESIZE(x)  list_resize (x, (int) fmax ( ceil ( (double) (x->capacity - 1) * list_resize_coefficient), x->capacity + 1), true)

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
    LS_INCORRECT_LINEARIZATION_FLAG  = 16,
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

    Node* root;

    int top_free_ind;


    List_info debug_info;
};



const size_t NODE_SIZE    = sizeof (Node);
const size_t LIST_SIZE    = sizeof (List);

const size_t MAX_COMMAND_LEN = 100;


Return_code _list_ctor          (List* list, const char* name, const char* file, const char* func, int line);
Return_code  list_dtor          (List* list);
Return_code  list_resize        (List* list, int new_capacity, bool linearize = false);
Return_code _list_canary_resize (List* list, int new_capacity);

Return_code  list_push_front    (List* list, Element_value new_element_value);
Return_code  list_push_back     (List* list, Element_value new_element_value);
Return_code  list_push_before   (List* list, int target, Element_value new_element_value);
Return_code  list_push_after    (List* list, int target, Element_value new_element_value);

Element      list_pop_back      (List* list, Return_code* return_code_ptr = nullptr, bool shrink = true);
Element      list_pop_front     (List* list, Return_code* return_code_ptr = nullptr, bool shrink = true);
Element      list_pop_before    (List* list, int target, Return_code* return_code_ptr = nullptr, bool shrink = false);
Element      list_pop_after     (List* list, int target, Return_code* return_code_ptr = nullptr, bool shrink = false);


Element      list_pop           (List* list, Return_code* return_code_ptr = nullptr);
Return_code  list_linearize     (List* list);

bool        list_linearized                  (List* list);
List_state  list_damaged                     (List* list);
void       _flist_dump                       (List* list, const char* file_name, const char* file, const char* func, int line);
void       _flist_graphdump                  (List* list, const char* file_name, const char* file, const char* func, int line, const char* num, const char* additional_text = "");
void        list_show_graph_dump             (void);
void        list_generate_graph_describtion  (List* list, const char* num);
void        list_generate_graph              (const char* num);

Return_code _list_fill_with_poison (List* list, int from,  int to);
Return_code _list_free_stack_fill  (List* list, int first, int last);
Return_code _list_free_stack_push  (List* list, int freed);

int _remainder (int a, int b);








#endif