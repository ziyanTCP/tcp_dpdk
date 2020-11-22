//
// Created by ziyan on 10/31/20.
//

#include "rte_malloc.h"
#include "tcp/tcp_listen.h"
#include "rte_tcp.h"

struct connection *handle_tcp_listen(struct tcp *_tcp, struct quad q, struct rte_tcp_hdr *tcpHdr,
                                     struct rte_ipv4_hdr *ipv4Hdr, void *data, int size) {

    if (tcpHdr->tcp_flags != RTE_TCP_SYN_FLAG) {
        return NULL;
    }
    uint32_t iss = 0;
    uint16_t wnd = 64240;
    struct connection *_connection;
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


    // parse tcp option
    int remaining_option_size = (((tcpHdr->data_off) >> 4) << 2) - 20;
    printf("remaining option size is %d\n", remaining_option_size);
    void *option_start = (void *) tcpHdr + 20;
    while (remaining_option_size > 0) {
        uint16_t option_information = rte_be_to_cpu_16(*(uint16_t *) (option_start));
        int option_size = option_information % 256;
        int option_type = option_information / 256;
        if (option_type == 1) {
            remaining_option_size = remaining_option_size - 1;
            option_start += 1;
            continue;
        }
        printf("option size is %u\n", option_size);
        printf("option type is %u\n", option_type);
        remaining_option_size -= option_size;
        option_start += option_size;
    }
    tcp_tx_packets(_tcp, _connection, data, size);

    return _connection;
}
