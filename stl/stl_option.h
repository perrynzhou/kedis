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
#ifndef STL_OPTION_H
#define STL_OPTION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define STL_OPTION_VERSION "1.0.0"

typedef struct
{
    const char letter;
    const char *name;
} stl_option_item;

typedef struct
{
    stl_option_item *options;
    int count;
    char **argv;
} stl_option;

/**
 *
 * @param opt    Already initialized stl_opt
 * @param index  Index for argv
 * @param value  [out] Value for the option if exists. It should be after '='
 *               sign. E.g : -key=value or -k=value. If value does not exists
 *               (*value) will point to '\0' character. It won't be NULL itself.
 *
 *               To check if option has value associated : if (*value != '\0')
 *
 * @return       Letter for the option. If option is not known, '?' will be
 *               returned.
 */
char stl_option_at(stl_option *opt, int index, char **value);

#endif
