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

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "stl_log.h"

#include <ctype.h>
#include <errno.h>
#include <time.h>

#ifndef thread_local
#if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
#define thread_local _Thread_local
#elif defined _WIN32 && (defined _MSC_VER || defined __ICL || \
                         defined __DMC__ || defined __BORLANDC__)
#define thread_local __declspec(thread)
#elif defined __GNUC__ || defined __SUNPRO_C || defined __xlC__
#define thread_local __thread
#else
#error "Cannot define  thread_local"
#endif
#endif

#if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_ATOMIC__
#define stl_atomic
#include <stdatomic.h>

#define stl_atomic _Atomic
#define stl_atomic_store(var, val) \
    (atomic_store_explicit(var, val, memory_order_relaxed))
#define stl_atomic_load(var) \
    (atomic_load_explicit(var, memory_order_relaxed))
#else
#define stl_atomic
#define stl_atomic_store(var, val) ((*(var)) = (val))
#define stl_atomic_load(var) (*(var))
#endif

thread_local char sc_name[32] = "Thread";

#if defined(_WIN32) || defined(_WIN64)

#pragma warning(disable : 4996)
#define strcasecmp _stricmp
#define localtime_r(a, b) (localtime_s(b, a) == 0 ? b : NULL)
#include <windows.h>

stl_log_mutex
{
    CRITICAL_SECTION mtx;
};

int stl_log_mutex_init(stl_log_mutex *mtx)
{
    InitializeCriticalSection(&mtx->mtx);
    return 0;
}

int stl_log_mutex_term(stl_log_mutex *mtx)
{
    DeleteCriticalSection(&mtx->mtx);
    return 0;
}

void stl_log_mutex_lock(stl_log_mutex *mtx)
{
    EnterCriticalSection(&mtx->mtx);
}

void stl_log_mutex_unlock(stl_log_mutex *mtx)
{
    LeaveCriticalSection(&mtx->mtx);
}

#else

#include <pthread.h>

typedef struct
{
    pthread_mutex_t mtx;
} stl_log_mutex;

int stl_log_mutex_init(stl_log_mutex *mtx)
{
    int rc;

    pthread_mutexattr_t attr;
    pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

    mtx->mtx = mut;

    rc = pthread_mutexattr_init(&attr);
    if (rc != 0)
    {
        return rc;
    }

    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    rc = pthread_mutex_init(&mtx->mtx, &attr);
    pthread_mutexattr_destroy(&attr);

    return rc;
}

void stl_log_mutex_term(stl_log_mutex *mtx)
{
    pthread_mutex_destroy(&mtx->mtx);
}

void stl_log_mutex_lock(stl_log_mutex *mtx)
{
    pthread_mutex_lock(&mtx->mtx);
}

void stl_log_mutex_unlock(stl_log_mutex *mtx)
{
    pthread_mutex_unlock(&mtx->mtx);
}

#endif

typedef struct
{
    FILE *fp;
    const char *current_file;
    const char *prev_file;
    size_t file_size;

    stl_log_mutex mtx;
    stl_atomic stl_log_level level;

    bool to_stdout;

    void *arg;
    int (*cb)(void *,  stl_log_level, const char *, va_list);
} stl_log;

stl_log s_log;

int stl_log_init(void)
{
    int rc;

    s_log = (stl_log){0};

    stl_atomic_store(&s_log.level, STL_LOG_INFO);
    s_log.to_stdout = true;

    rc = stl_log_mutex_init(&s_log.mtx);
    if (rc != 0)
    {
        errno = rc;
    }

    return rc;
}

int stl_log_term(void)
{
    int rc = 0;

    if (s_log.fp)
    {
        rc = fclose(s_log.fp);
        if (rc != 0)
        {
            rc = -1;
        }
    }

    stl_log_mutex_term(&s_log.mtx);

    return rc;
}

void stl_log_set_thread_name(const char *name)
{
    strncpy(sc_name, name, sizeof(sc_name) - 1);
}

static int stl_strcasecmp(const char *a, const char *b)
{
    int n;

    for (;; a++, b++)
    {
        if (*a == 0 && *b == 0)
        {
            return 0;
        }

        if ((n = tolower(*a) - tolower(*b)) != 0)
        {
            return n;
        }
    }
}

int stl_log_set_level(const char *str)
{
    size_t count = sizeof(stl_log_levels) / sizeof(stl_log_levels[0]);

    for (size_t i = 0; i < count; i++)
    {
        if (stl_strcasecmp(str, stl_log_levels[i].str) == 0)
        {
#ifdef stl_atomic
            stl_atomic_store(&s_log.level, stl_log_levels[i].id);
#else
            stl_log_mutex_lock(&s_log.mtx);
            s_log.level = stl_log_levels[i].id;
            stl_log_mutex_unlock(&s_log.mtx);
#endif
            return 0;
        }
    }

    return -1;
}

void stl_log_set_stdout(bool enable)
{
    stl_log_mutex_lock(&s_log.mtx);
    s_log.to_stdout = enable;
    stl_log_mutex_unlock(&s_log.mtx);
}

int stl_log_set_file(const char *prev_file, const char *current_file)
{
    int rc = 0, saved_errno = 0;
    long size;
    FILE *fp = NULL;

    stl_log_mutex_lock(&s_log.mtx);

    if (s_log.fp != NULL)
    {
        fclose(s_log.fp);
        s_log.fp = NULL;
    }

    s_log.prev_file = prev_file;
    s_log.current_file = current_file;

    if (prev_file == NULL || current_file == NULL)
    {
        goto out;
    }

    fp = fopen(s_log.current_file, "a+");
    if (fp == NULL || fprintf(fp, "\n") < 0 || (size = ftell(fp)) < 0)
    {
        goto error;
    }

    s_log.file_size = (size_t)size;
    s_log.fp = fp;

    goto out;

error:
    rc = -1;
    saved_errno = errno;

    if (fp != NULL)
    {
        fclose(fp);
    }
out:
    stl_log_mutex_unlock(&s_log.mtx);
    errno = saved_errno;

    return rc;
}

void stl_log_set_callback(void *arg, int (*cb)(void *,  stl_log_level,
                                               const char *, va_list))
{
    stl_log_mutex_lock(&s_log.mtx);
    s_log.arg = arg;
    s_log.cb = cb;
    stl_log_mutex_unlock(&s_log.mtx);
}

static int stl_log_print_header(FILE *fp, stl_log_level level)
{
    int rc;
    time_t t;
    struct tm result, *tm;

    t = time(NULL);
    tm = localtime_r(&t, &result);

    if (tm == NULL)
    {
        return -1;
    }

    rc = fprintf(fp, "[%d-%02d-%02d %02d:%02d:%02d][%-5s][%s] ",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
                 tm->tm_min, tm->tm_sec, stl_log_levels[level].str, sc_name);
    if (rc < 0)
    {
        return -1;
    }

    return 0;
}

static int stl_log_stdout(stl_log_level level, const char *fmt, va_list va)
{
    int rc;
    FILE *dest = level == STL_LOG_ERROR ? stderr : stdout;

    rc = stl_log_print_header(dest, level);
    if (rc < 0)
    {
        return -1;
    }

    rc = vfprintf(dest, fmt, va);
    if (rc < 0)
    {
        return -1;
    }

    return 0;
}

static int stl_log_file(stl_log_level level, const char *fmt, va_list va)
{
    int rc, size;

    rc = stl_log_print_header(s_log.fp, level);
    if (rc < 0)
    {
        return -1;
    }

    size = vfprintf(s_log.fp, fmt, va);
    if (size < 0)
    {
        return -1;
    }

    s_log.file_size += size;

    if (s_log.file_size > STL_LOG_FILE_SIZE)
    {
        fclose(s_log.fp);
        (void)rename(s_log.current_file, s_log.prev_file);

        s_log.fp = fopen(s_log.current_file, "w+");
        if (s_log.fp == NULL)
        {
            return -1;
        }

        s_log.file_size = 0;
    }

    return rc;
}

int stl_log_log(stl_log_level level, const char *fmt, ...)
{
    int rc = 0;
    va_list va;

    // Use relaxed atomics to avoid locking cost, e.g DEBUG logs when
    // level=INFO will get away without any synchronization on most platforms.
#ifdef stl_atomic
    stl_log_level curr;

    curr = stl_atomic_load(&s_log.level);
    if (level < curr)
    {
        return 0;
    }
#endif

    stl_log_mutex_lock(&s_log.mtx);

#ifndef stl_atomic
    if (level < s_log.level)
    {
        stl_log_mutex_unlock(&s_log.mtx);
        return 0;
    }
#endif

    if (s_log.to_stdout)
    {
        va_start(va, fmt);
        rc |= stl_log_stdout(level, fmt, va);
        va_end(va);
    }

    if (s_log.fp != NULL)
    {
        va_start(va, fmt);
        rc |= stl_log_file(level, fmt, va);
        va_end(va);
    }

    if (s_log.cb)
    {
        va_start(va, fmt);
        rc |= s_log.cb(s_log.arg, level, fmt, va);
        va_end(va);
    }

    stl_log_mutex_unlock(&s_log.mtx);

    return rc;
}
