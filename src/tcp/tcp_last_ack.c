//
// Created by ziyan on 11/1/20.
//

#include "../../headers/tcp/tcp_last_ack.h"

void handle_tcp_last_ack(struct tcp *_tcp, struct connection *_connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                         struct rte_ipv4_hdr *ipv4Hdr, void *data, int size) {
    uint32_t seq = rte_be_to_cpu_32(tcpHdr->sent_seq);
    uint32_t ackn = rte_be_to_cpu_32(tcpHdr->recv_ack);
//    printf("connection terminated!\n");
    _connection->tcpState = TCP_CLOSE;
}