//
// Created by ziyan on 11/1/20.
//

#ifndef TCP_TCP_LAST_ACK_H
#define TCP_TCP_LAST_ACK_H
#include "tcp.h"
void handle_tcp_last_ack(struct tcp *_tcp, struct connection *_connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                         struct rte_ipv4_hdr *ipv4Hdr, void *data, int size);

#endif //TCP_TCP_LAST_ACK_H
