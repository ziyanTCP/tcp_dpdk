//
// Created by ziyan on 11/1/20.
//

#include "tcp/tcp_establish.h"

void handle_tcp_establish(struct tcp *_tcp, struct connection *_connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                          struct rte_ipv4_hdr *ipv4Hdr, void *data, int size) {
//    printf("handle_tcp_establish called!\n");
    uint32_t seq = rte_be_to_cpu_32(tcpHdr->sent_seq);
    uint32_t ackn = rte_be_to_cpu_32(tcpHdr->recv_ack);

    if (segment_check(_connection, size, seq)) {
        if ((tcpHdr->tcp_flags && RTE_TCP_FIN_FLAG) != 0) {
            _connection->tcpState = TCP_CLOSE_WAIT;
            _connection->rteTcpHdr.tcp_flags = RTE_TCP_ACK_FLAG;
            _connection->receiveSequenceSpace.nxt = seq + 1;
//            tcp_tx_packets(_tcp, _connection);
            _connection->rteTcpHdr.tcp_flags = RTE_TCP_FIN_FLAG | RTE_TCP_ACK_FLAG;
            tcp_tx_packets(_tcp, _connection);
            _connection->tcpState = TCP_LAST_ACK;
        }
    }
}