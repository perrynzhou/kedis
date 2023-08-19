/*************************************************************************
  > File Name: rpc_utils.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wed 29 Mar 2023 06:59:35 PM HKT
 ************************************************************************/

#define _GNU_SOURCE
#include "rpc_utils.h"
#include "rpc_tcp_utils.h"
#include "../stl/stl.h"
#include <stdio.h>
#include <ucp/api/ucp.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

ucs_status_t rpc_create_server_endpoint(ucp_worker_h data_worker,
                                        ucp_conn_request_h conn_request,
                                        ucp_ep_h *server_ep, rpc_err_cb err_cb, void *arg)
{
  ucp_ep_params_t ep_params;
  ucs_status_t status;

  /* Server creates an ep to the client on the data worker.
   * This is not the worker the listener was created on.
   * The client side should have initiated the connection, leading
   * to this ep's creation */
  ep_params.field_mask = UCP_EP_PARAM_FIELD_ERR_HANDLER |
                         UCP_EP_PARAM_FIELD_CONN_REQUEST;
  ep_params.conn_request = conn_request;
  ep_params.err_handler.cb = err_cb;
  ep_params.err_handler.arg = arg;

  status = ucp_ep_create(data_worker, &ep_params, server_ep);
  if (status != UCS_OK)
  {
    stl_log_error("failed to create an endpoint on the server: (%s)\n",
                  ucs_status_string(status));
  }

  return status;
}

int rpc_set_tcp_family(sa_family_t *ai_family, rpc_communication_type rpc_type)
{
  switch (rpc_type)
  {
  case RPC_TCPV4_TYPE:
    *ai_family = AF_INET;
    break;
  case RPC_TCPV6_TYPE:
    *ai_family = AF_INET6;
    break;
  case RPC_RDMA_TYPE:
    break;
  }
  return 0;
}
void rpc_close_endpoint(ucp_worker_h ucp_worker, ucp_ep_h ep, uint64_t flags)
{
  ucp_request_param_t param;
  ucs_status_t status;
  void *close_req;

  param.op_attr_mask = UCP_OP_ATTR_FIELD_FLAGS;
  param.flags = flags;
  close_req = ucp_ep_close_nbx(ep, &param);
  if (UCS_PTR_IS_PTR(close_req))
  {
    do
    {
      ucp_worker_progress(ucp_worker);
      status = ucp_request_check_status(close_req);
    } while (status == UCS_INPROGRESS);
    ucp_request_free(close_req);
  }
  else
  {
    status = UCS_PTR_STATUS(close_req);
  }

  if (status != UCS_OK)
  {
    stl_log_error("failed to close ep %p\n", (void *)ep);
  }
}
int rpc_init_worker(ucp_context_h ucp_context, ucp_worker_h *ucp_worker)
{
  ucp_worker_params_t worker_params;
  ucs_status_t status;
  int ret = 0;

  memset(&worker_params, 0, sizeof(worker_params));

  worker_params.field_mask = UCP_WORKER_PARAM_FIELD_THREAD_MODE;
  worker_params.thread_mode = UCS_THREAD_MODE_MULTI;

  status = ucp_worker_create(ucp_context, &worker_params, ucp_worker);
  if (status != UCS_OK)
  {
    stl_log_error("failed to ucp_worker_create (%s)\n", ucs_status_string(status));
    ret = -1;
  }

  return ret;
}
int rpc_get_communication_type(const char *rpc_communication)
{
  if (rpc_communication == NULL)
  {
    return -1;
  }
  if (strncmp(rpc_communication, "tcpv4", 5) == 0)
  {
    return RPC_TCPV4_TYPE;
  }
  else if (strncmp(rpc_communication, "tcpv6", 5) == 0)
  {
    return RPC_TCPV6_TYPE;
  }
  else if (strncmp(rpc_communication, "rdma", 4) == 0)
  {
    return RPC_RDMA_TYPE;
  }
  else
  {
    return -1;
  }
}

/**
 * Initialize the client side. Create an endpoint from the client side to be
 * connected to the remote server (to the given IP).
 */
ucs_status_t rpc_create_client_endpoint(rpc_communication_type communication_type, char *server_addr, int port, ucp_worker_h ucp_worker, ucp_ep_h *client_ep, rpc_err_cb err_cb, void *arg)
{
  ucp_ep_params_t ep_params;
  struct sockaddr_storage connect_addr;
  ucs_status_t status;

  sa_family_t ai_family;

  rpc_set_tcp_family(&ai_family, communication_type);

  rpc_set_tcp_sockaddr(server_addr, port, ai_family, &connect_addr);

  /*
   * Endpoint field mask bits:
   * UCP_EP_PARAM_FIELD_FLAGS             - Use the value of the 'flags' field.
   * UCP_EP_PARAM_FIELD_SOCK_ADDR         - Use a remote sockaddr to connect
   *                                        to the remote peer.
   * UCP_EP_PARAM_FIELD_ERR_HANDLING_MODE - Error handling mode - this flag
   *                                        is temporarily required since the
   *                                        endpoint will be closed with
   *                                        UCP_EP_CLOSE_MODE_FORCE which
   *                                        requires this mode.
   *                                        Once UCP_EP_CLOSE_MODE_FORCE is
   *                                        removed, the error handling mode
   *                                        will be removed.
   */
  ep_params.field_mask = UCP_EP_PARAM_FIELD_FLAGS |
                         UCP_EP_PARAM_FIELD_SOCK_ADDR |
                         UCP_EP_PARAM_FIELD_ERR_HANDLER |
                         UCP_EP_PARAM_FIELD_ERR_HANDLING_MODE;
  ep_params.err_mode = UCP_ERR_HANDLING_MODE_PEER;
  ep_params.err_handler.cb = err_cb;
  ep_params.err_handler.arg = arg;
  ep_params.flags = UCP_EP_PARAMS_FLAGS_CLIENT_SERVER;
  ep_params.sockaddr.addr = (struct sockaddr *)&connect_addr;
  ep_params.sockaddr.addrlen = sizeof(connect_addr);

  status = ucp_ep_create(ucp_worker, &ep_params, client_ep);
  if (status != UCS_OK)
  {
    stl_log_error("failed to connect to %s (%s)\n", server_addr,
                  ucs_status_string(status));
  }

  return status;
}
int rpc_thread_setaffinity(pthread_t pid, size_t thd_idx)
{
  int cpu_cnt = sysconf(_SC_NPROCESSORS_ONLN);
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  int bind_idx = thd_idx % cpu_cnt;
  CPU_SET(bind_idx, &cpuset);
  if (pthread_setaffinity_np(pid, sizeof(cpuset), &cpuset) < 0)
  {
    return -1;
  }
  return bind_idx;
}