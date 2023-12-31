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

#ifndef stl_timer_H
#define stl_timer_H

#define stl_timer_VERSION "1.0.0"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef SC_HAVE_CONFIG_H
#include "config.h"
#else
#define stl_timer_malloc malloc
#define stl_timer_free free
#endif

#define stl_timer_INVALID UINT64_MAX

typedef struct
{
    uint64_t timeout;
    uint64_t type;
    void *data;
} stl_timer_data;

typedef struct
{
    uint64_t timestamp;
    uint32_t head;
    uint32_t wheel;
    uint32_t count;
    stl_timer_data *list;
} stl_timer;

/**
 * Init timer
 *
 * @param timer     timer
 * @param timestamp current timestamp. Use monotonic timer source.
 * @return          'false' on out of memory.
 */
bool stl_timer_init(stl_timer *timer, uint64_t timestamp);

/**
 * Destroy timer.
 * @param timer timer
 */
void stl_timer_term(stl_timer *timer);

/**
 * Remove all timers without deallocating underlying memory.
 * @param timer
 */
void stl_timer_clear(stl_timer *timer);

/**
 * Add timer
 * 'timeout' is relative to latest 'timestamp' value given to the 'timer'.
 *
 * e.g stl_timer_init(&timer, 1000); // Current timestamp is 1000.
 *     stl_timer_add(&timer, arg, 10); // Timeout will be at 1010.
 *     stl_timer_timeout(&timer, 2000, arg, callback); // Timestamp is now 2000.
 *     stl_timer_add(&timer, arg, 10); // Timeout will be at 2010.
 *
 *
 * @param timer   timer
 * @param timeout timeout value, this is relative to 'sc_timer_init's timer.
 *                e.g stl_timer_init(&timer, 10); // say, start time is 10
 * milliseconds
 * @param data    user data to pass into callback on 'sc_timer_timeout' call.
 * @param type    user data to pass into callback on 'sc_timer_timeout' call.
 * @return        stl_timer_INVALID on out of memory. Otherwise, timer id. You
 *                can cancel this timer via this id later.
 */
uint64_t stl_timer_add(stl_timer *timer, uint64_t timeout, uint64_t type,
                       void *data);

/**
 * uint64_t id = stl_timer_add(&timer, arg, 10);
 * stl_timer_cancel(&timer, &id);
 *
 * @param timer timer
 * @param id    timer id
 */
void stl_timer_cancel(stl_timer *timer, uint64_t *id);

/**
 * Checks timeouts and calls 'callback' function for each timeout.
 *
 * Logical pattern is :
 *
 * e.g:
 *  stl_timer timer;
 * stl_timer_init(&timer, time_ms());
 * stl_timer_add(&timer, data, 100);
 *
 * while (true) {
 *      uint64_t timeout = stl_timer_timeout(&timer, time_ms(), arg, callback);
 *      sleep(timeout); // or select(timeout), epoll_wait(timeout) etc..
 * }
 *
 *
 * @param timer     timer
 * @param timestamp current timestamp
 * @param arg       user data to user callback
 * @param callback  'arg' is user data.
 *                  'timeout' is scheduled timeout for that timer.
 *                  'type' is what user passed on 'sc_timer_add'.
 *                  'data' is what user passed on 'sc_timer_add'.
 * @return          next timeout.
 */
uint64_t stl_timer_timeout(stl_timer *timer, uint64_t timestamp, void *arg,
                           void (*callback)(void *arg, uint64_t timeout,
                                            uint64_t type, void *data));
#endif
