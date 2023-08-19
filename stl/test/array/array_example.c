#include "stl_array.h"

#include <stdio.h>

void example_str()
{
    char **p, *it;

    stl_array_create(p, 0);

    stl_array_add(p, "item0");
    stl_array_add(p, "item1");
    stl_array_add(p, "item2");

    printf("\nDelete first element \n\n");
    stl_array_del(p, 0);

    stl_array_foreach (p, it) {
        printf("Elem = %s \n", it);
    }

    stl_array_destroy(p);
}

void example_int()
{
    int *p;

    stl_array_create(p, 0);

    stl_array_add(p, 0);
    stl_array_add(p, 1);
    stl_array_add(p, 2);

    for (size_t i = 0; i < stl_array_size(p); i++) {
        printf("Elem = %d \n", p[i]);
    }

    stl_array_destroy(p);
}

int main()
{
    example_int();
    example_str();

    return 0;
}
