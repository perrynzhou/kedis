/*************************************************************************
  > File Name: rpc_server.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Thu 23 Mar 2023 02:08:44 PM HKT
 ************************************************************************/

#ifndef _RPC_SERVER_H
#define _RPC_SERVER_H
#include "rpc_utils.h"
#include "rpc_body.h"
#include "../stl/stl.h"
#include "rpc_tcp_protocol.h"
#include <ucp/api/ucp.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// 1.must to handle msg
// 2.reset body to response to client
typedef int (*rpc_server_msg_handle)(void *ctx, rpc_body *in, rpc_body *out);

typedef struct
{
  ucp_context_h ucp_context;
  void **dispatch_connection_queue;
  ucp_worker_h ucp_worker;
  size_t ucp_worker_num;
  void *ucp_worker_threads;
  ucp_listener_h listener;
  rpc_communication_type rpc_type;
  uint64_t request_cnt;
  pthread_mutex_t lock;
  void *server_threads;
  rpc_tcp_err_cb err_cb;
  rpc_tcp_send_cb send_cb;
  rpc_tcp_recv_cb recv_cb;
} rpc_server;
int rpc_server_init(rpc_server *rserver, size_t worker_num, const char *rpc_communication, const char *server_addr);
int rpc_server_run(rpc_server *rserver);
void rpc_server_deinit(rpc_server *rserver);
void rpc_server_set_msg_handle(rpc_server *rserver, void *ctx, rpc_server_msg_handle msg_handle);
#endif
