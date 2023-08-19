/*************************************************************************
  > File Name: rpc_body.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wed 29 Mar 2023 07:10:20 PM HKT
 ************************************************************************/

#ifndef _RPC_BODY_H
#define _RPC_BODY_H
#include <stdio.h>
#define RPC_BODY_MSG_MAX_CAP 128
typedef struct
{
  int complete;
} rpc_body_complete;

typedef struct
{
  void *msg;
  size_t len;
  // int complete;
} rpc_body;

int rpc_body_init(rpc_body *req, void *msg, size_t len);
void rpc_body_reset(rpc_body *req, void *msg);
void rpc_body_deinit(rpc_body *req);
#endif
