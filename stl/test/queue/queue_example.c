#include "stl_queue.h"

#include <stdio.h>

int main()
{
    int *queue;
    int elem;

    stl_queue_create(queue, 0);

    stl_queue_add_last(queue, 2);
    stl_queue_add_last(queue, 3);
    stl_queue_add_last(queue, 4);
    stl_queue_add_first(queue, 1);

    stl_queue_foreach (queue, elem) {
        printf("elem = [%d] \n", elem);
    }

    elem = stl_queue_del_last(queue);
    printf("Last element was : [%d] \n", elem);

    elem = stl_queue_del_first(queue);
    printf("First element was : [%d] \n", elem);

    stl_queue_destroy(queue);

    return 0;
}
