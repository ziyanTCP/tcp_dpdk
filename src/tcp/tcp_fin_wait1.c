//
// Created by ziyan on 11/8/20.
//

#include "tcp/tcp_fin_wait1.h"

void handle_tcp_fin_wait1(struct tcp *_tcp, struct connection *_connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                          struct rte_ipv4_hdr *ipv4Hdr, void *data, int size) {

    printf("handle_tcp_fin_wait1\n");
    uint32_t seq = rte_be_to_cpu_32(tcpHdr->sent_seq);
    uint32_t ackn = rte_be_to_cpu_32(tcpHdr->recv_ack);

    if (!segment_check(_connection, size, seq)) {
        printf("segment check failed!\n");
        return;
    } else {
        if (is_between_wrapped((_connection->sendSequenceSpace.una - 1), ackn,
                               (_connection->sendSequenceSpace.nxt + 1))) {
            _connection->tcpState = TCP_FIN_WAIT2;
            _connection->rteTcpHdr.tcp_flags = RTE_TCP_ACK_FLAG;
            _connection->receiveSequenceSpace.nxt = seq + 1;
            tcp_tx_packets(_tcp, _connection, NULL, 0);
            printf("closing\n"); // if it is FIN ACK, close directly
            rte_hash_del_key(_tcp->rteHash, &_connection->q);
        } else {
            printf("not ack the correct one\n");
        }
    }
}