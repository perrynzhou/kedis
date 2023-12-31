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

#include "stl_queue.h"

#ifndef STL_SIZE_MAX
#define STL_SIZE_MAX SIZE_MAX
#endif

#define STL_MAX_CAP ((STL_SIZE_MAX - sizeof(stl_queue)) / 2ul)

static const stl_queue sc_empty = {.cap = 1, .first = 0, .last = 0};

static void *queue_alloc(void *prev, size_t elem_size, size_t *cap)
{
    size_t alloc, v = *cap;

    if (*cap > STL_MAX_CAP)
    {
        return NULL;
    }

    // Find next power of two.
    v = v < 4 ? 4 : v;
    v--;
    for (size_t i = 1; i < sizeof(v) * 8; i *= 2)
    {
        v |= v >> i;
    }
    v++;

    *cap = v;
    alloc = sizeof(stl_queue) + (elem_size * v);

    return stl_queue_realloc(prev, alloc);
}

bool stl_queue_init(void *q, size_t elem_size, size_t cap)
{
    size_t p = cap;
    void **ptr = q;
    stl_queue *meta;

    if (cap == 0)
    {
        *ptr = (void *)sc_empty.elems;
        return true;
    }

    meta = queue_alloc(NULL, elem_size, &p);
    if (meta == NULL)
    {
        *ptr = NULL;
        return false;
    }

    meta->cap = p;
    meta->first = 0;
    meta->last = 0;
    *ptr = meta->elems;

    return true;
}

void stl_queue_term(void *q)
{
    stl_queue *meta;
    void **ptr = q;

    if (*ptr == NULL)
    {
        return;
    }

    meta = stl_queue_meta(*ptr);

    if (meta != &sc_empty)
    {
        stl_queue_free(meta);
    }

    *ptr = NULL;
}

bool stl_queue_expand(void *q, size_t elem_size)
{
    void **ptr = q;
    stl_queue *tmp;
    stl_queue *meta = stl_queue_meta(*ptr);
    size_t cap, count, size;
    size_t pos = (meta->last + 1) & (meta->cap - 1);
    uint8_t *e;

    if (pos == meta->first)
    {
        if (meta == &sc_empty)
        {
            return stl_queue_init(ptr, elem_size, 4);
        }

        cap = meta->cap * 2;
        tmp = queue_alloc(meta, elem_size, &cap);
        if (tmp == NULL)
        {
            return false;
        }

        /**
         * Move items to make empty slots at the end.
         *
         * Doing a lot here to work with realloc, normally
         * 1 - allocating new memory
         * 2 - copy items
         * 3 - free old memory
         *
         * would be easier. But I use this code with a specific allocator,
         * it's much more efficient with realloc, so leaving this one as it is
         * for now.
         *
         * e.g :
         *               last    first
         *                |       |
         * Step 0 : | 2 | 3 | - | 1 |                  // tmp->cap : 4
         * Step 1 : | 2 | 3 | - | 1 | - | - | - | - |  // realloc
         * Step 2 : | 2 | 3 | - | 1 | 1 | - | - | - |  // memcpy
         * Step 3 : | 2 | 2 | 3 | 1 | 1 | - | - | - |  // memmove
         * Step 4 : | 1 | 2 | 3 | 1 | 1 | - | - | - |  // memcpy
         * Step 5 : | 1 | 2 | 3 | - | - | - | - | - |  // tmp->last = cap - 1;
         *           |       |
         *         first   last
         *
         */

        e = tmp->elems;
        count = tmp->cap - tmp->first;
        size = elem_size;

        memcpy(e + (size * tmp->cap), e + (size * tmp->first), count * size);
        memmove(e + (count * size), e, tmp->first * size);
        memcpy(e, e + (size * tmp->cap), count * size);

        tmp->last = tmp->cap - 1;
        tmp->first = 0;
        tmp->cap = cap;
        *ptr = tmp->elems;
    }

    return true;
}
