#include "stl_heap.h"

#include <stdio.h>


int main()
{
    struct data
    {
        int priority;
        char *data;
    };

    struct data n[] = {{1, "first"},
                       {4, "fourth"},
                       {5, "fifth"},
                       {3, "third"},
                       {2, "second"}};

    int64_t key;
    void *data;
    struct stl_heap heap;

    stl_heap_init(&heap, 0);

    // Min-heap usage
    for (int i = 0; i < 5; i++) {
        stl_heap_add(&heap, n[i].priority, n[i].data);
    }

    while (stl_heap_pop(&heap, &key, &data)) {
        printf("key = %ld, data = %s \n", (long int) key, (char *) data);
    }
    printf("---------------- \n");

    /**
     * Max-heap usage, negate when adding into heap
     * and negate back after pop :
     */

    for (int i = 0; i < 5; i++) {
        stl_heap_add(&heap, -(n[i].priority), n[i].data);
    }

    while (stl_heap_pop(&heap, &key, &data)) {
        printf("key = %ld, data = %s \n", (long int) -key, (char *) data);
    }

    return 0;
}
