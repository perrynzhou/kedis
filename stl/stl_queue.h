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

#ifndef stl_queue_H
#define stl_queue_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define STL_QUEUE_VERSION "1.0.0"

#ifdef STL_HAVE_CONFIG_H
#include "config.h"
#else
#define stl_queue_realloc realloc
#define stl_queue_free free
#endif

// Internals
typedef struct
{
    size_t cap;
    size_t first;
    size_t last;
    unsigned char elems[];
} stl_queue;

#define stl_queue_meta(q) \
    ((stl_queue *)((char *)(q)-offsetof(stl_queue, elems)))

static inline size_t stl_queue_inc_first(void *q)
{
    stl_queue *meta = stl_queue_meta(q);
    size_t tmp = meta->first;

    meta->first = (meta->first + 1) & (meta->cap - 1);
    return tmp;
}

static inline size_t stl_queue_inc_last(void *q)
{
    stl_queue *meta = stl_queue_meta(q);
    size_t tmp = meta->last;

    meta->last = (meta->last + 1) & (meta->cap - 1);
    return tmp;
}

static inline size_t stl_queue_dec_first(void *q)
{
    stl_queue *meta = stl_queue_meta(q);

    meta->first = (meta->first - 1) & (meta->cap - 1);
    return meta->first;
}

static inline size_t stl_queue_dec_last(void *q)
{
    stl_queue *meta = stl_queue_meta(q);

    meta->last = (meta->last - 1) & (meta->cap - 1);
    return meta->last;
}

bool stl_queue_init(void *q, size_t elem_size, size_t cap);
void stl_queue_term(void *q);
bool stl_queue_expand(void *q, size_t elem_size);

/**
 *   @param q     queue
 *   @param count initial capacity, '0' is accepted.
 *   @return      'true' on success, 'false' on out of memory.
 */
#define stl_queue_create(q, count) stl_queue_init(&(q), sizeof(*(q)), count)

/**
 *   Destroy queue
 *   @param q queue
 */
#define stl_queue_destroy(q) stl_queue_term((&(q)))

/**
 *   @param q queue
 *   @return  current capacity
 */
#define stl_queue_cap(q) (stl_queue_meta((q))->cap)

/**
 *   @param q queue
 *   @return  element count
 */
#define stl_queue_size(q)                                   \
    ((stl_queue_meta(q)->last - stl_queue_meta(q)->first) & \
     (stl_queue_meta(q)->cap - 1))

/**
 *   @param q queue
 *   @return true if queue is empty
 */
#define stl_queue_empty(q) ((stl_queue_meta(q)->last == stl_queue_meta(q)->first))

/**
 * Clear the queue without deallocating underlying memory.
 * @param q queue
 */
#define stl_queue_clear(q)            \
    do                                \
    {                                 \
        stl_queue_meta(q)->first = 0; \
        stl_queue_meta(q)->last = 0;  \
    } while (0)

/**
 * @param q queue
 * @return  index of the first element. If queue is empty, result is undefined.
 */
#define stl_queue_first(q) (stl_queue_meta(q)->first)

/**
 * @param q queue
 * @return  index of the last element. If queue is empty, result is undefined.
 */
#define stl_queue_last(q) (stl_queue_meta(q)->last)

/**
 * @return index of the next element after i, if i is the last element,
 *            result is undefined.
 */
#define stl_queue_next(q, i) (((i) + 1) & (stl_queue_meta(q)->cap - 1))

/**
 *   Returns element at index 'i', so regular loops are possible :
 *
 *   for (size_t i = 0; i < stl_queue_size(q); i++) {
 *        printf("%d" \n, stl_queue_at(q, i));
 *   }
 *
 *   @param q queue
 *   @return element at index i
 */
#define stl_queue_at(q, i) \
    (q)[((stl_queue_meta(q)->first) + (i)) & (stl_queue_cap(q) - 1)]

/**
 *   @param q queue
 *   @return  peek first element, if queue is empty, result is undefined
 */
#define stl_queue_peek_first(q) ((q)[stl_queue_meta(q)->first])

/**
 *   @param q queue
 *   @return  peek last element, if queue is empty, result is undefined
 */
#define stl_queue_peek_last(q) \
    (q)[(stl_queue_meta(q)->last - 1) & (stl_queue_meta(q)->cap - 1)]

/**
 * @param q    queue
 * @param elem elem to be added at the end of the list
 * @return     'true' on success, 'false' on out of memory.
 */
#define stl_queue_add_last(q, elem)                                                              \
    stl_queue_expand(&(q), sizeof(*(q))) == true        ? (q)[stl_queue_inc_last((q))] = (elem), \
                                                   true : false

/**
 * @param q queue
 * @return  delete the last element from the queue and return its value.
 *          If queue is empty, result is undefined.
 */
#define stl_queue_del_last(q) ((q)[stl_queue_dec_last((q))])

/**
 * @param q    queue.
 * @param elem elem to be added at the head of the list.
 * @return     'true' on success, 'false' on out of memory.
 */
#define stl_queue_add_first(q, elem)                                                              \
    stl_queue_expand(&(q), sizeof(*(q))) == true        ? (q)[stl_queue_dec_first((q))] = (elem), \
                                                   true : false

/**
 * @param q queue
 * @return  delete the first element from the queue and return its value.
 *          If queue is empty, result is undefined.
 */
#define stl_queue_del_first(q) (q)[stl_queue_inc_first((q))]

/**
 *  For each loop,
 *
 *  int *queue;
 *  stl_queue_create(queue, 4);
 *
 *  int elem;
 *  stl_queue_foreach(queue, elem) {
 *      printf("Elem : %d \n, elem);
 *  }
 */
#define stl_queue_foreach(q, elem)                                              \
    for (size_t _k = 1, _i = stl_queue_first(q); _k && _i != stl_queue_last(q); \
         _k = !_k, _i = stl_queue_next(q, _i))                                  \
        for ((elem) = (q)[_i]; _k; _k = !_k)

#endif
