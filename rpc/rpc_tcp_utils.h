/*************************************************************************
  > File Name: rpc_tcp.h
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Wed 29 Mar 2023 06:57:51 PM HKT
 ************************************************************************/

#ifndef _RPC_TCP_H
#define _RPC_TCP_H
#include <stddef.h>
#include <string.h>    
#include <arpa/inet.h> /* inet_addr */
#include <unistd.h>    /* getopt */
#include <stdlib.h>    /* atoi */
char *rpc_get_tcp_port(const struct sockaddr_storage *sock_addr, char *port_str, size_t max_size);
char *rpc_get_tcp_address(const struct sockaddr_storage *sock_addr,char *ip_str, size_t max_size);
void rpc_set_tcp_sockaddr(const char *address_str, int port, sa_family_t ai_family, struct sockaddr_storage *saddr);
#endif
