/*************************************************************************
  > File Name: stl_md5.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Thu 04 Aug 2022 10:50:07 AM CST
 ************************************************************************/

#ifndef STL_MD5_H
#define STL_MD5_H
#include <stdint.h>
#include <stdio.h>
#define STL_MD5_BUFFER_SIZE 64

#define STL_MD5_VERSION "1.1.0"

typedef  struct 
{
  uint64_t bytes;
  uint32_t a, b, c, d;
  unsigned char buffer[STL_MD5_BUFFER_SIZE];
}stl_md5;
/**
 * @brief 
 * 
 * @return  stl_md5* 
 */
 stl_md5 *stl_md5_alloc();
void stl_md5_init( stl_md5 *ctx);
/**
 * @brief 
 * 
 * @param ctx 
 */
void stl_md5_init( stl_md5 *ctx);
/**
 * @brief 
 * 
 * @param ctx 
 * @param data 
 * @param size 
 */
void stl_md5_compute( stl_md5 *ctx, const void *data, size_t size);
/**
 * @brief 
 * 
 * @param ctx 
 * @param result 
 */
void stl_md5_checksum( stl_md5 *ctx, char result[16]);
/**
 * @brief 
 * 
 * @param ctx 
 */
void stl_md5_destroy( stl_md5 *ctx);
#endif