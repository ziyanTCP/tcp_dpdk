//
// Created by ziyan on 11/1/20.
//

#ifndef TCP_TCP_SYN_RECV_H
#define TCP_TCP_SYN_RECV_H
#include "tcp.h"
#include "rte_ip.h"
#include "rte_tcp.h"
void handle_tcp_syn_recv(struct tcp *_tcp, struct connection *_connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                       struct rte_ipv4_hdr *ipv4Hdr, void *data, int size);

#endif //TCP_TCP_SYN_RECV_H
