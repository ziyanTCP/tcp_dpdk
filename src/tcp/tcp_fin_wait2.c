//
// Created by ziyan on 11/8/20.
//

#include "tcp/tcp_fin_wait2.h"

void handle_tcp_fin_wait2(struct tcp *_tcp, struct connection *_connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                          struct rte_ipv4_hdr *ipv4Hdr, void *data, int size) {
    printf("handle tcp fin wait 2\n");
    uint32_t seq = rte_be_to_cpu_32(tcpHdr->sent_seq);
    uint32_t ackn = rte_be_to_cpu_32(tcpHdr->recv_ack); // check ack number

    if (!segment_check(_connection, size, seq)) {
        printf("segment check failed!\n");
        return;
    } else {
        // check FIN flag
        _connection->rteTcpHdr.tcp_flags = RTE_TCP_ACK_FLAG;
        _connection->receiveSequenceSpace.nxt = seq + 1;
        tcp_tx_packets(_tcp, _connection);
    }
}