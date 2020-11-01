//
// Created by ziyan on 10/31/20.
//

#include "rte_malloc.h"
#include "tcp/tcp_listen.h"
#include "rte_tcp.h"

void handle_tcp_listen(struct tcp *_tcp, struct connection *_connection, struct quad q, struct rte_tcp_hdr *tcpHdr,
                       struct rte_ipv4_hdr *ipv4Hdr, void *data, int size) {

    if (tcpHdr->tcp_flags != RTE_TCP_SYN_FLAG) {
        return;
    }
    uint32_t iss = 0;
    uint16_t wnd = 64240;
    _connection = rte_malloc(NULL, sizeof(struct connection), 0);
    _connection->q = q;
    _connection->tcpState = TCP_SYN_RECV;
    _connection->sendSequenceSpace.una = iss;
    _connection->sendSequenceSpace.nxt = iss;
    _connection->sendSequenceSpace.wnd = wnd;
    _connection->sendSequenceSpace.up = false;
    _connection->sendSequenceSpace.wl1 = 0;
    _connection->sendSequenceSpace.wl2 = 0;

    _connection->receiveSequenceSpace.irs = rte_be_to_cpu_32(tcpHdr->sent_seq);
    _connection->receiveSequenceSpace.nxt = rte_be_to_cpu_32(tcpHdr->sent_seq) + 1;
    _connection->receiveSequenceSpace.wnd = rte_be_to_cpu_16(tcpHdr->rx_win);
    _connection->receiveSequenceSpace.up = false;

    _connection->rteIpv4Hdr.src_addr = q.dip;
    _connection->rteIpv4Hdr.dst_addr = q.sip;
    _connection->rteTcpHdr.src_port = q.dport;
    _connection->rteTcpHdr.dst_port = q.sport;
    _connection->rteTcpHdr.sent_seq = 0;
    _connection->rteTcpHdr.rx_win = rte_cpu_to_be_16(wnd);
    _connection->rteTcpHdr.tcp_flags = RTE_TCP_SYN_FLAG | RTE_TCP_ACK_FLAG;

    tcp_tx_packets(_tcp, _connection);
}
