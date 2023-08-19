#include "stl_string.h"

#include <stdio.h>

int main()
{
    char* s1;

    s1 = stl_string_new("*-hello-*");
    printf("%s \n", s1); // prints **hello**
    
    stl_string_destroy(s1);
    /*
    stl_string_trim(&s1, "*-");
    printf("%s \n", s1); // prints hello

    stl_string_append_fmt(&s1, "%d", 2);
    printf("%s \n", s1); // prints hello2

    stl_string_replace(&s1, "2", " world!");
    printf("%s \n", s1); // prints hello world!

    stl_string_substring(&s1, 0, 5);
    printf("%s \n", s1); // prints hello

    stl_string_destroy(s1);
    */

    return 0;
}
