/*************************************************************************
  > File Name: stl_set.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wed 05 Apr 2023 12:56:04 PM HKT
 ************************************************************************/

#ifndef _STL_SET_H
#define _STL_SET_H
#include <stdio.h>
typedef unsigned int (*stl_set_entry_hash_func)(void *value);
typedef int (*stl_set_entry_equal_func)(void *value1, void *value2);
typedef void (*stl_set_entry_free_func)(void *value);
struct stl_set_entry
{
  void *data;
  struct stl_set_entry *next;
};

typedef struct stl_set_entry stl_set_entry;
typedef struct
{
  stl_set_entry **table;
  unsigned int entries;
  unsigned int table_size;
  unsigned int prime_index;
  stl_set_entry_hash_func hash_func;
  stl_set_entry_equal_func equal_func;
  stl_set_entry_free_func free_func;
} stl_set;
stl_set *stl_set_alloc(stl_set_entry_hash_func hash_func, stl_set_entry_equal_func equal_func);
int stl_set_remove(stl_set *set, void* data);
int stl_set_query(stl_set *set, void* data);
int stl_set_insert(stl_set *set, void *data);
stl_set *stl_set_union(stl_set *set1, stl_set *set2);
void **stl_set_to_array(stl_set *set);
void stl_set_free(stl_set *set);
inline void stl_set_register_free_func(stl_set *set, stl_set_entry_free_func free_func)
{
  if (set != NULL)
  {
    set->free_func = free_func;
  }
}

#endif
