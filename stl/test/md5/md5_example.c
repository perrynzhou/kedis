#include "stl_md5.h"

#include <stdio.h>

int main()
{

    char *str = "perrynzhou@gmail.com";
    fprintf(stdout, " md5 before:%s\n", str);
     stl_md5 ctx;
    stl_md5_init(&ctx);
    stl_md5_compute(&ctx, str, strlen(str));

    char md5_value[16] = {'\0'};
    stl_md5_checksum(&ctx, (char *)&md5_value);
    fprintf(stdout, " md5 value2:%s\n", (char *)&md5_value);
    return 0;
}
