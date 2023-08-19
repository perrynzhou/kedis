#include "stl_cond.h"

#include <stdio.h>

int main()
{
     stl_cond cond;

    stl_cond_init(&cond); // Init once

    stl_cond_signal(&cond, "test"); // Call this on thread-1
    char* p = stl_cond_wait(&cond); // Call this on another thread.

    printf("%s \n", p); // Prints "test"

    stl_cond_term(&cond); // Destroy

    return 0;
}
