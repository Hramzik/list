

#include "../headers/list.hpp"
#include "../lib/logs.hpp"
#include "../lib/stack.hpp"


Return_code  _list_ctor  (List* list, const char* name, const char* file, const char* func, int line) {

    assert ( (file) && (name) && (func) && (line > 0) );
    if (list == nullptr) { LOG_ERROR (BAD_ARGS); LIST_ERROR_DUMP (list); return BAD_ARGS; }


    list->eternal          = (Node*) calloc (NODE_SIZE, 1);
    list->eternal->element = {NAN, true};
    list->eternal->next    = 0;
    list->eternal->prev    = 0;


    list->size          = 0;
    list->capacity      = 0;
    list->is_linearized = false;

    list->top_free_ind = -1; 


    list->debug_info.name       = name;
    list->debug_info.birth_file = file;
    list->debug_info.birth_func = func;
    list->debug_info.birth_line = line;
    list->debug_info.adress     = list;


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code  list_dtor  (List* list) {

    ASSERT_LIST_OK (list);


    free (list->eternal);
    list->eternal = nullptr;


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code  _list_resize  (List* list, int new_capacity) {

    ASSERT_LIST_OK (list);


    if (!list->is_linearized) {

        list_linearize (list);
    }


    list->eternal = (Node*) realloc (list->eternal, NODE_SIZE * new_capacity);
    if (list->eternal == nullptr && new_capacity != 0) { LOG_ERROR (MEMORY_ERR); LIST_ERROR_DUMP (list); return MEMORY_ERR; }


    _list_fill_with_poison (list, list->capacity, new_capacity);
    if (new_capacity > list->capacity) { _list_free_stack_fill (list, list->capacity, new_capacity); }


    list->capacity = new_capacity;
    if (list->size > new_capacity) { list->size = new_capacity; }


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code  list_linearize  (List* list) {

    ASSERT_LIST_OK (list);


    if (list->is_linearized) { return SUCCESS; }


    Node* new_buffer = (Node*) calloc (list->capacity, 1);
    if (!new_buffer) { LOG_ERROR (MEMORY_ERR); LIST_ERROR_DUMP (list); return MEMORY_ERR; }


    int logical_num  = 0;
    int physical_num = 0;
    do {

        new_buffer[logical_num]      = list->eternal[physical_num];
        new_buffer[logical_num].next = (logical_num + 1) % (list->size);
        new_buffer[logical_num].prev =  logical_num - 1;
        physical_num = list->eternal[physical_num].next;
        logical_num++;
    }
    while (list->eternal[physical_num].next != 0);


    free (list->eternal);
    list->eternal = new_buffer;


    _list_free_stack_fill (list, logical_num, list->capacity);


    list->is_linearized = true;


    return SUCCESS;
}



Return_code _list_free_stack_fill  (List* list, int first, int last) {

    if ( (list_damaged (list)) || (first < 0) || (first >= list->capacity) 
                               || (last  < 0) || (last  >= list->capacity) ) { LOG_ERROR (BAD_ARGS); LIST_ERROR_DUMP(list); return BAD_ARGS; }


    int previous = list->top_free_ind;
    for (int current = first; current < last;   ) {
    
        list->eternal[current].next = previous;
        previous = current;
        current++;
    }


    return SUCCESS;
}


Return_code _list_free_stack_push  (List* list, int freed) { 

    if ( (list_damaged (list)) || (freed < 0) || (freed >= list->capacity) ) { LOG_ERROR (BAD_ARGS); LIST_ERROR_DUMP(list); return BAD_ARGS; }


    list->eternal[freed].next = list->top_free_ind;
    list->top_free_ind = freed;


    return SUCCESS;
}


Return_code _list_fill_with_poison (List* list, int from,  int to) {

    if ( (list_damaged (list)) || (from < 0) || (from >= list->capacity) 
                               || (to   < 0) || (to   >= list->capacity) ) { LOG_ERROR (BAD_ARGS); LIST_ERROR_DUMP(list); return BAD_ARGS; }


    for (int current = from; current < to; current++) {
    
        list->eternal[current].element = {NAN, true};
        list->eternal[current].prev  = -1;
    }


    return SUCCESS;
}


Return_code  list_push_front  (List* list, Element_value new_element_value) {

    ASSERT_LIST_OK (list);


    if ()


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Element  list_pop  (List* list, Return_code* return_code_ptr) {

    ASSERT_LIST_OK_FOR_LIST_POP (list);


    Element return_element = {NAN, true};

    if (list->size != 0) {

        list->size -= 1;
        return_element = list->eternal [list->size];
        list->eternal [list->size] = Element {NAN, true};
    }


    if ( (double) list->size * pow (list_resize_coefficient, 2) <= (double) list->capacity) {

        Return_code resize_code = LIST_POP_RESIZE (list);

        if (resize_code) {

            LOG_ERROR (resize_code);
            if (return_code_ptr) { *return_code_ptr = BAD_ARGS; }
            LIST_ERROR_DUMP (list);
            return Element {NAN, true};
        }
    }


    if (return_code_ptr) { *return_code_ptr = SUCCESS; }


    LIST_AFTER_OPERATION_DUMPING (list);


    return return_element;
}
*/

List_state  list_damaged  (List* list) {

    List_state list_state = LS_OK;


    if (list == nullptr) { list_state |= LS_NULLPTR; return list_state; }


    if (list->size > list->capacity) { list_state |= LS_SIZE_GREATER_THAN_CAPACITY; }
    if (list->eternal == nullptr)      { list_state |= LS_NULLPTR_NODES; }


    return list_state;
}


void  _flist_dump  (List* list, const char* file_name, const char* file, const char* func, int line) {

    assert ( (file_name) && (file) && (func) && (line > 0) );


    FILE* dump_file = fopen (file_name, "a");
    if (dump_file == nullptr) { LOG_ERROR (FILE_ERR); return; }


    setvbuf (dump_file, NULL, _IONBF, 0);


    fprintf (dump_file, "--------------------\n");
    fprintf (dump_file, "Dumping list at %s in function %s (line %d)...\n\n", file, func, line);


    if (!list) { fprintf (dump_file, "List pointer is nullptr!\n\n"); return; }


    fprintf (dump_file, "this list has name ");
    if (list->debug_info.name != nullptr) { fprintf (dump_file, "%s ", list->debug_info.name); }
    else                                  { fprintf (dump_file, "UNKNOWN NAME "); }
    fprintf (dump_file, "[%p]\n", list->debug_info.adress);

    fprintf (dump_file, "it was created in file ");
    if (list->debug_info.birth_file != nullptr) { fprintf (dump_file, "%s\n", list->debug_info.birth_file); }
    else                                        { fprintf (dump_file, "UNKNOWN NAME\n"); }

    fprintf (dump_file, "in function ");
    if (list->debug_info.birth_func != nullptr) { fprintf (dump_file, "%s ", list->debug_info.birth_func); }
    else                                        { fprintf (dump_file, "UNKNOWN NAME "); }

    fprintf (dump_file, "(line %d)\n\n", list->debug_info.birth_line);


    fprintf (dump_file, "list is ");
    List_state list_state = list_damaged (list);
    if (list_state) { fprintf (dump_file, "damaged (damage code %u)\n", list_state); }
    else            { fprintf (dump_file, "ok\n"); }


    fprintf (dump_file, "size     %d\n",      list->size);
    fprintf (dump_file, "capacity %d\n\n",  list->capacity);
    if (list->eternal) { fprintf (dump_file, "nodes [%p]:\n", list->eternal); }
    else               { fprintf (dump_file, "nodes [nullptr]:\n"); }


    fprintf (dump_file, "\n");


    fclose (dump_file);
}

/*
Return_code _list_fill_with_poison (List* list, int from, int to) {

    if (!list || (to > list->capacity) ) { LOG_ERROR (BAD_ARGS); LIST_ERROR_DUMP(list); return BAD_ARGS; }


    for (int i = from; i < to; i++) {

        list->eternal[i] = Element {NAN, true};
    }


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}
*/


