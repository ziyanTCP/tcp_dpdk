//
// Created by ziyan on 11/6/20.
//

#include "tcp/tcp_syn_sent.h"

void handle_tcp_syn_sent(struct tcp *_tcp, struct connection *_connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                         struct rte_ipv4_hdr *ipv4Hdr, void *data, int size) {
    printf("handle_tcp_syn_sent called\n");
    uint32_t seq = rte_be_to_cpu_32(tcpHdr->sent_seq);
    uint32_t ackn = rte_be_to_cpu_32(tcpHdr->recv_ack);
    _connection->receiveSequenceSpace.nxt = seq; // has to initialize this, but make segment check redundant?

    if (segment_check(_connection, 0, seq) == true) { // redundant?
        // whether ack our previous SYN
        if (is_between_wrapped((_connection->sendSequenceSpace.una - 1), ackn,
                               (_connection->sendSequenceSpace.nxt + 1))) {
            _connection->tcpState = TCP_ESTABLISHED;
            _connection->sendSequenceSpace.una = ackn + 1;
        }
    } else {
        return;
    }

    _connection->rteTcpHdr.tcp_flags = RTE_TCP_ACK_FLAG;
    _connection->receiveSequenceSpace.nxt = seq + 1;
    tcp_tx_packets(_tcp, _connection, data, size);


    // let's close the connection directly
//    sleep(2);
//    _connection->rteTcpHdr.tcp_flags = RTE_TCP_FIN_FLAG | RTE_TCP_ACK_FLAG;
//    _connection->receiveSequenceSpace.nxt = seq + 1;
//    tcp_tx_packets(_tcp, _connection, data, size);
//    _connection->tcpState = TCP_FIN_WAIT1;

}