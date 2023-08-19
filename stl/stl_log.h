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

#ifndef STL_LOG_H
#define STL_LOG_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STL_LOG_VERSION "1.0.0"

typedef enum
{

    STL_LOG_DEBUG,
    STL_LOG_INFO,
    STL_LOG_WARN,
    STL_LOG_ERROR,
    STL_LOG_OFF,
} stl_log_level;

// clang-format off

typedef struct {
   stl_log_level id;
 char *str;
}stl_log_level_pair;
static const stl_log_level_pair stl_log_levels[] = {
        {STL_LOG_DEBUG, "DEBUG"},
        {STL_LOG_INFO,  "INFO" },
        {STL_LOG_WARN,  "WARN" },
        {STL_LOG_ERROR, "ERROR"},
        {STL_LOG_OFF,   "OFF"  },
};
// clang-format on

// Internal function
int stl_log_log(stl_log_level level, const char *fmt, ...);

// Max file size to rotate, should not be more than 4 GB.
#define STL_LOG_FILE_SIZE (2 * 1024 * 1024)

// Define stl_log_PRINT_FILE_NAME to print file name and line no in the log line.
#ifdef STL_LOG_PRINT_FILE_NAME
#define stl_log_ap(fmt, ...) \
    "(%s:%d) " fmt, strrchr("/" __FILE__, '/') + 1, __LINE__, __VA_ARGS__
#else
#define stl_log_ap(fmt, ...) fmt, __VA_ARGS__
#endif

/**
 * stl_log_init() call once in your application before using log functions.
 * stl_log_term() call once to clean up, you must not use logger after this call.
 * These functions are not thread-safe, should be called from a single thread.
 *
 * @return '0' on success, negative value on error, errno will be set.
 */
int stl_log_init(void);
int stl_log_term(void);

/**
 * Call once from each thread if you want to set thread name.
 * @param name  Thread name
 */
void stl_log_set_thread_name(const char *name);

/**
 * @param level_str  One of "DEBUG", "INFO", "WARN", "ERROR", "OFF"
 * @return           '0' on success, negative value on invalid level string
 */
int stl_log_set_level(const char *level_str);

/**
 * @param enable 'true' to enable, 'false' to disable logging to stdout.
 */
void stl_log_set_stdout(bool enable);

/**
 * Log files will be rotated. Latest logs will always be in the 'current_file'.
 * e.g stl_log_set_file("/tmp/log.0.txt", "/tmp/log-latest.txt");
 *     To disable logging into file : stl_log_set_file(NULL, NULL);
 *
 * @param prev_file     file path for previous log file, 'NULL' to disable
 * @param current_file  file path for latest log file, 'NULL' to disable
 * @return              0 on success, -1 on error, errno will be set.
 */
int stl_log_set_file(const char *prev_file, const char *current_file);

/**
 * @param arg user arg to callback.
 * @param cb  log callback.
 */
void stl_log_set_callback(void *arg,
                          int (*cb)(void *arg, stl_log_level level,
                                    const char *fmt, va_list va));

// e.g : stl_log_error("Errno : %d, reason : %s", errno, strerror(errno));
#define stl_log_debug(...) (stl_log_log(STL_LOG_DEBUG, stl_log_ap(__VA_ARGS__, "")))
#define stl_log_info(...) (stl_log_log(STL_LOG_INFO, stl_log_ap(__VA_ARGS__, "")))
#define stl_log_warn(...) (stl_log_log(STL_LOG_WARN, stl_log_ap(__VA_ARGS__, "")))
#define stl_log_error(...) (stl_log_log(STL_LOG_ERROR, stl_log_ap(__VA_ARGS__, "")))

#endif
