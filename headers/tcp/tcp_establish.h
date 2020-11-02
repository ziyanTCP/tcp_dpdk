//
// Created by ziyan on 11/1/20.
//

#ifndef TCP_TCP_ESTABLISH_H
#define TCP_TCP_ESTABLISH_H
#include "tcp.h"
void handle_tcp_establish(struct tcp *_tcp, struct connection*connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                          struct rte_ipv4_hdr *ipv4Hdr, void *data, int size);
#endif //TCP_TCP_ESTABLISH_H
