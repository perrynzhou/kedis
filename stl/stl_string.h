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

#ifndef STL_STRING_H
#define STL_STRING_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#define stl_string_VERSION "1.0.0"

#ifdef STL_HAVE_CONFIG_H
    #include "config.h"
#else
    #define stl_string_malloc  malloc
    #define stl_string_realloc realloc
    #define stl_string_free    free
#endif

/**
 * length prefixed C strings, length is at the start of the allocated memory
 *  e.g :
 *  -----------------------------------------------
 *  | 0 | 0 | 0 | 4 | 'T' | 'E' | 'S' | 'T' | '\0'|
 *  -----------------------------------------------
 *                    ^
 *                  return
 *  User can keep pointer to first character, so it's like C style strings with
 *  additional functionality when it's used with these functions here.
 */

/**
 * @param str '\0' terminated C string, can be NULL.
 * @return    length prefixed string. NULL on out of memory or if 'str' is NULL.
 */
char *stl_string_new(const char *str);


/**
 * @param str \0' terminated C string, can be NULL.
 * @return    void value
 */
void stl_string_destroy(char *str);


/**
 * @param str string bytes, no need for '\0' termination.
 * @param len length of the 'str'.
 * @return    length prefixed string. NULL on out of memory or if 'str' is NULL.
 */
char *stl_string_new_len(const char *str, uint32_t len);

/**
 * @param fmt format
 * @param ... arguments
 * @return    length prefixed string. NULL on out of memory.
 */
char *stl_string_new_fmt(const char *fmt, ...);

/**
 * @param fmt format
 * @param va  va_list
 * @return    length prefixed string. NULL on out of memory.
 */
char *stl_string_new_va(const char *fmt, va_list va);

/**
 * Deallocate length prefixed string.
 * @param str length prefixed string. str may be NULL.
 */
void stl_string_destroy(char *str);

/**
 * @param str length prefixed string. NULL values are accepted.
 * @return    length of the string. If NULL, returns -1.
 */
int64_t stl_string_len(const char *str);

/**
 * @param str length prefixed string. NULL values are accepted.
 * @return    duplicate string. NULL on out of memory or if 'str' is NULL.
 */
char *stl_string_dup(const char *str);

/**
 * @param str    Pointer to length prefixed string, '*str' may change.
 * @param param  New value to set.
 * @return      'true' on success, 'false' on out of memory or if '*str' is NULL
 */
bool stl_string_set(char **str, const char *data);

/**
 * @param str pointer to length prefixed string, '*str' may change.
 * @param fmt format
 * @param ... arguments
 * @return    'true' on success, 'false' on out of memory
 */
bool stl_string_set_fmt(char **str, const char *fmt, ...);

/**
 * @param str  pointer to length prefixed string, '*str' may change.
 * @param text text to append.
 * @return    'true' on success, 'false' on out of memory or if '*str' is NULL.
 */
bool stl_string_append(char **str, const char *text);

/**
 * @param str pointer to length prefixed string. (char**).'*str' may change.
 * @param fmt format
 * @param ... arguments
 * @return    'true' on success, 'false' on out of memory or if '*str' is NULL.
 */
#define stl_string_append_fmt(str, fmt, ...)                                       \
    ((*str) ? (stl_string_set_fmt(str, "%s" fmt, *str, __VA_ARGS__)) :             \
              (stl_string_set_fmt(str, fmt, __VA_ARGS__)))

/**
 * Compare two length prefixed strings. To compare with C string, use strcmp().
 *
 * @param str   length prefixed string, must not be NULL.
 * @param other length prefixed string, must not be NULL.
 * @return      'true' if equals.
 */
bool stl_string_cmp(const char *str, const char *other);

/**
 * @param str  length prefixed string, '*str' may change
 * @param list character list to trim.
 * @return    'true' on success or if '*str' is NULL. 'false' on out of memory
 */
bool stl_string_trim(char **str, const char *list);

/**
 * @param str   length prefixed string, *str' may change.
 * @param start start index.
 * @param end   end index.
 * @return      'false' on out of range, on out of memory or if '*str' is NULL.
 *              'true' on success.
 */
bool stl_string_substring(char **str, uint32_t start, uint32_t end);

/**
 * @param str  length prefixed string, '*str' may change.
 * @param rep  string to be replaced
 * @param with string to replace with
 * @return    'true' on success or if '*str' is NULL.  'false' on out of memory
 */
bool stl_string_replace(char **str, const char *rep, const char *with);

/**
 * Tokenization is zero-copy but a bit tricky. This function will mutate 'str',
 * but it is temporary. On each 'stl_string_token_begin' call, this function will
 * place '\0' character at the end of a token and put delimiter at the end of
 * the 'str'.
 * e.g user1-user2\0 after first iteration will be user1\0user2-
 *
 * stl_string_token_end() will fix original string if necessary.
 *
 * usage:
 *
 * char *str = stl_string_new("user1-user2-user3");
 * char *save = NULL; // Must be initialized with NULL.
 * const char *token;
 *
 * while ((token = stl_string_token_begin(str, &save, "-") != NULL) {
 *      printf("token : %s \n", token);
 * }
 *
 * stl_string_token_end(str, &save);
 *
 *
 * @param str   length prefixed string, must not be NULL.
 * @param save  helper variable for tokenizer code.
 * @param delim delimiter list.
 * @return      token.
 */
 char *stl_string_token_begin(char *str, char **save, const char *delim);
void stl_string_token_end(char *str, char **save);


#endif
