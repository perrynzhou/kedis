/*************************************************************************
  > File Name: rpc_client.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Thu 02 Mar 2023 06:52:26 AM HKT
 ************************************************************************/

#ifndef _RPC_CLIENT_H
#define _RPC_CLIENT_H
#include "rpc_utils.h"
#include "rpc_body.h"
#include "rpc_tcp_protocol.h"
#include <ucp/api/ucp.h>
typedef struct
{
  ucp_context_h ucp_context;
  ucp_worker_h ucp_worker;
  ucp_ep_h client_ep;
  rpc_communication_type rpc_communication;
  rpc_tcp_err_cb err_cb;
  rpc_tcp_send_cb send_cb;
  rpc_tcp_recv_cb recv_cb;
} rpc_client;

int rpc_client_init(rpc_client *rclient, const char *server_addr, const char *var_rpc_communication);

int rpc_client_send(rpc_client *rclient, rpc_body *in);
int rpc_client_recv(rpc_client *rclient, rpc_body *out);
void rpc_client_deinit(rpc_client *rclient);

#endif
