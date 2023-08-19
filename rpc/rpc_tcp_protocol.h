/*************************************************************************
  > File Name: rpc_tcp_protocol.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Fri 31 Mar 2023 07:07:18 PM HKT
 ************************************************************************/

#ifndef _RPC_TCP_PROTOCOL_H
#define _RPC_TCP_PROTOCOL_H
#include "rpc_body.h"
#include <ucp/api/ucp.h>

typedef void (*rpc_tcp_send_cb)(void *request, ucs_status_t status, void *user_data);
typedef void (*rpc_tcp_recv_cb)(void *request, ucs_status_t status, size_t length,void *user_data);
typedef void (*rpc_tcp_err_cb)(void *arg, ucp_ep_h ep, ucs_status_t status);

int rpc_tcp_send(ucp_worker_h ucp_worker, ucp_ep_h ep, rpc_body *data,rpc_tcp_send_cb send_cb);
ucs_status_t rpc_tcp_recv(ucp_worker_h ucp_worker, ucp_ep_h ep, rpc_body *data,rpc_tcp_recv_cb recv_cb);
ucs_status_t rpc_wait_complete(ucp_worker_h ucp_worker, void *request, rpc_body_complete *ctx);

#endif
