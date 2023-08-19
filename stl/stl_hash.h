/*************************************************************************
    > File Name: stl_hash.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Tue 22 Mar 2022 08:41:36 AM UTC
 ************************************************************************/

#ifndef STL_HASH_H
#define STL_HASH_H

#include <stdint.h>
#include <stdio.h>

#define STL_HASH_VERSION "1.1.0"
/**
 * @brief 
 * 
 * @param key 
 * @param key_length 
 * @return uint32_t 
 */
uint32_t stl_hash_crc16(const char* key, size_t key_length);
/**
 * @brief 
 * 
 * @param key 
 * @param key_length 
 * @return uint32_t 
 */
uint32_t stl_hash_crc32(const char *key, size_t key_length);
/**
 * @brief 
 * 
 * @param key 
 * @param key_length 
 * @return uint32_t 
 */
uint32_t stl_hash_crc32a(const char *key, size_t key_length);
/**
 * @brief 
 * 
 * @param key 
 * @param key_length 
 * @return uint32_t 
 */
uint32_t stl_hash_fnv1_64(const char *key, size_t key_length);
/**
 * @brief 
 * 
 * @param key 
 * @param key_length 
 * @return uint32_t 
 */
uint32_t stl_hash_fnv1a_64(const char *key, size_t key_length);
/**
 * @brief 
 * 
 * @param key 
 * @param key_length 
 * @return uint32_t 
 */
uint32_t stl_hash_fnv1_32(const char *key, size_t key_length);
/**
 * @brief 
 * 
 * @param key 
 * @param key_length 
 * @return uint32_t 
 */
uint32_t stl_hash_fnv1a_32(const char *key, size_t key_length);
/**
 * @brief 
 * 
 * @param key 
 * @param key_length 
 * @return uint32_t 
 */
uint32_t stl_hash_one_at_a_time(const char *key, size_t key_length);
/**
 * @brief 
 * 
 * @param key 
 * @param num_buckets 
 * @return uint32_t 
 */
uint32_t stl_hash_jump_consistent(uint64_t key, int32_t num_buckets);
#endif
