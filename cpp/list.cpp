

#include "../headers/list.hpp"
#include "../lib/logs.hpp"
#include "../lib/stack.hpp"


Return_code  _list_ctor  (List* list, const char* name, const char* file, const char* func, int line) {

    assert ( (file) && (name) && (func) && (line > 0) );
    if (list == nullptr) { LOG_ERROR (BAD_ARGS); LIST_ERROR_DUMP (list); return BAD_ARGS; }


    list->root          = (Node*) calloc (NODE_SIZE, 1);
    list->root->element = {NAN, true};
    list->root->next    = 0;
    list->root->prev    = 0;


    list->size          = 0;
    list->capacity      = 1;
    list->is_linearized = true;

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


    free (list->root);
    list->root = nullptr;


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code  list_resize  (List* list, int new_capacity, bool linearize) {

    ASSERT_LIST_OK (list);


    if (linearize) {

        try ( list_linearize (list) );
    }


    int old_capacity = list->capacity;


    list->root = (Node*) realloc (list->root, NODE_SIZE * new_capacity);
    list->capacity = new_capacity;
    if (list->root == nullptr && new_capacity != 0) { LOG_ERROR (MEMORY_ERR); LIST_ERROR_DUMP (list); return MEMORY_ERR; }


    try ( _list_fill_with_poison (list, old_capacity, new_capacity) );
    if (new_capacity > old_capacity) { try ( _list_free_stack_fill (list, old_capacity, new_capacity) ); }


    if (list->size > new_capacity) { list->size = new_capacity; }


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code  list_linearize  (List* list) {

    ASSERT_LIST_OK (list);


    if (list->is_linearized) { LIST_AFTER_OPERATION_DUMPING (list); return SUCCESS; }


    Node* new_buffer = (Node*) calloc (list->capacity * NODE_SIZE, 1);
    if (!new_buffer) { LOG_ERROR (MEMORY_ERR); LIST_ERROR_DUMP (list); return MEMORY_ERR; }


    int logical_num  = 0;
    int physical_num = 0;
    while (logical_num <= list->size) {

        new_buffer[logical_num]      = list->root[physical_num];
        new_buffer[logical_num].next = _remainder ( (logical_num + 1), (list->size + 1) );
        new_buffer[logical_num].prev = _remainder ( (logical_num - 1), (list->size + 1) );

        physical_num = list->root[physical_num].next;
        logical_num++;
    }


    free (list->root);
    list->root = new_buffer;


    try ( _list_free_stack_fill (list, logical_num, list->capacity) );


    list->is_linearized = true;


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code _list_free_stack_fill  (List* list, int first, int last) {

    if ( (!list) || (first < 0) || (last < 0) ) { LOG_ERROR (BAD_ARGS); LIST_ERROR_DUMP(list); return BAD_ARGS; }


    int  next     = list->top_free_ind;
    int  current  = last - 1;
    bool changed_flag = false;
    while (current >= first) {
    
        list->root[current].next = next;
        next = current;
        current--;

        changed_flag = true;
    }


    if (changed_flag) {

        current++;
        list->top_free_ind = current;
    }


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code _list_free_stack_push  (List* list, int freed) { 

    if ( (!list) || (freed < 0) ) { LOG_ERROR (BAD_ARGS); LIST_ERROR_DUMP(list); return BAD_ARGS; }


    list->root[freed].next = list->top_free_ind;
    list->top_free_ind = freed;


    return SUCCESS;
}


Return_code _list_fill_with_poison (List* list, int from,  int to) {

    if ( (from < 0) || (to < 0) ) { LOG_ERROR (BAD_ARGS); LIST_ERROR_DUMP(list); return BAD_ARGS; }


    for (int current = from; current < to; current++) {
    
        list->root[current].element = {NAN, true};
        list->root[current].prev  = -1;
        list->root[current].next  = -1;
    }


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code  list_push_front  (List* list, Element_value new_element_value) {

    ASSERT_LIST_OK (list);


    if (list->top_free_ind == -1) {

        try ( LIST_PUSH_RESIZE (list) );
    }


    int anker          = list->top_free_ind;
    list->top_free_ind = list->root [list->top_free_ind].next;


    list->root [anker] = {
        .element = {.value = new_element_value, .poisoned = false},
        .prev    = 0,
        .next    = list->root->next,
    };

    list->root [list->root->next].prev = anker;
    list->root->next                   = anker;


    list->is_linearized = false;
    list->size += 1;


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code  list_push_back  (List* list, Element_value new_element_value) {

    ASSERT_LIST_OK (list);


    if (list->top_free_ind == -1) {

        try ( LIST_PUSH_RESIZE (list) );
    }


    int anker          = list->top_free_ind;
    list->top_free_ind = list->root [list->top_free_ind].next;


    list->root [anker] = {
        .element = {new_element_value, false},
        .prev    = list->root->prev,
        .next    = 0
    };

    list->root [list->root->prev].next = anker;
    list->root->prev                   = anker;


    list->is_linearized = false;
    list->size += 1;


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code  list_push_before  (List* list, int target, Element_value new_element_value) {

    ASSERT_LIST_OK (list);


    if (list->top_free_ind == -1) {

        try ( LIST_PUSH_RESIZE (list) );
    }


    int anker          = list->top_free_ind;
    list->top_free_ind = list->root [list->top_free_ind].next;


    list->root [anker] = {
        .element = {new_element_value, false},
        .prev    = list->root [target].prev,
        .next    = target,
    };

    list->root [list->root [target].prev].next = anker;
    list->root [target].prev                   = anker;


    list->is_linearized = false;
    list->size += 1;


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code  list_push_after  (List* list, int target, Element_value new_element_value) {

    ASSERT_LIST_OK (list);


    if (list->top_free_ind == -1) {

        try ( LIST_PUSH_RESIZE (list) );
    }


    int anker          = list->top_free_ind;
    list->top_free_ind = list->root [list->top_free_ind].next;


    list->root [anker] = {
        .element = {new_element_value, false},
        .prev    = target,
        .next    = list->root [target].next,
    };

    list->root [list->root [target].next].prev = anker;
    list->root [target].next                   = anker;


    list->is_linearized = false;
    list->size += 1;


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}

/*
Element  list_pop  (List* list, Return_code* return_code_ptr) {

    ASSERT_LIST_OK_FOR_LIST_POP (list);


    Element return_element = {NAN, true};

    if (list->size != 0) {

        list->size -= 1;
        return_element = list->root [list->size];
        list->root [list->size] = Element {NAN, true};
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
    if (list->root == nullptr)       { list_state |= LS_NULLPTR_NODES; }


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


    fprintf (dump_file, "list is ");
    if (list->is_linearized) { fprintf (dump_file, "n't linearized\n"); }
    else                     { fprintf (dump_file,     "linearized\n"); }


    fprintf (dump_file, "root     %d\n",   1);
    fprintf (dump_file, "size     %d\n",   list->size);
    fprintf (dump_file, "capacity %d\n\n", list->capacity);
    if (list->root) { fprintf (dump_file, "nodes [%p]:\n", list->root); }
    else            { fprintf (dump_file, "nodes [nullptr]:\n"); }


    for (int i = 0; i < list->capacity; i++) {
    
        if (i == 0)                         { fprintf (dump_file, "(root)     "); }
        else if (list->root [i].prev != -1) { fprintf (dump_file, "(in)       "); }
        else                                { fprintf (dump_file, "(out)      "); }

        fprintf (dump_file, "[%2d] ", i);
        fprintf (dump_file, "%8.2lf     ", list->root [i].element.value);

        if (list->root [i].element.poisoned) { fprintf (dump_file, "    (poisoned)     "); }
        else                                 { fprintf (dump_file, "(not poisoned)     "); }

        fprintf (dump_file, "next - [%2d],     prev - [%2d]\n", list->root [i].next, list->root [i].prev);
    }


    fprintf (dump_file, "\nfree elements:\n");
    for (int i = list->top_free_ind; i != -1; i = list->root [i].next) {
    
        if (list->root [i].prev != -1) { fprintf (dump_file, "(in)       "); }
        else                           { fprintf (dump_file, "(out)      "); }

        fprintf (dump_file, "[%2d] ", i);
        fprintf (dump_file, "%8.2lf     ",  list->root [i].element.value);

        if (list->root [i].element.poisoned) { fprintf (dump_file, "    (poisoned)     "); }
        else                                 { fprintf (dump_file, "(not poisoned)     "); }

        fprintf (dump_file, "next - [%2d],     prev - [%2d]\n", list->root [i].next, list->root [i].prev);
    }


    fprintf (dump_file, "\n");


    fclose (dump_file);
}

/*
Return_code _list_fill_with_poison (List* list, int from, int to) {

    if (!list || (to > list->capacity) ) { LOG_ERROR (BAD_ARGS); LIST_ERROR_DUMP(list); return BAD_ARGS; }


    for (int i = from; i < to; i++) {

        list->root[i] = Element {NAN, true};
    }


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}
*/


int  _remainder  (int a, int b) {

    assert (b != 0);


    int ans = a % b;
    if (ans < 0) { ans += b; }


    return ans;
}


