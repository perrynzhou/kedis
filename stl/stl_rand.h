/*************************************************************************
    > File Name: stl_rand.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Mon 05 Sep 2022 11:10:42 PM EDT
 ************************************************************************/

#ifndef STL_RAND_H
#define STL_RAND_H

#include <stdint.h>
#include <stdio.h>

typedef struct
{
    unsigned char i;
    unsigned char j;
    unsigned char init[256];
} stl_rand;
void stl_rand_init(stl_rand *r, const unsigned char *init);
void stl_rand_read(stl_rand *r, void *buf, int size);

// update from redis
/**
 * @brief
 *
 * @return int32_t
 */
int32_t stl_rand_int32();
/**
 * @brief
 *
 * @param buf
 * @param n
 * @return int
 */
int stl_rand_str(char *buf, size_t n);
/**
 * @brief
 *
 * @param buf
 * @param n
 * @return int
 */
int stl_gen_uuid(char *buf, size_t n);
#endif
