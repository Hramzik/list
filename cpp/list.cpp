

#include "../headers/list.hpp"
#include "../lib/logs.hpp"
#include "../lib/stack.hpp"


static char GLOBAL_graph_dump_num [100] = "1";


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
    if (new_capacity <= 0) { LOG_ERROR (BAD_ARGS); LIST_ERROR_DUMP (list); return BAD_ARGS; }


    if (linearize) {

        try ( list_linearize (list) );
    }


    int old_capacity = list->capacity;


    list->root = (Node*) realloc (list->root, NODE_SIZE * new_capacity);
    list->top_free_ind = -1;
    list->capacity = new_capacity;
    if (list->root == nullptr && new_capacity != 0) { LOG_ERROR (MEMORY_ERR); LIST_ERROR_DUMP (list); return MEMORY_ERR; }


    if (new_capacity > old_capacity) {

        try ( _list_fill_with_poison (list, old_capacity, new_capacity) );
    }


    if (list->size > new_capacity) { list->size = new_capacity; }


    try ( _list_free_stack_fill  (list, list->size + 1, new_capacity) );


    LIST_AFTER_OPERATION_DUMPING (list);


    return SUCCESS;
}


Return_code  list_linearize  (List* list) {

    ASSERT_LIST_OK (list);


    if (list->is_linearized) { LIST_AFTER_OPERATION_DUMPING (list); return SUCCESS; }


    Node* new_buffer = (Node*) calloc (list->capacity * NODE_SIZE, 1);
    if (!new_buffer) { LOG_ERROR (MEMORY_ERR); LIST_ERROR_DUMP (list); return MEMORY_ERR; }


    int logical_ind  = 0;
    int physical_ind = 0;
    while (logical_ind <= list->size) {

        new_buffer[logical_ind]      = list->root[physical_ind];
        new_buffer[logical_ind].next = _remainder ( (logical_ind + 1), (list->size + 1) );
        new_buffer[logical_ind].prev = _remainder ( (logical_ind - 1), (list->size + 1) );

        physical_ind = list->root[physical_ind].next;
        logical_ind++;
    }


    free (list->root);
    list->root = new_buffer;


    list->top_free_ind = -1;
    try ( _list_free_stack_fill (list, logical_ind, list->capacity) );


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
    
        list->root[current].element = Element {NAN, true};
        list->root[current].next    = next;
        list->root[current].prev    = -1;

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


    list->root[freed].element = Element {NAN, true};
    list->root[freed].next    = list->top_free_ind;
    list->root[freed].prev    = -1;

    list->top_free_ind        = freed;


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

    return list_push_after (list, 0, new_element_value);
}


Return_code  list_push_back  (List* list, Element_value new_element_value) {

    return list_push_after (list, list->root->prev, new_element_value);
}


Return_code  list_push_before  (List* list, int target, Element_value new_element_value) {

    return list_push_after (list, list->root [target].prev, new_element_value);
}


Return_code  list_push_after  (List* list, int target, Element_value new_element_value) {

    ASSERT_LIST_OK (list);


    if (list->top_free_ind == -1) {

        try ( LIST_PUSH_RESIZE (list) );
    }


    int anker          = list->top_free_ind;
    list->top_free_ind = list->root [list->top_free_ind].next;


    list->root [anker] = Node {
        .element = Element {new_element_value, false},
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


Element  list_pop_back  (List* list, Return_code* return_code_ptr, bool shrink) {

    return list_pop_after (list, list->root [list->root->prev].prev, return_code_ptr, shrink);
}


Element  list_pop_front  (List* list, Return_code* return_code_ptr, bool shrink) {

    return list_pop_after (list, 0, return_code_ptr, shrink);
}


Element  list_pop_before  (List* list, int target, Return_code* return_code_ptr, bool shrink) {

    return list_pop_after (list, list->root [list->root [target].prev].prev, return_code_ptr, shrink);
}


Element  list_pop_after  (List* list, int target, Return_code* return_code_ptr, bool shrink) {

    ASSERT_LIST_OK_FOR_LIST_POP (list);


    Element return_element = {NAN, true};
    int     deleted_ind    = list->root [target].next;


    if (list->size != 0) {

        list->size -= 1;


        return_element = list->root [deleted_ind].element;


        list->root [list->root [deleted_ind].next].prev = target;                        //ubernext
        list->root [target].next                        = list->root [deleted_ind].next; //target
        _list_free_stack_push (list, deleted_ind);                                       //deleted
    }


    if ( (shrink) && (double) list->size * pow (list_resize_coefficient, 2) <= (double) (list->capacity - 1) ) {

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


List_state  list_damaged  (List* list) {

    List_state list_state = LS_OK;


    if (list == nullptr) { list_state |= LS_NULLPTR; return list_state; }


    if (list->size > list->capacity) { list_state |= LS_SIZE_GREATER_THAN_CAPACITY; }


    if (list->root == nullptr)       { list_state |= LS_NULLPTR_NODES; }


    if (!list->root->element.poisoned) { list_state |= LS_INCORRECT_POISON_DISTRIBUTION; }
    for (int i = 1; i < list->capacity; i++) {
    
        if ((list->root [i].prev != -1 &&  list->root [i].element.poisoned) ||
            (list->root [i].prev == -1 && !list->root [i].element.poisoned)) {
        
            list_state |= LS_INCORRECT_POISON_DISTRIBUTION;
            break;
        }
    }


    if (list->is_linearized && !list_linearized (list)) { list_state |= LS_INCORRECT_LINEARIZATION_FLAG; }


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


    fprintf (dump_file, "list is");
    if (list->is_linearized) { fprintf (dump_file,    " linearized\n"); }
    else                     { fprintf (dump_file, "n't linearized\n"); }


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


int  _remainder  (int a, int b) {

    assert (b != 0);


    int ans = a % b;
    if (ans < 0) { ans += b; }


    return ans;
}


bool  list_linearized  (List* list) {

    if (!list) { return false; }


    for (int i = 0; i <= list->size; i++) {
    
        if (list->root [i].next != _remainder (i + 1, list->size + 1) ||
            list->root [i].prev != _remainder (i - 1, list->size + 1)) {

            return false;
        }
    }


    return true;
}


void  _flist_graphdump  (List* list, const char* file_name, const char* file, const char* func, int line, const char* additional_text) {

    assert ( (file_name) && (file) && (func) && (line > 0) );


    char file_path [MAX_COMMAND_LEN] = "";
    strcat (file_path, file_name);
    strcat (file_path, ".html");


    const char* file_mode = nullptr;
    if ( !strcmp (GLOBAL_graph_dump_num, "1")) { file_mode = "w"; }
    else                                       { file_mode = "a"; }


    FILE* dump_file = fopen (file_path, file_mode);
    if (dump_file == nullptr) { LOG_ERROR (FILE_ERR); return; }


    setvbuf (dump_file, NULL, _IONBF, 0);


    fprintf (dump_file, "<pre><h1>");
    fprintf (dump_file,"%s", additional_text);
    fprintf (dump_file, "</h1>");
    fprintf (dump_file, "<h2>Dumping list at %s in function %s (line %d)...</h2>\n\n", file, func, line);


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


    fprintf (dump_file, "list is");
    if (list->is_linearized) { fprintf (dump_file,    " linearized\n"); }
    else                     { fprintf (dump_file, "n't linearized\n"); }


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


    char name [MAX_COMMAND_LEN] = list_graph_file_name;
    strcat (name, GLOBAL_graph_dump_num);
    strcat (name, ".svg");
    fprintf (dump_file, "<img src=\"%s\" width=\"%zd\">", name, GRAPH_WIDTH);


    itoa ( (atoi (GLOBAL_graph_dump_num) + 1), GLOBAL_graph_dump_num, DEFAULT_COUNTING_SYSTEM); //incrementation of graph_dump_num


    fclose (dump_file);
}


void  list_show_graph_dump  (void) {

    char command [MAX_COMMAND_LEN] = "start ";
    strcat (command, list_graph_dump_file_name);
    strcat (command, ".html");
    system (command);
}

void  list_generate_graph_describtion  (List* list) {

    if (!list) { return; }


    printf ("generating %s graph dump...\n", GLOBAL_graph_dump_num);


    char name [MAX_COMMAND_LEN] = "";
    strcat (name, list_graph_describtion_file_name);
    strcat (name, GLOBAL_graph_dump_num);
    strcat (name, ".txt");

    FILE* graph_file = fopen (name, "w");
    if (graph_file == nullptr) { LOG_ERROR (FILE_ERR); return; }


    fprintf (graph_file, "digraph G {\n\n");


    fprintf (graph_file, "    ranksep = 0.5; splines = ortho\n    bgcolor = \"#%s\"\n", list_dump_bgclr);
    fprintf (graph_file, "    edge [minlen = 3, penwidth = 3];\n    node [shape = record, style = rounded, fixedsize = true, height = 1, width = 2, fontsize = 10];\n\n");
    fprintf (graph_file, "    {rank = min; above_node [width = 3, style = invis];}\n\n");
    fprintf (graph_file, "    {rank = same;\n");

    for (int i = 0; i < list->capacity; i++) {

        if (list->root [i].prev != -1) {

            fprintf (graph_file, "        node%d [style = \"rounded, filled\", color=\"#%s\", ", i, list_dump_nodeclr);
        }

        else {

            fprintf (graph_file, "        node%d [style = \"rounded, filled\", color=\"#%s\", ", i, list_dump_freeclr);
        }

        fprintf (graph_file, "label = \"{[%i] | %.2lf | next = %d | prev = %d}\"];\n", i, list->root [i].element.value, list->root [i].next, list->root [i].prev);
    }

    fprintf (graph_file, "    }\n\n    {rank = max; below_node[width = 3, style = invis]; }\n\n");

    fprintf (graph_file, "    above_node -> node0 [style = invis]; below_node -> node0 [style = invis];\n\n");


    for (int i = 0; i < list->capacity; i++) {

        if (i + 1 != list->capacity) {

            fprintf (graph_file, "    node%d -> node%d [style = invis, weight = 5]\n", i, i + 1);
        }

        if (list->root [i].next != -1) {

            if (list->root [i].prev != -1) {
            
                fprintf (graph_file, "    node%d -> node%d [color = \"#%s\"]\n", i, list->root [i].next, list_dump_arrwclr);
            }

            else {

                fprintf (graph_file, "    node%d -> node%d [color = \"#%s\"]\n", i, list->root [i].next, list_dump_freearrwclr);
            }
        }
    }


    fprintf (graph_file, "}");


    fclose (graph_file);
}


void  list_generate_graph  (void) {

    char command [MAX_COMMAND_LEN] = "dot -Tsvg ";
    strcat (command, list_graph_describtion_file_name);
    strcat (command, GLOBAL_graph_dump_num);
    strcat (command, ".txt");
    strcat (command, " -o ");
    strcat (command, "work/");
    strcat (command, list_graph_file_name);
    strcat (command, GLOBAL_graph_dump_num);
    strcat (command, ".svg");
    system (command);
}


int  list_search_physical_index_given_logical_index  (List* list, int logical_ind) { //don't use this too much!

    ASSERT_LIST_OK (list);


    int physical_ind = 0;
    for (int i = 0; i < logical_ind; i++) {
    
        physical_ind = list->root [physical_ind].next;
    }


    return physical_ind;
}

