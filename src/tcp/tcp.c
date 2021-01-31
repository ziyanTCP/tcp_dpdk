//
// Created by ziyan
//

#include <rte_ethdev.h>
#include <utility/utility.h>
#include <rte_malloc.h>
#include <tcp/tcp_listen.h>
#include "tcp/tcp.h"
#include "rte_jhash.h"
#include "tcp/tcp_syn_recv.h"
#include "tcp/tcp_establish.h"
#include "tcp/tcp_last_ack.h"
#include "tcp/tcp_syn_sent.h"
#include "tcp/tcp_fin_wait1.h"
#include "tcp/tcp_fin_wait2.h"

_Noreturn void tcp_rx_packets(struct tcp *_tcp) {
    uint16_t port;
    int i;
    struct rte_ether_hdr *rteEtherHdr;
    struct rte_ipv4_hdr *rteIpv4Hdr;
    struct rte_tcp_hdr *tcpHdr;

    printf("\nCore %u is running\n",
           rte_lcore_id());

    /* Run until the application is quit or killed. */
    for (;;) {

        struct rte_mbuf *bufs[1000];
        const uint16_t nb_rx = rte_eth_rx_burst(port, _tcp->queue_id,
                                                bufs, 1000);

        if (unlikely(nb_rx == 0))
            continue;

//            packet_count += nb_rx;

        //printf("received %d packets:\n",nb_rx);

        for (i = 0; i < nb_rx; ++i) {

//                printf("----->processing packet %d\n",i);
//                printf("----->pkt_len=%d\n",bufs[i]->pkt_len);
            rteEtherHdr = rte_pktmbuf_mtod(bufs[i], struct rte_ether_hdr *);
            if (!rte_is_same_ether_addr(&rteEtherHdr->d_addr, &_tcp->nic->mac)) {
                continue;
            }
            if (rteEtherHdr->ether_type == rte_be_to_cpu_16(RTE_ETHER_TYPE_IPV4)) {
                rteIpv4Hdr = rte_pktmbuf_mtod_offset(bufs[i], struct ipv4_hdr *, sizeof(struct rte_ether_hdr));
                if (_tcp->nic->ip != rteIpv4Hdr->dst_addr) {
//                        DumpHex(rte_pktmbuf_mtod(bufs[i], char *), bufs[i]->pkt_len);
//                        printf("drop the ip packet");
//                    continue;
                }

                if (rteIpv4Hdr->next_proto_id == IPPROTO_TCP) {
//                    DumpHex(rte_pktmbuf_mtod(bufs[i], char *), bufs[i]->pkt_len);
                    tcpHdr = rte_pktmbuf_mtod_offset(bufs[i], struct rte_tcp_hdr *,
                                                     sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr));
                    void *data = rte_pktmbuf_mtod_offset(bufs[i], void *,
                                                         sizeof(struct rte_ether_hdr) +
                                                         sizeof(struct rte_ipv4_hdr)) +
                                 (((tcpHdr->data_off) >> 4) << 2);
//                        printf("total length is %u\n", rte_be_to_cpu_16(rteIpv4Hdr->total_length));
//                        printf("size of ipv4 hdr is %lu\n", sizeof(struct rte_ipv4_hdr));
//                        printf("size of ipv4 hdr is %u\n", (tcpHdr->data_off)>>4 *4);


                    int size = rte_be_to_cpu_16(rteIpv4Hdr->total_length) - sizeof(struct rte_ipv4_hdr) -
                               (((tcpHdr->data_off) >> 4) << 2);
                    struct quad q;
                    q.sip = rteIpv4Hdr->src_addr;
                    q.dip = rteIpv4Hdr->dst_addr;
                    q.sport = tcpHdr->src_port;
                    q.dport = tcpHdr->dst_port;
                    struct connection *connection = NULL;
                    int result = rte_hash_lookup_data(_tcp->rteHash, &q, (void **) &connection);

                    if (result == -ENOENT) {
                        connection = handle_tcp_listen(_tcp, q, tcpHdr, rteIpv4Hdr, data, size);
                        if (connection != NULL) {
                            rte_hash_add_key_data(_tcp->rteHash, &q, (void *) connection);
                        }
                    } else {
//                            printf("found!\n");
                        switch (connection->tcpState) {
                            case TCP_SYN_RECV:
                                handle_tcp_syn_recv(_tcp, connection, q, tcpHdr, rteIpv4Hdr, data, size);
                                break;
                            case TCP_ESTABLISHED:
                                handle_tcp_establish(_tcp, connection, q, tcpHdr, rteIpv4Hdr, data, size);
                                break;
                            case TCP_LAST_ACK:
                                handle_tcp_last_ack(_tcp, connection, q, tcpHdr, rteIpv4Hdr, data, size);
                                // rte_hash_
                                rte_free(connection);
                                rte_hash_del_key(_tcp->rteHash, &q);
                                break;
                            case TCP_SYN_SENT:
                                handle_tcp_syn_sent(_tcp, connection, q, tcpHdr, rteIpv4Hdr, data, size);
                                break;
                            case TCP_FIN_WAIT1:
                                handle_tcp_fin_wait1(_tcp, connection, q, tcpHdr, rteIpv4Hdr, data, size);
                                break;
                            case TCP_FIN_WAIT2:
                                handle_tcp_fin_wait2(_tcp, connection, q, tcpHdr, rteIpv4Hdr, data, size);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
            rte_pktmbuf_free(bufs[i]);
        }
    }
}

struct tcp *initialize_tcp(struct rte_mempool *mempool, struct tcp *_tcp, unsigned int lcore_id) {
    _tcp->stats.first_connection = 0;
    _tcp->stats.first_connection = 0;
    _tcp->stats.connection_count = 0;
    // https://doc.dpdk.org/api/examples_2l3fwd-power_2main_8c-example.html#_a101

    struct rte_hash_parameters hash_params = {
            .entries = 128, // number of entries
            .key_len = sizeof(struct quad),
            .hash_func = rte_jhash,
            .hash_func_init_val = 0,
    };

    // number of connections
    char s[64];
    /* create hash */
    snprintf(s, sizeof(s), "hash %d", lcore_id);
    hash_params.name = s; // each hash table should have a different name
    hash_params.socket_id = 0;
    _tcp->rteHash = rte_hash_create(&hash_params);
    if (_tcp->rteHash == NULL)
        rte_exit(EXIT_FAILURE, "Unable to create the hash");

    struct nic *_nic = rte_malloc(NULL, sizeof(struct nic), 0);
    _nic->ip = rte_cpu_to_be_32(string_to_ip("192.168.11.11"));
    char mac[] = "24:4b:fe:5b:3e:6e";
    struct rte_ether_addr eth;
    rte_ether_unformat_addr(mac, &eth); //fake a mac address
    _nic->mac = eth;
    // char destination_mac[] = "24:4d:54:25:51:3e";
    char destination_mac[] = "24:4b:fe:5b:3e:5e";
    rte_ether_unformat_addr(destination_mac, &eth); //fake a mac address

    _nic->dst_mac = eth;
    _tcp->nic = _nic;
    return _tcp;
}

void tcp_tx_packets(struct tcp *_tcp, struct connection *_connection, void *data, size_t data_size) {
    _connection->rteTcpHdr.sent_seq = rte_cpu_to_be_32(_connection->sendSequenceSpace.nxt);
    _connection->rteTcpHdr.recv_ack = rte_cpu_to_be_32(_connection->receiveSequenceSpace.nxt);
//    if ((_connection->rteTcpHdr.tcp_flags & RTE_TCP_SYN_FLAG) != 0) {
//        _connection->rteTcpHdr.data_off = 10 << 4;
//    } else {
//        _connection->rteTcpHdr.data_off = 8 << 4;
//    }

    _connection->rteTcpHdr.data_off = 5 << 4;
    uint16_t size = sizeof(struct rte_tcp_hdr) + sizeof(struct rte_ipv4_hdr) + data_size;

    _connection->rteIpv4Hdr.fragment_offset = rte_cpu_to_be_16(0x4000);
    _connection->rteIpv4Hdr.packet_id = rte_cpu_to_be_16(rte_be_to_cpu_16(_connection->rteIpv4Hdr.packet_id) + 1);
    _connection->rteIpv4Hdr.time_to_live = IP_DEFTTL;
    _connection->rteIpv4Hdr.version_ihl = IP_VHL_DEF;
    _connection->rteIpv4Hdr.next_proto_id = IPPROTO_TCP;
    _connection->rteIpv4Hdr.total_length = rte_cpu_to_be_16(size);
    //calculate ip checksum
    _connection->rteIpv4Hdr.hdr_checksum = 0;
    _connection->rteTcpHdr.cksum = 0;

    _connection->rteIpv4Hdr.hdr_checksum = rte_ipv4_cksum(&_connection->rteIpv4Hdr);
    _connection->rteTcpHdr.cksum = rte_ipv4_udptcp_cksum(&_connection->rteIpv4Hdr, &_connection->rteTcpHdr);

    send_p(_tcp, _connection, data, data_size);

    if ((_connection->rteTcpHdr.tcp_flags & RTE_TCP_SYN_FLAG) != 0) {
        _connection->sendSequenceSpace.nxt = _connection->sendSequenceSpace.nxt + 1;
        _connection->rteTcpHdr.tcp_flags = _connection->rteTcpHdr.tcp_flags & ~RTE_TCP_SYN_FLAG;
    }

    if ((_connection->rteTcpHdr.tcp_flags & RTE_TCP_FIN_FLAG) != 0) {
        _connection->sendSequenceSpace.nxt = _connection->sendSequenceSpace.nxt + 1;
        _connection->rteTcpHdr.tcp_flags = _connection->rteTcpHdr.tcp_flags & ~RTE_TCP_FIN_FLAG;
    }
}

bool segment_check(struct connection *_connection, uint32_t slen, uint32_t seqn) {
    uint32_t wnd = _connection->receiveSequenceSpace.nxt + _connection->receiveSequenceSpace.wnd;
    if (slen == 0) {
        if (_connection->receiveSequenceSpace.wnd == 0) {
            return (seqn == _connection->receiveSequenceSpace.nxt);
        } else {
            return is_between_wrapped(_connection->receiveSequenceSpace.nxt - 1, seqn, wnd);
        }
    } else {
        if (_connection->receiveSequenceSpace.wnd == 0) {
            return false;
        } else {
            return true; // more rigours check later
        }
    }
}

// From RFC1323:
//     TCP determines if a data segment is "old" or "new" by testing
//     whether its sequence number is within 2**31 bytes of the left edge
//     of the window, and if it is not, discarding the data as "old".  To
//     insure that new data is never mistakenly considered old and vice-
//     versa, the left edge of the sender's window has to be at most
//     2**31 away from the right edge of the receiver's window.
bool wrapping_lt(uint32_t lhs, uint32_t rhs) {
    return (lhs - rhs) > (1 << 31);
}

//strictly less than
bool is_between_wrapped(uint32_t start, uint32_t x, uint32_t end) {
    return wrapping_lt(start, x) && wrapping_lt(x, end);
}

void active_connect(struct tcp *_tcp, rte_be32_t dip, rte_be16_t dport, rte_be16_t sport) {
    struct quad q;
    q.sport = dport;
    q.sip = dip;
    q.dport = sport; // hard-coded for now
    q.dip = _tcp->nic->ip;

    struct connection *_connection;
    int result = rte_hash_lookup_data(_tcp->rteHash, &q, (void **) &_connection);
    if (result == -ENOENT) {

        uint32_t iss = 0;
        uint16_t wnd = 64240;

        _connection = rte_malloc(NULL, sizeof(struct connection), 0);
        _connection->q = q;

        _connection->sendSequenceSpace.una = iss;
        _connection->sendSequenceSpace.nxt = iss;
        _connection->sendSequenceSpace.wnd = wnd;
        _connection->sendSequenceSpace.up = false;
        _connection->sendSequenceSpace.wl1 = 0;
        _connection->sendSequenceSpace.wl2 = 0;
        _connection->sendSequenceSpace.iss = iss;

        _connection->receiveSequenceSpace.irs = iss;
        _connection->receiveSequenceSpace.nxt = iss;
        _connection->receiveSequenceSpace.wnd = wnd;
        _connection->receiveSequenceSpace.up = false;
        _connection->rteIpv4Hdr.src_addr = q.dip;
        _connection->rteIpv4Hdr.dst_addr = q.sip;
        _connection->rteTcpHdr.src_port = q.dport;
        _connection->rteTcpHdr.dst_port = q.sport;
        _connection->rteTcpHdr.sent_seq = 0;
        _connection->rteTcpHdr.rx_win = rte_cpu_to_be_16(wnd);
        _connection->rteTcpHdr.tcp_flags = RTE_TCP_SYN_FLAG;
        _connection->tcpState = TCP_SYN_SENT;

        tcp_tx_packets(_tcp, _connection, NULL, 0);
        rte_hash_add_key_data(_tcp->rteHash, &q, (void *) _connection);
        printf("connection added");
    } else {
        // altrady has that connection
        return;
    }
}