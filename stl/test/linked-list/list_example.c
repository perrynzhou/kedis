/*
 * @Author: perrynzhou perrynzhou@gmail.com
 * @Date: 2023-03-30 12:19:45
 * @LastEditors: perrynzhou perrynzhou@gmail.com
 * @LastEditTime: 2023-03-30 19:50:13
 * @FilePath: /kirinfs/stl/linked-list/list_example.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stl_list.h"

#include <stdio.h>


int main()
{
    struct user
    {
        char *name;
        struct stl_list next;
    };

    struct user users[] = {{"first", {0}},
                           {"second", {0}},
                           {"third", {0}},
                           {"fourth", {0}},
                           {"fifth", {0}}};

    struct stl_list list;

    stl_list_init(&list);


    for (int i = 0; i < 5; i++) {
        stl_list_init(&users[i].next);
        stl_list_add_tail(&list, &users[i].next);
    }

    struct stl_list *it;
    struct user *user;

    stl_list_foreach (&list, it) {
        user = stl_list_entry(it, struct user, next);
        printf("%s \n", user->name);
    }

    return 0;
}
