//
// Created by ziyan on 10/31/20.
//

#ifndef TCP_TCP_LISTEN_H
#define TCP_TCP_LISTEN_H

#include "tcp.h"
#include "rte_tcp.h"
#include "rte_ip.h"

void handle_tcp_listen(struct tcp* _tcp, struct connection *_connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                       struct rte_ipv4_hdr *ipv4Hdr, void *data, int size);

#endif //TCP_TCP_LISTEN_H
