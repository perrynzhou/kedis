/*************************************************************************
  > File Name: rpc_tcp.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wed 29 Mar 2023 06:57:57 PM HKT
 ************************************************************************/

#include "rpc_tcp_utils.h"
#include <stdio.h>
#include <string.h>    /* memset */
#include <arpa/inet.h> /* inet_addr */
#include <unistd.h>    /* getopt */
#include <stdlib.h>    /* atoi */

char *rpc_get_tcp_address(const struct sockaddr_storage *sock_addr,
                          char *ip_str, size_t max_size)
{
  struct sockaddr_in addr_in;
  struct sockaddr_in6 addr_in6;

  switch (sock_addr->ss_family)
  {
  case AF_INET:
    memcpy(&addr_in, sock_addr, sizeof(struct sockaddr_in));
    inet_ntop(AF_INET, &addr_in.sin_addr, ip_str, max_size);
    return ip_str;
  case AF_INET6:
    memcpy(&addr_in6, sock_addr, sizeof(struct sockaddr_in6));
    inet_ntop(AF_INET6, &addr_in6.sin6_addr, ip_str, max_size);
    return ip_str;
  default:
    return "Invalid address family";
  }
}

char *rpc_get_tcp_port(const struct sockaddr_storage *sock_addr,
                       char *port_str, size_t max_size)
{
  struct sockaddr_in addr_in;
  struct sockaddr_in6 addr_in6;

  switch (sock_addr->ss_family)
  {
  case AF_INET:
    memcpy(&addr_in, sock_addr, sizeof(struct sockaddr_in));
    snprintf(port_str, max_size, "%d", ntohs(addr_in.sin_port));
    return port_str;
  case AF_INET6:
    memcpy(&addr_in6, sock_addr, sizeof(struct sockaddr_in6));
    snprintf(port_str, max_size, "%d", ntohs(addr_in6.sin6_port));
    return port_str;
  default:
    return "Invalid address family";
  }
}
void rpc_set_tcp_sockaddr(const char *address_str, int port, sa_family_t ai_family, struct sockaddr_storage *saddr)
{
  struct sockaddr_in *sa_in;
  struct sockaddr_in6 *sa_in6;

  /* The server will listen on INADDR_ANY */
  memset(saddr, 0, sizeof(*saddr));

  switch (ai_family)
  {
  case AF_INET:
    sa_in = (struct sockaddr_in *)saddr;
    if (address_str != NULL)
    {
      inet_pton(AF_INET, address_str, &sa_in->sin_addr);
    }
    else
    {
      sa_in->sin_addr.s_addr = INADDR_ANY;
    }
    sa_in->sin_family = AF_INET;
    sa_in->sin_port = htons(port);
    break;
  case AF_INET6:
    sa_in6 = (struct sockaddr_in6 *)saddr;
    if (address_str != NULL)
    {
      inet_pton(AF_INET6, address_str, &sa_in6->sin6_addr);
    }
    else
    {
      sa_in6->sin6_addr = in6addr_any;
    }
    sa_in6->sin6_family = AF_INET6;
    sa_in6->sin6_port = htons(port);
    break;
  default:
    fprintf(stderr, "Invalid address family");
    break;
  }
}
