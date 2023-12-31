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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "stl_perf.h"

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <unistd.h>

#define ITEMS_SIZE (sizeof(stl_perf_hw) / sizeof(stl_perf_event))

static int initialized = 0;
static int running = 0;
static uint64_t total = 0;
static uint64_t start = 0;

typedef struct
{
    stl_perf_event event;
    double value;
    double active;
    int fd;
} stl_perf_item;

static stl_perf_item stl_perf_items[ITEMS_SIZE];

#define stl_perf_assert(val)                                     \
    do                                                           \
    {                                                            \
        if (!(val))                                              \
        {                                                        \
            fprintf(stderr, "%s:%d: error", __FILE__, __LINE__); \
            if (errno)                                           \
            {                                                    \
                fprintf(stderr, " (%s)", strerror(errno));       \
            }                                                    \
            abort();                                             \
        }                                                        \
    } while (0)

static void stl_perf_set(stl_perf_item *items, size_t size)
{
    const uint64_t flags =
        PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
    int fd;

    for (size_t i = 0; i < size; i++)
    {
       struct  perf_event_attr p = {.size = sizeof(struct perf_event_attr),
                             .read_format = flags,
                             .type = items[i].event.type,
                             .config = items[i].event.config,
                             .disabled = 1,
                             .inherit = 1,
                             .inherit_stat = 0,
                             .exclude_kernel = false,
                             .exclude_hv = false};

        fd = syscall(__NR_perf_event_open, &p, 0, -1, -1, PERF_FLAG_FD_CLOEXEC);
        if (fd == -1)
        {
            fprintf(stderr,
                    "Failed to set counter : %s , probably your system does "
                    "not support it! \n",
                    items[i].event.name);
            abort();
        }

        items[i].fd = fd;
    }
}

static void stl_read(stl_perf_item *items, size_t size)
{
    struct read_format
    {
        uint64_t value;
        uint64_t time_enabled;
        uint64_t time_running;
    } fmt;

    for (size_t i = 0; i < size; i++)
    {
        double n = 1.0;

        stl_perf_assert(read(items[i].fd, &fmt, sizeof(fmt)) == sizeof(fmt));

        if (fmt.time_enabled > 0 && fmt.time_running > 0)
        {
            n = (double)fmt.time_running / (double)fmt.time_enabled;
            items[i].active = n;
        }

        items[i].value += fmt.value * n;
    }
}

static void stl_perf_clear(void)
{
    total = 0;
    start = 0;
    running = 0;
    initialized = 0;

    for (size_t i = 0; i < ITEMS_SIZE; i++)
    {
        stl_perf_items[i].event = stl_perf_hw[i];
        stl_perf_items[i].value = 0;
        stl_perf_items[i].active = 0;
        stl_perf_items[i].fd = -1;
    }
}

static uint64_t sy_time_nano(void)
{
    int rc;
    struct timespec ts;

    rc = clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    if (rc == -1)
    {
        abort();
    }

    return ((uint64_t)(ts.tv_nsec + (ts.tv_sec * 1000 * 1000 * 1000)));
}

void stl_perf_start(void)
{
    if (!initialized)
    {
        stl_perf_clear();
        stl_perf_set(stl_perf_items, ITEMS_SIZE);
        initialized = 1;
    }

    stl_perf_assert(prctl(PR_TASK_PERF_EVENTS_ENABLE) == 0);

    start = sy_time_nano();
    running = 1;
}

void stl_perf_pause(void)
{
    stl_perf_assert(initialized);

    if (!running)
    {
        return;
    }

    stl_perf_assert(prctl(PR_TASK_PERF_EVENTS_DISABLE) == 0);

    total += sy_time_nano() - start;
    running = 0;
}

void stl_perf_end(void)
{
    stl_perf_assert(initialized);

    stl_perf_pause();
    stl_read(stl_perf_items, ITEMS_SIZE);

    for (size_t i = 0; i < ITEMS_SIZE; i++)
    {
        close(stl_perf_items[i].fd);
    }

    printf("\n| %-25s | %-18s | %s  \n", "Event", "Value", "Measurement time");
    printf("---------------------------------------------------------------\n");
    printf("| %-25s | %-18.2f | %s  \n", "time (seconds)",
           ((double)total / 1e9), "(100,00%)");

    for (size_t i = 0; i < ITEMS_SIZE; i++)
    {
        printf("| %-25s | %-18.2f | (%.2f%%)  \n", stl_perf_items[i].event.name,
               stl_perf_items[i].value, stl_perf_items[i].active * 100);
    }

    stl_perf_clear();
}
