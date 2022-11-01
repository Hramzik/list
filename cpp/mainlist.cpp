

#include "../headers/list.hpp"


int main (void) {

    List list = {};
    LIST_CTOR (&list);


    list_push_back  (&list, 1);
    list_push_back  (&list, 2);
    list_push_back  (&list, 3);
    list_push_back  (&list, 4);
    list_push_back  (&list, 5);


    FLIST_GRAPHDUMP (&list, "after pushes:");


    list_pop_front  (&list);
    list_pop_before (&list, 3);
    list_pop_after  (&list, 3);


    FLIST_GRAPHDUMP (&list, "after all:");


    printf ("\ndone\n");


    list_show_graph_dump ();


    return 0;
}