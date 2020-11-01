//
// Created by ziyan
//

#include <rte_ethdev.h>
#include <utility/utility.h>
#include <rte_malloc.h>
#include <tcp/tcp_listen.h>
#include "tcp/tcp.h"
#include "rte_jhash.h"

_Noreturn void tcp_rx_packets(struct tcp *_tcp) {
    uint16_t port;
    int i;
    struct rte_ether_hdr *rteEtherHdr;
    struct rte_ipv4_hdr *rteIpv4Hdr;
    struct rte_tcp_hdr *tcpHdr;

    printf("\nCore %u receiving packets. [Ctrl+C to quit]\n",
           rte_lcore_id());

    /* Run until the application is quit or killed. */
    for (;;) {
        RTE_ETH_FOREACH_DEV(port) {

            struct rte_mbuf *bufs[BURST_SIZE];
            const uint16_t nb_rx = rte_eth_rx_burst(port, 0,
                                                    bufs, BURST_SIZE);

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
                    if (rteIpv4Hdr->next_proto_id == IPPROTO_TCP) {
                        DumpHex(rte_pktmbuf_mtod(bufs[i], char *), bufs[i]->pkt_len);
                        tcpHdr = rte_pktmbuf_mtod_offset(bufs[i], struct rte_tcp_hdr *,
                                                         sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr));
                        void *data = rte_pktmbuf_mtod_offset(bufs[i], void *,
                                                             sizeof(struct rte_ether_hdr) +
                                                             sizeof(struct rte_ipv4_hdr)) + sizeof(struct rte_tcp_hdr);
                        int size = bufs[i]->pkt_len - ((sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr)) +
                                                       sizeof(struct rte_tcp_hdr));
                        struct quad q;
                        q.sip = rteIpv4Hdr->src_addr;
                        q.dip = rteIpv4Hdr->dst_addr;
                        q.sport = tcpHdr->src_port;
                        q.dport = tcpHdr->dst_port;
                        int result = rte_hash_lookup(_tcp->rteHash, &q);

                        if (result == -ENOENT) {
                            struct connection *connection = NULL;
                            handle_tcp_listen(_tcp, &connection, q, tcpHdr, rteIpv4Hdr, data, size);
                            rte_hash_add_key_data(_tcp->rteHash, &q, connection);
                        } else {
                            printf("found!\n");
                        }
                    }
                }
                rte_pktmbuf_free(bufs[i]);
            }

        }
    }
}

struct tcp *initialize_tcp(struct rte_mempool * mempool) {
    struct tcp *_tcp = rte_malloc(NULL, sizeof(struct tcp), 0);

    _tcp->mbuf_pool = mempool;
    // https://doc.dpdk.org/api/examples_2l3fwd-power_2main_8c-example.html#_a101

    struct rte_hash_parameters hash_params = {
            .name = NULL,
            .entries = 1024, // number of entries
            .key_len = sizeof(struct quad),
            .hash_func = rte_jhash,
            .hash_func_init_val = 0,
    };

    char s[64];
    /* create hash */
    snprintf(s, sizeof(s), "hash", 0);
    hash_params.name = s;
    hash_params.socket_id = 0;
    _tcp->rteHash = rte_hash_create(&hash_params);
    if (_tcp->rteHash == NULL)
        rte_exit(EXIT_FAILURE, "Unable to create the hash");

    struct nic *_nic = rte_malloc(NULL, sizeof(struct nic), 0);
    _nic->ip = string_to_ip("192.168.11.11");
    char mac[] = "24:4b:fe:5b:3e:6e";
    struct rte_ether_addr eth;
    rte_ether_unformat_addr(mac, &eth); //fake a mac address
    _nic->mac = eth;
    char destination_mac[]="24:4b:fe:5b:3e:5e";
    rte_ether_unformat_addr(destination_mac, &eth); //fake a mac address
    _nic->dst_mac = eth;
    _tcp->nic = _nic;
    return _tcp;
}

void tcp_tx_packets(struct tcp *_tcp, struct connection *_connection) {
    _connection->rteTcpHdr.sent_seq = rte_cpu_to_be_32(_connection->sendSequenceSpace.nxt);
    _connection->rteTcpHdr.recv_ack = rte_cpu_to_be_32(_connection->receiveSequenceSpace.nxt);
    _connection->rteTcpHdr.data_off = 5<<4;
    uint16_t size = sizeof(struct rte_tcp_hdr) + sizeof(struct rte_ipv4_hdr);

    _connection->rteIpv4Hdr.type_of_service = 0;
    _connection->rteIpv4Hdr.time_to_live = IP_DEFTTL;
    _connection->rteIpv4Hdr.version_ihl = IP_VHL_DEF;
    _connection->rteIpv4Hdr.next_proto_id = IPPROTO_TCP;
    _connection->rteIpv4Hdr.packet_id = 0;
    _connection->rteIpv4Hdr.total_length = rte_cpu_to_be_16(size);
    //calculate ip checksum
    _connection->rteIpv4Hdr.hdr_checksum = 0;
    _connection->rteTcpHdr.cksum = 0;
    _connection->rteIpv4Hdr.hdr_checksum = rte_ipv4_cksum(&_connection->rteIpv4Hdr);
    _connection->rteTcpHdr.cksum = rte_ipv4_udptcp_cksum(&_connection->rteIpv4Hdr, &_connection->rteTcpHdr);

    send_p(_tcp, _connection);
}