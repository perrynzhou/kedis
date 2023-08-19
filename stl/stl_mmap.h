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

#ifndef STL_MMAP_H
#define STL_MMAP_H

#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>

#define stl_MMAP_VERSION "1.0.0"

#if defined(_WIN32)

#include <windows.h>
#include <memoryapi.h>

#define PROT_READ FILE_MAP_READ
#define PROT_WRITE FILE_MAP_WRITE

#define MAP_FIXED 0x01
#define MAP_ANONYMOUS 0x02
#define MAP_SHARED 0x04

#else /*POSIX*/

#include <sys/mman.h>

#endif

typedef struct
{
    int fd;
    unsigned char *ptr; // memory map start address
    size_t len;         // memory map length
    char err[128];
} stl_mmap;

/**
 * Initializes mmap ure, opens file and maps into memory. If file is
 * smaller than 'offset + len' and PROT_WRITE flag is provided, then the file
 * will be expanded to 'offset + len' bytes.
 *
 * @param m           mmap
 * @param name        file name
 * @param file_flags  flags for open(), e.g : O_RDWR | O_CREAT
 * @param prot        prot flags,       e.g : PROT_READ | PROT_WRITE
 * @param map_flags   mmap flags,       e.g : MAP_SHARED
 * @param offset      offset
 * @param len         len
 * @return            '0' on success, negative on failure,
 *                    call stl_mmap_err() for error string.
 */
int stl_mmap_init(stl_mmap *m, const char *name, int file_flags, int prot,
                  int map_flags, size_t offset, size_t len);
/**
 * @param m mmap
 * @return       '0' on success, '-1' on error, call stl_mmap_err() for details.
 */
int stl_mmap_term(stl_mmap *m);

/**
 * @param m      mmap
 * @param offset offset
 * @param len    len
 * @return      '0' on success, negative on failure,
 *              call stl_mmap_err() for error string.
 */
int stl_mmap_msync(stl_mmap *m, size_t offset, size_t len);

/**
 * @param m       mmap
 * @param offset  offset
 * @param len     len
 * @return        '0' on success, negative on failure,
 *                call stl_mmap_err() for error string.
 */
int stl_mmap_mlock(stl_mmap *m, size_t offset, size_t len);

/**
 * @param m       mmap
 * @param offset  offset
 * @param len     len
 * @return        '0' on success, negative on failure,
 *                call stl_mmap_err() for error string.
 */
int stl_mmap_munlock(stl_mmap *m, size_t offset, size_t len);

/**
 * @param m mmap
 * @return  last error string.
 */
const char *stl_mmap_err(stl_mmap *m);

#endif
