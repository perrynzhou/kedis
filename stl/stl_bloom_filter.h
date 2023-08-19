/*************************************************************************
  > File Name: stl_bloom_filter.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wed 05 Apr 2023 10:32:08 AM HKT
 ************************************************************************/

#ifndef _STL_BLOOM_FILTER_H
#define _STL_BLOOM_FILTER_H
#include <stdint.h>
typedef unsigned int (*stl_bf_hashf_unc)(void *data);

typedef struct
{
  stl_bf_hashf_unc hash_func;
  uint8_t *table;
  uint32_t table_size;
  uint32_t num_functions;
} stl_bloom_filter;
stl_bloom_filter *stl_bloom_filter_new(uint32_t table_size,
                                       stl_bf_hashf_unc hash_func,
                                       uint32_t num_functions);
void stl_bloom_filter_insert(stl_bloom_filter *bloomfilter, void *value);
int stl_bloom_filter_query(stl_bloom_filter *bloomfilter, void *value);
stl_bloom_filter *stl_bloom_filter_intersection(stl_bloom_filter *filter1,
                                                stl_bloom_filter *filter2);
void stl_bloom_filter_free(stl_bloom_filter *bloomfilter);
#endif
