/*
 * @Author: perrynzhou perrynzhou@gmail.com
 * @Date: 2023-02-14 06:06:02
 * @LastEditors: perrynzhou perrynzhou@gmail.com
 * @LastEditTime: 2023-04-06 13:58:34
 * @FilePath: /kirinfs/stl/dict/stl_dict.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#ifndef _STL_DICT_H
#define _STL_DICT_H
#include <stdint.h>
#include <stdio.h>
typedef uint32_t (*stl_dict_entry_hash)(const char *key, size_t key_len);
typedef void (*stl_dict_entry_handle)(void *val);

typedef struct
{
    uint64_t max_count;
    uint32_t count;
    uint32_t *member_count;
    void **members;
    stl_dict_entry_hash hash_fn;
    stl_dict_entry_handle handle_fn;
} stl_dict;

int stl_dict_init(stl_dict *d, uint32_t max_count);

stl_dict *stl_dict_create(uint32_t max_count);
void *stl_dict_put(stl_dict *d, char *key, void *val);
void *stl_dict_get(stl_dict *d, char *key);
int stl_dict_del(stl_dict *d, char *key);
void stl_dict_deinit(stl_dict *d);
void stl_dict_traverse(stl_dict *d);
void stl_dict_destroy(stl_dict *d);

inline void stl_dict_entry_hash_fn(stl_dict *d, stl_dict_entry_hash hash_fn)
{
    if (d != NULL)
    {
        d->hash_fn = hash_fn;
    }
}

inline void stl_dict_entry_handle_fn(stl_dict *d, stl_dict_entry_handle handle_fn)
{
    if (d != NULL)
    {
        d->handle_fn = handle_fn;
    }
}

#endif