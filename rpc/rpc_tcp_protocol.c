/*************************************************************************
  > File Name: rpc_tcp_protocol.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Fri 31 Mar 2023 07:07:25 PM HKT
 ************************************************************************/

#include "rpc_tcp_protocol.h"
#include "rpc_body.h"
#include "../stl/stl.h"
#include <stdio.h>
#include <ucp/api/ucp.h>
ucs_status_t rpc_wait_complete(ucp_worker_h ucp_worker, void *request,
                                      rpc_body_complete *ctx)
{
  ucs_status_t status;

  if (request == NULL)
  {
    return UCS_OK;
  }

  if (UCS_PTR_IS_ERR(request))
  {
    return UCS_PTR_STATUS(request);
  }

  while (ctx->complete == 0)
  {
    ucp_worker_progress(ucp_worker);
  }
  status = ucp_request_check_status(request);

  ucp_request_free(request);

  return status;
}

ucs_status_t rpc_tcp_recv(ucp_worker_h ucp_worker, ucp_ep_h ep, rpc_body *data,rpc_tcp_recv_cb recv_cb)
{
  rpc_body_complete req = {.complete = 0};
  rpc_body_complete *req_ptr = NULL;
  ucp_request_param_t param;
  param.op_attr_mask = UCP_OP_ATTR_FIELD_CALLBACK |
                       UCP_OP_ATTR_FIELD_DATATYPE |
                       UCP_OP_ATTR_FIELD_USER_DATA;
  param.datatype = ucp_dt_make_contig(1);
  param.user_data = &req;

  /* Server receives a message from the client using the stream API */
  param.op_attr_mask |= UCP_OP_ATTR_FIELD_FLAGS;
  param.flags = UCP_STREAM_RECV_FLAG_WAITALL;
  param.cb.recv_stream = recv_cb;
  req_ptr = ucp_stream_recv_nbx(ep, data->msg, data->len,
                                &data->len, &param);

  ucs_status_t status = rpc_wait_complete(ucp_worker, req_ptr, &req);

  return status;
}

int rpc_tcp_send(ucp_worker_h ucp_worker, ucp_ep_h ep, rpc_body *data,rpc_tcp_send_cb send_cb)
{
  ucp_request_param_t param;
  rpc_body_complete *request;
  size_t msg_length;
  void *msg;
  rpc_body_complete ctx = {.complete = 0};

  // memset(iov, 0, iov_cnt * sizeof(*iov));

  msg = data->msg;

  msg_length = data->len;

  ctx.complete = 0;
  param.op_attr_mask = UCP_OP_ATTR_FIELD_CALLBACK |
                       UCP_OP_ATTR_FIELD_DATATYPE |
                       UCP_OP_ATTR_FIELD_USER_DATA;
  param.datatype = ucp_dt_make_contig(1);
  param.user_data = &ctx;

  /* Client sends a message to the server using the stream API */
  param.cb.send = send_cb;
  request = ucp_stream_send_nbx(ep, msg, msg_length, &param);

  int ret = 0;
  ucs_status_t status;

  status = rpc_wait_complete(ucp_worker, request, &ctx);
  if (status != UCS_OK)
  {
    stl_log_error("send unable to %s UCX message (%s)\n", ucs_status_string(status));
    ret = -1;
  }
  else
  {
    if (data->msg != NULL)
    {
      stl_log_info(" send msg=%s,len=%d\n", (char *)data->msg, data->len);
    }
  }

  return ret;
}
