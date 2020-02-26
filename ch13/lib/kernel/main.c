#include "list.h"
#include <stdio.h>

int main()
{
    list lst;
    list_elem a, b, c;
    list_init(&lst);
    
    printf("%d\n", list_len(&lst));
    list_push_back(&lst, &a);
    list_pop_head(&lst);
    printf("%d\n", list_len(&lst));
    list_push_back(&lst, &a);
    printf("%d\n", list_len(&lst));
     list_push_back(&lst, &b);
    printf("%d\n", list_len(&lst)); 
    list_push_back(&lst, &c);
    list_pop_head(&lst);
    printf("%d\n", list_len(&lst));

    return 0;
}

