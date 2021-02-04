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
        if ((tcpHdr->tcp_flags & RTE_TCP_SYN_FLAG) != 0) {
            _connection->sendSequenceSpace.nxt = _connection->sendSequenceSpace.nxt - 1;
            _connection->rteTcpHdr.tcp_flags = _connection->rteTcpHdr.tcp_flags | RTE_TCP_SYN_FLAG;
//            printf("retransmit!\n");
            tcp_tx_packets(_tcp, _connection, data, size);
        }
        return;
    }
    if (is_between_wrapped(_connection->sendSequenceSpace.una - 1, ackn, _connection->sendSequenceSpace.nxt + 1)) {
//        printf("connection established! %d\n", _tcp->core_id);
        _connection->tcpState = TCP_ESTABLISHED;
        if (_tcp->stats.first_connection == 0) {
            _tcp->stats.first_connection = clock();
        } else {
            _tcp->stats.last_connection = clock();
        }
        _tcp->stats.connection_count += 1;
        //printf("Total connections: %lu\n", _tcp->stats.connection_count);
    }

    // no need to ack if no data
    return;
}
