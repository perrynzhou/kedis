/*
 * @Author: perrynzhou perrynzhou@gmail.com
 * @Date: 2023-02-14 06:07:48
 * @LastEditors: perrynzhou perrynzhou@gmail.com
 * @LastEditTime: 2023-04-06 16:40:19
 * @FilePath: /kirinfs/stl/dict/stl_dict.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stl_dict.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DM_DELTA 0x9E3779B9
#define DM_FULLROUNDS 10
#define DM_PARTROUNDS 6
#if !defined(get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8) + (uint32_t)(((const uint8_t *)(d))[0]))
#endif
typedef struct stl_dict_entry
{
    struct stl_dict_entry *next;
    char *key;
    void *val;
} stl_dict_entry;
uint32_t stl_dict_default_hash_fn(const char *key, size_t key_length)
{
    uint32_t hash = 0, tmp;
    int rem;

    if (key_length <= 0 || key == NULL)
    {
        return 0;
    }

    rem = key_length & 3;
    key_length >>= 2;

    for (; key_length > 0; key_length--)
    {
        hash += get16bits(key);
        tmp = (get16bits(key + 2) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        key += 2 * sizeof(uint16_t);
        hash += hash >> 11;
    }

    switch (rem)
    {
    case 3:
        hash += get16bits(key);
        hash ^= hash << 16;
        hash ^= (uint32_t)key[sizeof(uint16_t)] << 18;
        hash += hash >> 11;
        break;

    case 2:
        hash += get16bits(key);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;

    case 1:
        hash += (unsigned char)(*key);
        hash ^= hash << 10;
        hash += hash >> 1;

    default:
        break;
    }

    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}
inline static stl_dict_entry *stl_dict_entry_alloc(const char *key, void *val)
{
    stl_dict_entry *de = (stl_dict_entry *)calloc(1, sizeof(stl_dict_entry));
    de->next = NULL;
    size_t len = strlen(key);

    de->key = calloc(len, sizeof(char));
    memcpy(de->key, key, len);
    de->key[len] = '\0';
    de->val = val;
    return de;
}
inline static void stl_dict_entry_destroy(stl_dict_entry *de)
{
    if (de != NULL)
    {
        free(de->key);
        free(de);
        de = NULL;
    }
}

static uint64_t stl_dict_jump_consistent(uint64_t key, uint32_t num_buckets)
{

    int64_t b = -1, j = 0;
    uint32_t value = 0;

    while (j < num_buckets)
    {
        b = j;
        key = key * 2862933555777941757ULL + 1;
        j = (int64_t)((b + 1) * ((double)(1LL << 31) / (double)((key >> 33) + 1)));
    }
    value = (b < 0) ? (~b + 1) : b;
    return value;
}

int stl_dict_init(stl_dict *d, uint32_t max_count)
{
    memset(d, 0, sizeof(*d));
    d->max_count = max_count;
    d->count = 0;
    d->hash_fn = stl_dict_default_hash_fn;
    d->members = (void *)calloc(max_count, sizeof(void *));
    d->member_count = (uint32_t *)calloc(max_count, sizeof(uint32_t));
    return 0;
}
stl_dict *stl_dict_create(uint32_t max_count)
{
    stl_dict *d = (stl_dict *)calloc(1, sizeof(stl_dict));
    if (stl_dict_init(d, max_count) != 0)
    {
        free(d);
        d = NULL;
    }
    return d;
}
static stl_dict_entry *stl_dict_search(stl_dict *d, const char *key, uint32_t *index_ptr, stl_dict_entry **prev)
{

    size_t key_len = strlen(key);
    uint32_t hash = d->hash_fn(key, key_len);
    uint32_t index = stl_dict_jump_consistent(hash, d->max_count);

    *index_ptr = index;
    stl_dict_entry *cur = NULL;
    stl_dict_entry *data = NULL;
    if (d->member_count[index] > 0)
    {
        for (cur = (stl_dict_entry *)d->members[index]; cur != NULL; cur = cur->next)
        {
            if (strncmp(cur->key, key, key_len) == 0)
            {
                data = cur;
                break;
            }
            *prev = cur;
        }
    }
    return data;
}
void *stl_dict_put(stl_dict *d, char *key, void *val)
{

    if (d->hash_fn != NULL)
    {
        uint32_t index = 0;
        stl_dict_entry *prev = NULL;

        stl_dict_entry *data = stl_dict_search(d, key, &index, &prev);
        if (data != NULL)
        {
            return NULL;
        }
        data = stl_dict_entry_alloc(key, val);
        if (d->members[index] == NULL)
        {
            d->members[index] = data;
        }
        else
        {
            data->next = d->members[index];
            d->members[index] = data;
        }
        __sync_fetch_and_add(&d->member_count[index], 1);
        __sync_fetch_and_add(&d->count, 1);
        return (void *)&data->val;
    }
    return NULL;
}
void *stl_dict_get(stl_dict *d, char *key)
{
    void *val = NULL;
    uint32_t index = 0;
    stl_dict_entry *prev = NULL;
    stl_dict_entry *de = stl_dict_search(d, key, &index, &prev);
    if (de != NULL)
    {
        val = de->val;
    }
    return val;
}
int stl_dict_del(stl_dict *d, char *key)
{
    stl_dict_entry *prev = NULL;
    uint32_t index = 0;
    stl_dict_entry *de = stl_dict_search(d, key, &index, &prev);
    if (de != NULL)
    {
        if (prev == NULL)
        {
            d->members[index] = de->next;
        }
        else
        {
            prev->next = de->next;
        }
        de->next = NULL;
        free(de->key);
        free(de);
        __sync_fetch_and_sub(&d->member_count[index], 1);
        __sync_fetch_and_sub(&d->count, 1);
        return 0;
    }
    return -1;
}
void stl_dict_traverse(stl_dict *d)
{
    if (d != NULL && d->handle_fn != NULL)
    {
        size_t i = 0;
        for (; i < d->max_count; i++)
        {
            if (d->member_count[i] > 0)
            {
                stl_dict_entry *de = (stl_dict_entry *)d->members[i];
                while (de != NULL)
                {
                    void *data = de->val;
                    d->handle_fn(data);
                    de = de->next;
                }
            }
        }
    }
}
void stl_dict_deinit(stl_dict *d)
{
    if (d != NULL && d->count > 0)
    {
        size_t i;
        for (i = 0; i < d->max_count; i++)
        {
            if (d->member_count[i] > 0)
            {
                stl_dict_entry *de = (stl_dict_entry *)d->members[i];
                while (de != NULL)
                {
                    stl_dict_entry *next = de->next;
                    free(de->key);
                    free(de);
                    de = next;
                    __sync_fetch_and_sub(&d->member_count[i], 1);
                    __sync_fetch_and_sub(&d->count, 1);
                }
            }
        }
    }
}
void stl_dict_destroy(stl_dict *d)
{
    if (d != NULL)
    {
        stl_dict_deinit(d);
        free(d);
        d = NULL;
    }
}
extern inline void stl_dict_entry_handle_fn(stl_dict *d, stl_dict_entry_handle handle_fn);
extern inline void stl_dict_entry_hash_fn(stl_dict *d, stl_dict_entry_hash hash_fn);