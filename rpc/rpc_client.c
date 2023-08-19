/*************************************************************************
  > File Name: rpc_client.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Thu 02 Mar 2023 06:52:30 AM HKT
 ************************************************************************/

#include "rpc_client.h"
#include "rpc_utils.h"
#include "rpc_body.h"
#include "rpc_tcp_protocol.h"
#include "rpc_tcp_utils.h"
#include "../stl/stl.h"
#include <stdio.h>
#include <string.h>    /* memset */
#include <arpa/inet.h> /* inet_addr */
#include <unistd.h>    /* getopt */
#include <stdlib.h>    /* atoi */

static void rpc_client_common_cb(void *user_data, const char *type_str)
{
  rpc_body_complete *ctx;

  if (user_data == NULL)
  {
    stl_log_error("user_data passed to %s mustn't be NULL\n", type_str);
    return;
  }

  ctx = user_data;
  ctx->complete = 1;
}

void rpc_client_tcp_send_cb(void *request, ucs_status_t status, void *user_data)
{
  assert(request != NULL);
  rpc_client_common_cb(user_data, "rpc_client_tcp_send_cb");
}

void rpc_client_tcp_recv_cb(void *request, ucs_status_t status, size_t length,
                            void *user_data)
{
  assert(request != NULL);
  rpc_client_common_cb(user_data, "rpc_client_tcp_recv_cb");
}

void rpc_client_tcp_err_cb(void *arg, ucp_ep_h ep, ucs_status_t status)
{
  stl_log_error("error handling callback was invoked with status %d (%s)\n", status, ucs_status_string(status));
}

int rpc_client_init(rpc_client *rclient, const char *server_addr, const char *var_rpc_communication)
{
  int ret = -1;
  rpc_communication_type rpc_communication = rpc_get_communication_type(var_rpc_communication);
  if (rpc_communication < 0)
  {
    return ret;
  }

  char *save = NULL;
  char *token;
  int j = 0;
  char *temp = stl_string_new(server_addr);
  char *addr_info[2] = {NULL};
  while ((token = stl_string_token_begin(temp, &save, ":")) != NULL)
  {
    addr_info[j++] = stl_string_new(token);
  }

  ucp_params_t ucp_params;
  ucs_status_t status;

  memset(&ucp_params, 0, sizeof(ucp_params));
  rclient->rpc_communication = rpc_communication;

  /* UCP initialization */
  ucp_params.field_mask = UCP_PARAM_FIELD_FEATURES;

  switch (rpc_communication)
  {
  case RPC_TCPV4_TYPE:
    ucp_params.features = UCP_FEATURE_STREAM | UCP_FEATURE_WAKEUP;
    break;
  case RPC_TCPV6_TYPE:
    ucp_params.features = UCP_FEATURE_STREAM | UCP_FEATURE_WAKEUP;
    break;
  case RPC_RDMA_TYPE:
    ucp_params.features = UCP_FEATURE_AM;
    break;
  }

  status = ucp_init(&ucp_params, NULL, &rclient->ucp_context);
  if (status != UCS_OK)
  {
    stl_log_info("failed to ucp_init (%s)\n", ucs_status_string(status));
    ret = -1;
    goto err;
  }

  ret = rpc_init_worker(rclient->ucp_context, &rclient->ucp_worker);
  if (ret != 0)
  {
    goto err_cleanup;
  }

  status = rpc_create_client_endpoint(rclient->rpc_communication, addr_info[0], atoi(addr_info[1]), rclient->ucp_worker, &rclient->client_ep, rclient->err_cb, NULL);
  if (status != UCS_OK)
  {
    stl_log_info("failed to start client (%s)\n", ucs_status_string(status));
    ret = -1;
  }
  rclient->err_cb = rpc_client_tcp_err_cb;
  rclient->send_cb = rpc_client_tcp_send_cb;
  rclient->recv_cb = rpc_client_tcp_recv_cb;
  return ret;
err_cleanup:
  ucp_cleanup(rclient->ucp_context);
err:
  return ret;
}

int rpc_client_recv(rpc_client *rclient, rpc_body *out)
{
  int ret = -1;
  switch (rclient->rpc_communication)
  {
  case RPC_TCPV4_TYPE:
    ret = rpc_tcp_recv(rclient->ucp_worker, rclient->client_ep, out, rclient->recv_cb);
    break;
  case RPC_TCPV6_TYPE:
    ret = rpc_tcp_recv(rclient->ucp_worker, rclient->client_ep, out, rclient->recv_cb);
    break;

  case RPC_RDMA_TYPE:
    break;
  }
  return ret;
}
int rpc_client_send(rpc_client *rclient, rpc_body *in)

{
  int ret = -1;
  switch (rclient->rpc_communication)
  {
  case RPC_TCPV4_TYPE:
    ret = rpc_tcp_send(rclient->ucp_worker, rclient->client_ep, in, rclient->send_cb);
    break;
  case RPC_TCPV6_TYPE:
    ret = rpc_tcp_send(rclient->ucp_worker, rclient->client_ep, in, rclient->send_cb);
    break;
  case RPC_RDMA_TYPE:
    break;
  }
  return ret;
}
void rpc_client_deinit(rpc_client *rclient)
{
  if (rclient != NULL)
  {
    rpc_close_endpoint(rclient->ucp_worker, rclient->client_ep, UCP_EP_CLOSE_MODE_FORCE);
    ucp_worker_destroy(rclient->ucp_worker);
  }
}

#ifdef RPC_CLIENT
#include <pthread.h>

typedef struct
{
  rpc_client client;
  size_t cnt;
  pthread_t tid;
} rpc_client_thread;
void *rpc_client_thread_func(void *arg)
{
  rpc_client_thread *thd = (rpc_client_thread *)arg;
  rpc_client *client = &thd->client;
  rpc_client_init(client, "127.0.0.1:8800", "tcpv4");
  rpc_body *req = calloc(1, sizeof(rpc_body));

  for (size_t i = 0; i < thd->cnt; i++)
  {

    char buf[RPC_BODY_MSG_MAX_CAP] = {'\0'};
    snprintf((char *)&buf, RPC_BODY_MSG_MAX_CAP, "%ld-data-%ld", pthread_self(), i);
    rpc_body_init(req, (char *)&buf, RPC_BODY_MSG_MAX_CAP);
    int ret = rpc_client_send(client, req);
    memset(req->msg, 0, req->len);
    stl_log_info(" client send  ret=%d\n", ret);
    ret = rpc_client_recv(client, req);
    stl_log_info(" client recv  ret=%d,msg=%s,len=%d\n", ret, (char *)req->msg, req->len);
  }
  stl_log_info("finish thread %ld\n", pthread_self());
  rpc_client_deinit(client);
  return NULL;
}
int main(int argc, char *argv[])
{
  if(argc < 3) {
    fprintf(stdout,"usage: %s {thread_num}  {message_cnt}\n",argv[0]);
    exit(0);
  }
  stl_log_init();
  int thd_num = atoi(argv[1]);
  int thd_cnt = atoi(argv[2]);
  rpc_client_thread *thds = calloc(thd_num, sizeof(rpc_client_thread));
  for (int i = 0; i < thd_num; i++)
  {
    thds[i].cnt = thd_cnt;
    pthread_create(&thds[i].tid, NULL, rpc_client_thread_func, &thds[i]);
  }

  for (int i = 0; i < thd_num; i++)
  {
    pthread_join(thds[i].tid, NULL);
  }
}
#endif
