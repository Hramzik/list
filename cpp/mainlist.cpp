

#include "../headers/list.hpp"


int main (void) {

    List list = {};
    LIST_CTOR (&list);


    list_push_back (&list, 1);
    list_push_back (&list, 2);
    list_push_back (&list, 3);


    printf ("\nfuck yeah\n");
    FLIST_DUMP (&list);


    return 0;
}