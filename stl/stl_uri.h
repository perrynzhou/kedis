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
#ifndef stl_uri_H
#define stl_uri_H

#define stl_uri_VERSION "1.0.0"

#ifdef SC_HAVE_CONFIG_H
#include "config.h"
#else
#define stl_uri_malloc malloc
#define stl_uri_free free
#endif

#include <stdbool.h>
#include <stdlib.h>

/**
 * Based on RFC 3986
 *
 *    The following is a example URIs and their component parts:
 *
 *      foo://user:password@example.com:8042/over/there?name=ferret#nose
 *      \_/   \____________________________/\_________/ \_________/ \__/
 *       |                |                     |            |       |
 *     scheme        authority                path        query   fragment
 *
 *
 *      user:password@example.com:8042
 *      \__________/ \_________/ \__/
 *           |            |       |
 *       userinfo       host     port
 * --------------------------------------------------------------------
 *
 */

typedef struct
{
    const char *str; // Full string
    const char *scheme;
    const char *host;
    const char *userinfo;
    const char *port;
    const char *path;
    const char *query;
    const char *fragment;

    char buf[];
} stl_uri;

/**
 * Parse uri.
 *
 * Internally, it does a single allocation but each part is also represented as
 * NULL ended string.
 *
 * E.g :
 *
 *  stl_uri* uri;
 *
 *  stl_uri* uri;
 * uri = stl_uri_create("http://user:pass@any.com:8042/over/there?name=jane#doe");
 *
 * printf("%s \n", uri->str);       // prints full string.
 * printf("%s \n", uri->scheme);    // prints "http"
 * printf("%s \n", uri->host);      // prints "any.com"
 * printf("%s \n", uri->userinfo);  // prints "user:pass"
 * printf("%s \n", uri->port);      // prints "8042"
 * printf("%s \n", uri->path);      // prints "/over/there"
 * printf("%s \n", uri->query);     // prints "name=jane"
 * printf("%s \n", uri->fragment);  // prints "doe"
 *
 *
 * @param str uri string
 * @return    uri, NULL on error
 */
stl_uri *stl_uri_create(const char *str);

/**
 * @param uri uri
 */
void stl_uri_destroy(stl_uri *uri);

#endif
