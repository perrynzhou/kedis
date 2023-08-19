/*************************************************************************
  > File Name: rpc_server.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Thu 23 Mar 2023 02:09:02 PM HKT
 ************************************************************************/
#include "rpc_server.h"
#include "rpc_tcp_protocol.h"
#include "../stl/stl.h"
#include "rpc_utils.h"
#include "rpc_tcp_utils.h"
#include "rpc_body.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#define RPC_CONNECTION_MAX_CACHE_SIZE (65535)
typedef struct
{
  ucp_ep_h server_ep;
  ucp_worker_h ucp_data_worker;
  ucp_conn_request_h conn_request;
  size_t magic;
  void *thread;
} rpc_server_connection;

typedef struct
{
  pthread_t thd;
  size_t index;
  rpc_server_connection **thd_request_queue;
  sem_t sem;
  void *server_ctx;
  rpc_server_msg_handle msg_handle;
  void *msg_handle_ctx;
  stl_dict *connection_cache;
} rpc_server_thread;

static void rpc_server_traverse_cache_handle(void *val)
{
  rpc_server_connection *connection = (rpc_server_connection *)val;
  rpc_server_thread *sthread = (rpc_server_thread *)connection->thread;
  stl_log_info("remove connection=%p magic=%d from cache\n", (void *)connection, connection->magic);
  char key[256] = {'\0'};
  snprintf((char *)&key, 256, "%ld-%p", connection->magic, (void *)connection);
  rpc_server_connection *temp = (rpc_server_connection *)stl_dict_get(sthread->connection_cache, (char *)&key);
  rpc_close_endpoint(connection->ucp_data_worker, connection->server_ep, UCP_EP_CLOSE_MODE_FORCE);
  free(connection);
  connection = NULL;
}
static void rpc_server_common_cb(void *user_data, const char *type_str)
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

void rpc_server_tcp_send_cb(void *request, ucs_status_t status, void *user_data)
{
  assert(request != NULL);
  rpc_server_common_cb(user_data, "rpc_server_tcp_send_cb");
}

void rpc_server_tcp_recv_cb(void *request, ucs_status_t status, size_t length,
                            void *user_data)
{
  assert(request != NULL);
  rpc_server_common_cb(user_data, "rpc_server_tcp_recv_cb");
}

void rpc_server_tcp_err_cb(void *arg, ucp_ep_h ep, ucs_status_t status)
{
  rpc_server_traverse_cache_handle(arg);
  stl_log_error("error handling callback was invoked with status %d (%s)\n", status, ucs_status_string(status));
}

// typedef int (*rpc_server_msg_handle)(void *ctx, rpc_body *body);
static int rpc_server_do_response(void *arg, ucp_worker_h worker, ucp_ep_h ep, void *rbody)
{
  rpc_server_thread *rst = (rpc_server_thread *)arg;

  rpc_body *in = (rpc_body *)rbody;
  rpc_body out;
  int ret = 0;
  bool done = false;
  if (rst->msg_handle_ctx != NULL && rst->msg_handle != NULL)
  {
    ret = rst->msg_handle(rst->msg_handle_ctx, in, &out);
    done = true;
  }
  if (!done)
  {
    out.msg = in->msg;
    out.len = in->len;
  }
  rpc_server *server = (rpc_server *)rst->server_ctx;
  ret = rpc_tcp_send(worker, ep, &out, server->send_cb);
  stl_log_info("server send ret=%d\n", ret);

  if (done)
  {
    rpc_body_deinit(&out);
  }
  return 0;
}

static int rpc_server_thread_init(rpc_server_thread *sthread, size_t idx, void *server_ctx)
{
  if (sthread != NULL)
  {
    sem_init(&sthread->sem, 0, 0);
    stl_queue_create(sthread->thd_request_queue, 0);
    sthread->index = idx;
    sthread->server_ctx = server_ctx;
    sthread->msg_handle_ctx = NULL;
    sthread->msg_handle = NULL;
    sthread->connection_cache = stl_dict_create(RPC_CONNECTION_MAX_CACHE_SIZE);
    stl_dict_entry_handle_fn(sthread->connection_cache, rpc_server_traverse_cache_handle);

    return 0;
  }
  return -1;
}
static void rpc_server_thread_deinit(rpc_server_thread *sthread)
{
  if (sthread != NULL)
  {
    stl_queue_destroy(sthread->thd_request_queue);
    sem_destroy(&sthread->sem);

    stl_dict_entry_handle_fn(sthread->connection_cache, rpc_server_traverse_cache_handle);
    stl_dict_traverse(sthread->connection_cache);
    stl_dict_destroy(sthread->connection_cache);
  }
}

static int tcp_server_recv(ucp_worker_h ucp_worker, ucp_ep_h ep, void *server_thread)
{
  rpc_server_thread *rst = (rpc_server_thread *)server_thread;
  rpc_server *rserver = (rpc_server *)rst->server_ctx;
  rpc_body body;

  rpc_body_init(&body, NULL, RPC_BODY_MSG_MAX_CAP);
  ucs_status_t status = rpc_tcp_recv(ucp_worker, ep, &body, rserver->recv_cb);
  if (status == UCS_OK)
  {
    stl_log_info("server recv msg: %s  len:%ld\n", (char *)body.msg, body.len);
    int ret = rpc_server_do_response(rst, ucp_worker, ep, &body);
    stl_log_info("ret=%d\n", ret);
  }
  rpc_body_deinit(&body);
  return status;
}
static int rpc_server_do_request(void *arg, ucp_worker_h worker, ucp_ep_h ep)
{
  int ret;
  rpc_server_thread *server_thread = (rpc_server_thread *)arg;
  rpc_server *rserver = (rpc_server *)server_thread->server_ctx;

  switch (rserver->rpc_type)
  {
  case RPC_TCPV4_TYPE:
    /* Client-Server communication via Stream API */
    ret = tcp_server_recv(worker, ep, server_thread);
    break;
  case RPC_TCPV6_TYPE:
    /* Client-Server communication via Stream API */
    ret = tcp_server_recv(worker, ep, server_thread);
    break;
  default:
    stl_log_error("unknown rpc type %d\n", rserver->rpc_type);
    return -1;
  }

  return ret;
}

static void rpc_server_accept_cb(ucp_conn_request_h conn_request, void *arg)
{
  rpc_server *rserver = (rpc_server *)arg;
  ucp_conn_request_attr_t attr;
  char ip_str[64];
  char port_str[64];
  ucs_status_t status;

  attr.field_mask = UCP_CONN_REQUEST_ATTR_FIELD_CLIENT_ADDR;
  status = ucp_conn_request_query(conn_request, &attr);
  if (status == UCS_OK)
  {
    printf("Server accept a connection request from client at address %s:%s\n",
           rpc_get_tcp_address(&attr.client_address, ip_str, sizeof(ip_str)),
           rpc_get_tcp_port(&attr.client_address, port_str, sizeof(port_str)));
  }
  else if (status != UCS_ERR_UNSUPPORTED)
  {
    stl_log_error("failed to query the connection request (%s)\n",
                  ucs_status_string(status));
  }

  pthread_mutex_lock(&rserver->lock);
  rpc_server_connection *conn = calloc(1, sizeof(rpc_server_connection));
  conn->conn_request = conn_request;
  conn->magic = ++rserver->request_cnt;
  stl_queue_add_last(rserver->dispatch_connection_queue, conn);

  pthread_mutex_unlock(&rserver->lock);
  stl_log_info("rpc_server_accept_cb: push connection  %p connection macigc=%ld\n", (void *)conn, conn->magic);
}
static void *rpc_server_thread_func(void *arg)
{

  stl_log_info("start rpc_server_thread_func  on %ld\n", pthread_self());
  rpc_server_thread *server_thread = (rpc_server_thread *)arg;
  int cpu_affinity=rpc_thread_setaffinity(pthread_self(),server_thread->index);
  if(cpu_affinity >= 0 )
  {
    stl_log_info("thread bind  %ld on cpu\n", cpu_affinity);
  }
  rpc_server *rserver = (rpc_server *)server_thread->server_ctx;
  while (1)
  {
    sem_wait(&server_thread->sem);

    size_t qlen = stl_queue_size(server_thread->thd_request_queue);

    rpc_server_connection *connection = (rpc_server_connection *)stl_queue_del_first(server_thread->thd_request_queue);
    connection->thread = server_thread;
    char key[256] = {'\0'};
    snprintf((char *)&key, 256, "%ld-%p", connection->magic, (void *)connection);
    stl_dict_put(server_thread->connection_cache, (char *)&key, connection);
    stl_log_info("cache connection key=%s,value=%p\n", (char *)&key, (void *)connection);
    stl_log_info("thread handle qlen=%ld, connection=%p connection macigc=%ld\n", qlen, (void *)connection, connection->magic);

    int ret = rpc_init_worker(rserver->ucp_context, &connection->ucp_data_worker);
    ucs_status_t status = rpc_create_server_endpoint(connection->ucp_data_worker, connection->conn_request,
                                                     &connection->server_ep, rserver->err_cb, connection);
    if (status != UCS_OK)
    {
      stl_log_error("rpc_create_server_endpoint error\n");
      continue;
    }
    int max_msg_cnt = 1024;
    int i = 0;
    do
    {
      ret = rpc_server_do_request(server_thread, connection->ucp_data_worker, connection->server_ep);
      if (ret != 0)
      {
        break;
      }
      i++;
    } while (i < max_msg_cnt);
  }
  return NULL;
}
static ucs_status_t rpc_server_create_listener(rpc_server *rserver, const char *server_addr)
{
  struct sockaddr_storage listen_sockaddr;
  ucp_listener_params_t params;
  ucp_listener_attr_t attr;
  ucs_status_t status;
  char ip_str[64];
  char port_str[64];

  sa_family_t ai_family;

  rpc_set_tcp_family(&ai_family, rserver->rpc_type);
  char *addr_info[2] = {NULL};
  char *save = NULL;
  const char *token;
  int j = 0;
  char *temp = stl_string_new(server_addr);
  while ((token = stl_string_token_begin(temp, &save, ":")) != NULL)
  {
    addr_info[j++] = stl_string_new(token);
  }
  stl_string_destroy(temp);
  rpc_set_tcp_sockaddr(addr_info[0], atoi(addr_info[1]), ai_family, &listen_sockaddr);

  for (size_t j = 0; j < 2; j++)
  {
    stl_string_destroy(addr_info[j++]);
  }
  params.field_mask = UCP_LISTENER_PARAM_FIELD_SOCK_ADDR |
                      UCP_LISTENER_PARAM_FIELD_CONN_HANDLER;
  params.sockaddr.addr = (const struct sockaddr *)&listen_sockaddr;
  params.sockaddr.addrlen = sizeof(listen_sockaddr);
  params.conn_handler.cb = rpc_server_accept_cb;
  params.conn_handler.arg = rserver;

  /* Create a listener on the server side to listen on the given address.*/
  status = ucp_listener_create(rserver->ucp_worker, &params, &rserver->listener);
  if (status != UCS_OK)
  {
    stl_log_error("failed to listen (%s)\n", ucs_status_string(status));
    goto out;
  }

  /* Query the created listener to get the port it is listening on. */
  attr.field_mask = UCP_LISTENER_ATTR_FIELD_SOCKADDR;
  status = ucp_listener_query(rserver->listener, &attr);
  if (status != UCS_OK)
  {
    stl_log_error("failed to query the listener (%s)\n",
                  ucs_status_string(status));
    ucp_listener_destroy(rserver->listener);
    goto out;
  }

  stl_log_info("server is listening on IP %s port %s\n",
               rpc_get_tcp_address(&attr.sockaddr, ip_str, 64),
               rpc_get_tcp_port(&attr.sockaddr, port_str, 64));

  stl_log_info("waiting for connection...\n");

out:
  return status;
}

int rpc_server_init(rpc_server *rserver, size_t worker_num, const char *rpc_communication, const char *server_addr)
{
  if (rserver == NULL || rpc_communication == NULL)
  {
    return -1;
  }
  rserver->rpc_type = rpc_get_communication_type(rpc_communication);
  if (rserver->rpc_type < 0)
  {
    return -1;
  }
  ucp_params_t ucp_params;
  ucs_status_t status;

  memset(&ucp_params, 0, sizeof(ucp_params));

  ucp_params.field_mask = UCP_PARAM_FIELD_FEATURES;

  switch (rserver->rpc_type)
  {
  case RPC_TCPV4_TYPE:
    ucp_params.features = UCP_FEATURE_STREAM | UCP_FEATURE_WAKEUP;
    break;
  case RPC_TCPV6_TYPE:
    ucp_params.features = UCP_FEATURE_STREAM | UCP_FEATURE_WAKEUP;
    break;
  case RPC_RDMA_TYPE:
    ucp_params.features = UCP_FEATURE_STREAM | UCP_FEATURE_WAKEUP;
    break;
  }

  status = ucp_init(&ucp_params, NULL, &rserver->ucp_context);
  if (status != UCS_OK)
  {
    stl_log_error("failed to ucp_init (%s)\n", ucs_status_string(status));
    return -1;
  }
  int ret = rpc_init_worker(rserver->ucp_context, &rserver->ucp_worker);
  ret = rpc_server_create_listener(rserver, server_addr);
  stl_log_info("rpc_server_create_listener ret=%d\n", ret);
  stl_queue_create(rserver->dispatch_connection_queue, 0);
  rserver->ucp_worker_num = worker_num;
  rserver->ucp_worker_threads = calloc(worker_num, sizeof(rpc_server_thread));
  rpc_server_thread *server_threads = (rpc_server_thread *)rserver->ucp_worker_threads;
  rserver->err_cb = rpc_server_tcp_err_cb;
  rserver->send_cb = rpc_server_tcp_send_cb;
  rserver->recv_cb = rpc_server_tcp_recv_cb;
  rserver->request_cnt = 0;
  for (size_t i = 0; i < worker_num; i++)
  {
    rpc_server_thread *server_thread = &server_threads[i];
    int ret = rpc_server_thread_init(server_thread, i, rserver);
    if (ret != 0)
    {
      exit(-1);
    }
    pthread_create(&server_thread->thd, NULL, rpc_server_thread_func, server_thread);
  }
  pthread_mutex_init(&rserver->lock, NULL);
  rserver->server_threads = server_threads;
  return 0;
}
int rpc_server_run(rpc_server *rserver)
{
  while (1)
  {
    while (stl_queue_empty(rserver->dispatch_connection_queue))
    {
      ucp_worker_progress(rserver->ucp_worker);
      ucp_worker_wait(rserver->ucp_worker);
    }

    pthread_mutex_lock(&rserver->lock);

    rpc_server_connection *connection = stl_queue_del_first(rserver->dispatch_connection_queue);

    if (connection != NULL)
    {
      stl_log_info("rpc_server_run: pop connection  %p connection macigc=%ld\n", (void *)connection, connection->magic);
    }
    rserver->request_cnt++;
    size_t idx = rserver->request_cnt % rserver->ucp_worker_num;
    rpc_server_thread *server_threads = (rpc_server_thread *)rserver->ucp_worker_threads;
    rpc_server_thread *server_thread = &server_threads[idx];
    stl_queue_add_last(server_thread->thd_request_queue, connection);
    size_t qlen = stl_queue_size(server_thread->thd_request_queue);
    stl_log_info("rpc_server thread_num=%ld,dispatch  to %ld thread,queue len=%ld .....\n", rserver->ucp_worker_num, idx, qlen);

    sem_post(&server_thread->sem);
    pthread_mutex_unlock(&rserver->lock);
  }
}

void rpc_server_deinit(rpc_server *rserver)
{
  if (rserver != NULL)
  {
    rpc_server_thread *server_threads = (rpc_server_thread *)rserver->ucp_worker_threads;

    for (size_t i = 0; i < rserver->ucp_worker_num; i++)
    {
      rpc_server_thread *server_thread = (rpc_server_thread *)&server_threads[i];
      pthread_join(server_thread->thd, NULL);
      rpc_server_thread_deinit(server_thread);
    }
    free(rserver->ucp_worker_threads);
  }
  rpc_server_deinit(rserver);
}
void rpc_server_set_msg_handle(rpc_server *rserver, void *ctx, rpc_server_msg_handle msg_handle)
{
  if (rserver != NULL && ctx != NULL && msg_handle != NULL)
  {
    rpc_server_thread *server_threads = (rpc_server_thread *)rserver->server_threads;
    for (size_t i = 0; i < rserver->ucp_worker_num; i++)
    {
      server_threads[i].msg_handle_ctx = ctx;
      server_threads[i].msg_handle = msg_handle;
    }
  }
}
#ifdef RPC_SERVER
int main(int argc, char *argv[])
{

  if(argc < 2) {
    fprintf(stdout,"usage:%s {thread_num}\n",argv[0]);
    exit(0);
  }
  stl_log_init();
  size_t n = atoi(argv[1]);
  rpc_server s1;
  rpc_server_init(&s1, n, "tcpv4", "127.0.0.1:8800");
  rpc_server_run(&s1);
}
#endif