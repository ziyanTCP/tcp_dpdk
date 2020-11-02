//
// Created by ziyan on 11/1/20.
//

#include "tcp/tcp_syn_recv.h"

void handle_tcp_syn_recv(struct tcp *_tcp, struct connection *_connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                         struct rte_ipv4_hdr *ipv4Hdr, void *data, int size) {
    printf("handle_tcp_syn_recv called\n");
    uint32_t seq = rte_be_to_cpu_32(tcpHdr->sent_seq);
    uint32_t ackn = rte_be_to_cpu_32(tcpHdr->recv_ack);

    // do the segment check
    if (!segment_check(_connection, size, seq)) {
        printf("segment check failed!\n");
        return;
    }
    if (is_between_wrapped(_connection->sendSequenceSpace.una - 1, ackn, _connection->sendSequenceSpace.nxt + 1)) {
        printf("connection established!");
        _connection->tcpState = TCP_ESTABLISHED;
    }

    // no need to ack if no data
    return;
}
