#include "stl_thread.h"

#include <stdio.h>

void* fn(void* arg)
{
    printf("%s \n", (char*) arg);
    return arg;
}

int main()
{
     stl_thread thread;

    stl_thread_init(&thread);
    stl_thread_start(&thread, fn, "first");
    stl_thread_term(&thread);

    return 0;
}
