//
// Created by ziyan on 11/8/20.
//

#ifndef TCP_TCP_FIN_WAIT1_H
#define TCP_TCP_FIN_WAIT1_H

#include "tcp.h"
void handle_tcp_fin_wait1(struct tcp *_tcp, struct connection *_connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                         struct rte_ipv4_hdr *ipv4Hdr, void *data, int size);
#endif //TCP_TCP_FIN_WAIT1_H
