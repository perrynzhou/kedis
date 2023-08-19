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

#include "stl_string.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// avoid hard code
#define stl_string_VA_BUFFER_SIZE (1024)

/**
 * String with 'length' at the start of the allocated memory
 *  e.g :
 *  -----------------------------------------------
 *  | 0 | 0 | 0 | 4 | 'T' | 'E' | 'S' | 'T' | '\0'|
 *  -----------------------------------------------
 *
 *  User can keep pointer to first character, so it's like C style strings with
 *  additional functionality when it's used with these functions here.
 */
typedef struct
{
    uint32_t len;
    char buf[];
} stl_string;

#define stl_string_meta(str) \
    ((stl_string *)((char *)(str)-offsetof(stl_string, buf)))

#define stl_string_bytes(n) ((n) + sizeof(stl_string) + 1)

#ifndef stl_string_SIZE_MAX
#define stl_string_SIZE_MAX (UINT32_MAX - sizeof(stl_string) - 1)
#endif

char *stl_string_new(const char *str)
{
    size_t size;

    if (str == NULL || (size = strlen(str)) > stl_string_SIZE_MAX)
    {
        return NULL;
    }

    return stl_string_new_len(str, (uint32_t)size);
}

char *stl_string_new_len(const char *str, uint32_t len)
{
    stl_string *copy;

    if (str == NULL)
    {
        return NULL;
    }

    copy = stl_string_malloc(stl_string_bytes(len));
    if (copy == NULL)
    {
        return NULL;
    }

    memcpy(copy->buf, str, len);
    copy->buf[len] = '\0';
    copy->len = len;

    return copy->buf;
}

char *stl_string_new_va(const char *fmt, va_list va)
{
    int rc;
    char tmp[stl_string_VA_BUFFER_SIZE];
    stl_string *str;
    va_list args;

    va_copy(args, va);
    rc = vsnprintf(tmp, sizeof(tmp), fmt, args);
    if (rc < 0)
    {
        va_end(args);
        return NULL;
    }
    va_end(args);

    str = stl_string_malloc(stl_string_bytes(rc));
    if (str == NULL)
    {
        return NULL;
    }

    str->len = (uint32_t)rc;

    if (rc < (int)sizeof(tmp))
    {
        memcpy(str->buf, tmp, str->len + 1);
    }
    else
    {
        va_copy(args, va);
        rc = vsnprintf(str->buf, str->len + 1, fmt, args);
        va_end(args);

        if (rc < 0 || (uint32_t)rc > str->len)
        {
            stl_string_free(str);
            return NULL;
        }
    }

    return str->buf;
}

char *stl_string_new_fmt(const char *fmt, ...)
{
    char *str;
    va_list args;

    va_start(args, fmt);
    str = stl_string_new_va(fmt, args);
    va_end(args);

    return str;
}

void stl_string_destroy(char *str)
{
    if (str != NULL)
    {
        stl_string_free(stl_string_meta(str));
    }
}

int64_t stl_string_len(const char *str)
{
    if (str == NULL)
    {
        return -1;
    }

    return stl_string_meta(str)->len;
}

char *stl_string_dup(const char *str)
{
    if (str == NULL)
    {
        return NULL;
    }

    return stl_string_new_len(str, stl_string_meta(str)->len);
}

bool stl_string_set(char **str, const char *data)
{
    if (str != NULL && *str != NULL && data != NULL)
    {
        char *copy = stl_string_new(data);
        if (copy == NULL)
        {
            return false;
        }
        stl_string_destroy(*str);
        *str = copy;
        return true;
    }
    return false;
}

bool stl_string_set_fmt(char **str, const char *fmt, ...)
{
    char *ret;
    va_list args;

    va_start(args, fmt);
    ret = stl_string_new_va(fmt, args);
    va_end(args);

    if (ret != NULL)
    {
        stl_string_destroy(*str);
        *str = ret;
    }

    return ret != NULL;
}

bool stl_string_append(char **str, const char *text)
{
    size_t len, alloc;
    stl_string *meta;

    if (*str == NULL)
    {
        return (*str = stl_string_new(text)) != NULL;
    }

    meta = stl_string_meta(*str);
    len = strlen(text);
    alloc = stl_string_bytes(meta->len + len);

    if (len > stl_string_SIZE_MAX - meta->len ||
        (meta = stl_string_realloc(meta, alloc)) == NULL)
    {
        return false;
    }

    memcpy(&meta->buf[meta->len], text, len);
    meta->len += (uint32_t)len;
    meta->buf[meta->len] = '\0';
    *str = meta->buf;

    return true;
}

bool stl_string_cmp(const char *str, const char *other)
{
    stl_string *s1 = stl_string_meta(str);
    stl_string *s2 = stl_string_meta(other);

    return s1->len == s2->len && !memcmp(s1->buf, s2->buf, s1->len);
}

static void swap(char *str, char *d)
{
    char tmp;
    char *c = str + stl_string_meta(str)->len;

    tmp = *c;
    *c = *d;
    *d = tmp;
}

char *stl_string_token_begin(char *str, char **save, const char *delim)
{
    char *it = str;

    if (str == NULL)
    {
        return NULL;
    }

    if (*save != NULL)
    {
        it = *save;
        swap(str, it);
        if (*it == '\0')
        {
            return NULL;
        }
        it++;
    }

    *save = it + strcspn(it, delim);
    swap(str, *save);

    return it;
}

void stl_string_token_end(char *str, char **save)
{
    char *end;

    if (str == NULL)
    {
        return;
    }

    end = str + stl_string_meta(str)->len;
    if (*end == '\0')
    {
        return;
    }

    swap(str, *save);
}

bool stl_string_trim(char **str, const char *list)
{
    size_t len;
    char *start, *end;

    if (*str == NULL)
    {
        return true;
    }

    len = stl_string_meta(*str)->len;
    start = *str + strspn(*str, list);
    end = (*str) + len;

    while (end > start)
    {
        end--;
        if (!strchr(list, *end))
        {
            end++;
            break;
        }
    }

    if (start != *str || end != (*str) + len)
    {
        start = stl_string_new_len(start, (uint32_t)(end - start));
        if (start == NULL)
        {
            return false;
        }

        stl_string_destroy(*str);
        *str = start;
    }

    return true;
}

bool stl_string_substring(char **str, uint32_t start, uint32_t end)
{
    char *c;
    stl_string *meta;

    if (*str == NULL)
    {
        return false;
    }

    meta = stl_string_meta(*str);
    if (start > meta->len || end > meta->len || start > end)
    {
        return false;
    }

    c = stl_string_new_len(*str + start, end - start);
    if (c == NULL)
    {
        return false;
    }

    stl_string_destroy(*str);
    *str = c;

    return true;
}

bool stl_string_replace(char **str, const char *replace, const char *with)
{
    assert(replace != NULL && with != NULL);

    if (*str == NULL)
    {
        return true;
    }

    size_t replace_len = strlen(replace);
    size_t with_len = strlen(with);
    int64_t diff;
    size_t len_unmatch;
    size_t count, size;
    stl_string *dest;
    stl_string *meta = stl_string_meta(*str);
    char *orig = *str;
    char *orig_end = *str + meta->len;
    char *tmp;

    if (replace_len >= UINT32_MAX || with_len >= UINT32_MAX)
    {
        return false;
    }

    diff = (int64_t)with_len - (int64_t)replace_len;

    // Fast path, same size replacement.
    if (diff == 0)
    {
        while ((orig = strstr(orig, replace)) != NULL)
        {
            memcpy(orig, with, replace_len);
            orig += replace_len;
        }

        return true;
    }

    // Calculate new string size.
    tmp = orig;
    size = meta->len;
    for (count = 0; (tmp = strstr(tmp, replace)) != NULL; count++)
    {
        tmp += replace_len;
        // Check overflow.
        if ((int64_t)size > (int64_t)stl_string_SIZE_MAX - diff)
        {
            return false;
        }
        size += diff;
    }

    // No match.
    if (count == 0)
    {
        return true;
    }

    dest = stl_string_malloc(stl_string_bytes(size));
    if (!dest)
    {
        return false;
    }

    dest->len = (uint32_t)size;
    tmp = dest->buf;

    while (count--)
    {
        len_unmatch = strstr(orig, replace) - orig;
        memcpy(tmp, orig, len_unmatch);
        tmp += len_unmatch;

        memcpy(tmp, with, with_len);
        tmp += with_len;
        orig += len_unmatch + replace_len;
    }

    memcpy(tmp, orig, orig_end - orig + 1);

    stl_string_destroy(*str);
    *str = dest->buf;

    return true;
}
