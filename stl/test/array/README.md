### Generic array

### Overview

- Type generic array/vector.
- Index access is possible (e.g float* arr; 'printf("%f", arr[i]')).


### Usage


```c
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

int main(int argc, char *argv[])
{
    example_int();
    example_str();

    return 0;
}
```

##### Note

Array pointer is not stable. If you pass the array to another function which  
can add items, do it by passing reference of the array pointer :

```c
void some_function_to_add_elems(long **p)
{
    stl_array_add(*p, 500);
}

int main(int argc, char *argv[])
{
    long *p;

    stl_array_create(p, 0);
    stl_array_add(p, 300);
    
    // Pass via address of p
    some_function_to_add_elems(&p);
    stl_array_destroy(p);
}

```

Most probably you will hold array pointer in a struct anyway, so you have a  
stable address of the pointer, "*numbers" might change but "numbers" is stable :

```c
struct my_app {
    int *numbers;
};

void func(struct my_app* app)
{
    stl_array_add(app->numbers, 0);
    stl_array_add(app->numbers, 1);
    stl_array_add(app->numbers, 2);
}

```