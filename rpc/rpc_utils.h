/*************************************************************************
  > File Name: rpc_utils.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wed 29 Mar 2023 06:59:30 PM HKT
 ************************************************************************/

#ifndef _RPC_UTILS_H
#define _RPC_UTILS_H
#include <ucp/api/ucp.h>
#include <stddef.h>

typedef enum
{
  RPC_TCPV4_TYPE = 0,
  RPC_TCPV6_TYPE = 1,
  RPC_RDMA_TYPE = 2,

} rpc_communication_type;
typedef void (*rpc_err_cb)(void *arg, ucp_ep_h ep, ucs_status_t status);
int rpc_get_communication_type(const char *rpc_communication);
ucs_status_t rpc_create_server_endpoint(ucp_worker_h data_worker,ucp_conn_request_h conn_request,ucp_ep_h *server_ep, rpc_err_cb err_cb,void *arg);
ucs_status_t rpc_create_client_endpoint(rpc_communication_type communication_type, char *server_addr, int port, ucp_worker_h ucp_worker, ucp_ep_h *client_ep, rpc_err_cb err_cb,void *arg);
void rpc_close_endpoint(ucp_worker_h ucp_worker, ucp_ep_h ep, uint64_t flags);
int rpc_init_worker(ucp_context_h ucp_context, ucp_worker_h *ucp_worker);
int rpc_set_tcp_family(sa_family_t *ai_family, rpc_communication_type rpc_type);
int rpc_thread_setaffinity(pthread_t pid,size_t thd_idx);
#endif
