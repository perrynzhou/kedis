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

#ifndef STL_LIST_H
#define STL_LIST_H

#include <stdbool.h>
#include <stddef.h>

#define STL_LIST_VERSION "1.0.0"

struct stl_list
{
    struct stl_list *next;
    struct stl_list *prev;
};

typedef struct stl_list stl_list;
#define stl_list_entry(ptr, type, elem) \
    ((type *)((char *)(ptr)-offsetof(type, elem)))

/**
 * Call once to initialize.
 * @param list list
 */
void stl_list_init(stl_list *list);

/**
 * @param list list
 */
void stl_list_clear(stl_list *list);

/**
 * @param list list
 * @return     'true' if empty
 */
bool stl_list_is_empty(stl_list *list);

/**
 * @param list list
 * @return     element count in the list, beware this is a log(n) operation.
 */
size_t stl_list_count(stl_list *list);

/**
 * @param list list
 * @return     returns head. If list is empty, returns NULL.
 */
stl_list *stl_list_head(stl_list *list);

/**
 * @param list list
 * @return     returns tail. If list is empty, returns NULL.
 */
stl_list *stl_list_tail(stl_list *list);

/**
 *  before : [head]item1 -> [tail]item2
 *  after  : [head]'elem' -> item1 -> [tail]item2
 *
 * @param list list
 * @param elem elem to add to the head
 */
void stl_list_add_head(stl_list *list, stl_list *elem);

/**
 *  before : [head]item1 -> item2 -> item3
 *  after  : [head]item2 -> item3
 *
 * @param list list
 * @return     head element, if list is empty, returns NULL.
 */
stl_list *stl_list_pop_head(stl_list *list);

/**
 *  before : [head]item1 -> [tail]item2
 *  after  : [head]item1 -> item2 -> [tail]'elem'
 *
 * @param list list
 * @param elem elem to append to the tail
 */
void stl_list_add_tail(stl_list *list, stl_list *elem);

/**
 *  before : [head]item1 -> item2 -> item3
 *  after  : [head]item1 -> item2
 *
 * @param list list
 * @return     head element, if list is empty, returns NULL.
 */
stl_list *stl_list_pop_tail(stl_list *list);

/**
 *  before : item1 -> 'prev' -> item2
 *  after  : item1 -> 'prev' -> 'elem' -> item2
 *
 * @param list list
 * @param prev previous element of the 'elem'
 * @param elem elem to be added after 'prev'
 */
void stl_list_add_after(stl_list *list, stl_list *prev,
                        stl_list *elem);

/**
 *  before : item1 -> 'next' -> item2
 *  after  : item1 -> 'elem'-> 'next' -> item2
 *
 * @param list list
 * @param next next element of the 'elem'
 * @param elem elem to be added before 'next'
 */
void stl_list_add_before(stl_list *list, stl_list *next,
                         stl_list *elem);

/**
 *  before : item1 -> 'elem' -> item2
 *  after  : item1 -> item2
 *
 * @param list list pointer
 * @param elem elem to be deleted
 */
void stl_list_del(stl_list *list, stl_list *elem);

/**
 *  container {
 *       stl_list others;
 * };
 *
 *  container *container; // User object
 *  stl_list *list;        // List pointer, should already be initialized.
 *  stl_list *it;          // Iterator
 *
 * stl_list_foreach (list, it) {
 *       container = stl_list_entry(it,  container, others);
 * }
 */
#define stl_list_foreach(list, elem) \
    for ((elem) = (list)->next; (elem) != (list); (elem) = (elem)->next)

/**
 *  It is safe to delete items from the list while using this iterator.
 *
 *  container {
 *       stl_list others;
 * };
 *
 *  container *container; // User object
 *  stl_list *list;        // List pointer, should already be initialized.
 *  stl_list *tmp;         // Variable for loop, user should not use this.
 *  stl_list *it;          // Iterator
 *
 * stl_list_foreach_safe (list, it) {
 *       container = stl_list_entry(it,  container, others);
 *       stl_list_del(list, &container->others);
 * }
 */
#define stl_list_foreach_safe(list, n, elem)                          \
    for ((elem) = (list)->next, (n) = (elem)->next; (elem) != (list); \
         (elem) = (n), (n) = (elem)->next)

/**
 * Reverse iterator
 *
 *  container {
 *       stl_list others;
 * };
 *
 *  container *container; // User object
 *  stl_list *list;        // List pointer, should already be initialized.
 *  stl_list *it;          // Iterator
 *
 * stl_list_foreach_r (list, it) {
 *       container = stl_list_entry(it,  container, others);
 * }
 */
#define stl_list_foreach_r(list, elem) \
    for ((elem) = (list)->prev; (elem) != (list); (elem) = (elem)->prev)

/**
 *  Reverse iterator.
 *
 *  It is safe to delete items from the list while using
 *  this iterator.
 *
 *  container {
 *       stl_list others;
 * };
 *
 *  container *container; // User object
 *  stl_list *list;        // List pointer, should already be initialized.
 *  stl_list *tmp;         // Variable for loop, user should not use this.
 *  stl_list *it;          // Iterator
 *
 * stl_list_foreach_safe_r (list, tmp, it) {
 *       container = stl_list_entry(it,  container, others);
 *       stl_list_del(list, &container->others);
 * }
 */
#define stl_list_foreach_safe_r(list, n, elem)                        \
    for ((elem) = (list)->prev, (n) = (elem)->prev; (elem) != (list); \
         (elem) = (n), (n) = (elem)->prev)

#endif
