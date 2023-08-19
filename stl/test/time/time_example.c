#include "stl_time.h"

#include <stdio.h>

int main()
{
    uint64_t t = stl_time_ms();
    stl_time_sleep(1000);
    printf("%lu \n", (unsigned long) (stl_time_ms() - t));

    return 0;
}
