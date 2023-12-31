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
#ifndef stl_thread_H
#define stl_thread_H

#define stl_thread_VERSION "1.0.0"

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

stl_thread
{
    HANDLE id;
    void *(*fn)(void *);
    void *arg;
    void *ret;
    char err[64];
};

#else

#include <pthread.h>

typedef struct
{
    pthread_t id;
    char err[128];
} stl_thread;

#endif

/**
 * @param thread thread
 */
void stl_thread_init(stl_thread *thread);

/**
 * @param thread thread
 * @return       '0' on success,
 *               '-1' on error, call 'stl_thread_err()' for error string.
 */
int stl_thread_term(stl_thread *thread);

/**
 * @param thread thread
 * @return       last error message
 */
const char *stl_thread_err(stl_thread *thread);

/**
 * @param thread thread
 * @return       '0' on success,
 *               '-1' on error, call 'stl_thread_err()' for error string.
 */
int stl_thread_start(stl_thread *thread, void *(*fn)(void *), void *arg);

/**
 * @param thread thread
 * @return       '0' on success,
 *               '-1' on error, call 'stl_thread_err()' for error string.
 */
int stl_thread_join(stl_thread *thread, void **ret);

#endif
