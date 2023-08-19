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

#ifndef stl_cond_H
#define stl_cond_H

#include <stdbool.h>

#define stl_cond_VERSION "1.0.0"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct
{
    bool done;
    void *data;

#if defined(_WIN32) || defined(_WIN64)
    CONDITION_VARIABLE cond;
    CRITICAL_SECTION mtx;
#else
    pthread_cond_t cond;
    pthread_mutex_t mtx;
#endif
} stl_cond;

/**
 * @param cond cond
 * @return     '0' on success, negative on error, errno will be set.
 */
int stl_cond_init(stl_cond *cond);

/**
 * @param cond cond
 * @return     '0' on success, negative on error, errno will be set.
 */
int stl_cond_term(stl_cond *cond);

/**
 * @param cond cond
 * @param data data to pass to thread which will call 'stl_cond_wait'.
 */
void stl_cond_signal(stl_cond *cond, void *data);

/**
 * @param cond cond
 * @return     'user data'.'data' argument on previous stl_cond_signal() call
 */
void *stl_cond_wait(stl_cond *cond);

#endif
