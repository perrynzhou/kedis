/*
 * MIT License
 *
 * Copyright (c) 2021 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "stl_list.h"

void stl_list_init( stl_list *list)
{
    list->next = list;
    list->prev = list;
}

void stl_list_clear( stl_list *list)
{
     stl_list *tmp, *elem;

    stl_list_foreach_safe (list, tmp, elem) {
        stl_list_del(NULL, elem);
    }
}

bool stl_list_is_empty( stl_list *list)
{
    return list->next == list;
}

size_t stl_list_count( stl_list *list)
{
    size_t count = 0;
     stl_list *elem;

    stl_list_foreach (list, elem) {
        count++;
    }

    return count;
}

 stl_list *stl_list_head( stl_list *list)
{
    return list->next != list ? list->next : NULL;
}

 stl_list *stl_list_tail( stl_list *list)
{
    return list->prev != list ? list->prev : NULL;
}

void stl_list_add_tail( stl_list *list,  stl_list *elem)
{
     stl_list *prev;

    // Delete if exists to prevent adding same item twice
    stl_list_del(list, elem);

    prev = list->prev;
    list->prev = elem;
    elem->next = list;
    elem->prev = prev;
    prev->next = elem;
}

 stl_list *stl_list_pop_tail( stl_list *list)
{
     stl_list *tail = list->prev;

    if (stl_list_is_empty(list)) {
        return NULL;
    }

    stl_list_del(list, list->prev);

    return tail;
}

void stl_list_add_head( stl_list *list,  stl_list *elem)
{
     stl_list *next;

    // Delete if exists to prevent adding same item twice
    stl_list_del(list, elem);

    next = list->next;
    list->next = elem;
    elem->prev = list;
    elem->next = next;
    next->prev = elem;
}

 stl_list *stl_list_pop_head( stl_list *list)
{
     stl_list *head = list->next;

    if (stl_list_is_empty(list)) {
        return NULL;
    }

    stl_list_del(list, list->next);

    return head;
}

void stl_list_add_after( stl_list *list,  stl_list *prev,
                        stl_list *elem)
{
    (void) list;
     stl_list *next;

    // Delete if exists to prevent adding same item twice
    stl_list_del(list, elem);

    next = prev->next;
    prev->next = elem;
    elem->next = next;
    elem->prev = prev;
    next->prev = elem;
}

void stl_list_add_before( stl_list *list,  stl_list *next,
                         stl_list *elem)
{
    (void) list;
     stl_list *prev;

    // Delete if exists to prevent adding same item twice
    stl_list_del(list, elem);

    prev = next->prev;
    next->prev = elem;
    elem->next = next;
    elem->prev = prev;
    prev->next = elem;
}

void stl_list_del( stl_list *list,  stl_list *elem)
{
    (void) (list);

    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    elem->next = elem;
    elem->prev = elem;
}
