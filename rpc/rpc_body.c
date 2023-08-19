/*************************************************************************
  > File Name: rpc_body.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wed 29 Mar 2023 07:10:26 PM HKT
 ************************************************************************/

#include "rpc_body.h"
#include "rpc_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int rpc_body_init(rpc_body *req, void *msg, size_t len)
{
  if (req != NULL)
  {
   
    req->msg = calloc(len, sizeof(char));
    if (msg != NULL)
    {
      memcpy(req->msg, msg, len);
    }
    req->len = len;
    return 0;
  }
  return -1;
}
void rpc_body_deinit(rpc_body *req)
{
  if (req != NULL)
  {
    free(req->msg);
  }
}
void rpc_body_reset(rpc_body *req, void *msg)
{
  if (req != NULL && req->msg != NULL)
  {
    if (msg != NULL)
    {
      memcpy(req->msg, msg, req->len);
      return;
    }
    memset(req->msg, 0, req->len);
  }
}