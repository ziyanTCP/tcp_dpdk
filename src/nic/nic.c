//
// Created by ziyan
//

#include "nic/nic.h"
#include "tcp/tcp.h"
#include "utility/utility.h"

void recieve() {

}

void send_p(struct tcp *_tcp, struct connection *connection, void *data, size_t data_size) {
    struct rte_mbuf *pkt;
    union {
        uint64_t as_int;
        struct rte_ether_addr as_addr;
    } dst_eth_addr;
    struct rte_ether_hdr eth_hdr;
    struct rte_ipv4_hdr ipv4Hdr;
    struct rte_tcp_hdr tcpHdr;
    struct rte_mbuf *pkts_burst[1];
    pkt = rte_mbuf_raw_alloc(_tcp->mbuf_pool);
    if (pkt == NULL) {
        printf("trouble at rte_mbuf_raw_alloc\n");
        return;
    }
    rte_pktmbuf_reset_headroom(pkt);

    pkt->data_len = sizeof(struct rte_ether_hdr) + sizeof(struct rte_tcp_hdr) + sizeof(struct rte_ipv4_hdr) +
                    data_size;

//    //copy tcp option
//    if (tcpHdr.tcp_flags & RTE_TCP_SYN_FLAG) {
//        pkt->data_len = sizeof(struct rte_ether_hdr) + sizeof(struct rte_tcp_hdr) + sizeof(struct rte_ipv4_hdr) + 20 +
//                        data_size;
//        rte_memcpy(rte_pktmbuf_mtod_offset(pkt, char *,
//                                           sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr) +
//                                           (((tcpHdr.data_off) >> 4) << 2)),
//                   tcp_mss, (size_t) 4);
//        rte_memcpy(rte_pktmbuf_mtod_offset(pkt, char *,
//                                           sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr) +
//                                           (((tcpHdr.data_off) >> 4) << 2)) + 4,
//                   sack_permitted, (size_t) 2);
//    } else {
//        pkt->data_len = sizeof(struct rte_ether_hdr) + sizeof(struct rte_tcp_hdr) + sizeof(struct rte_ipv4_hdr) + //12 +
//                        data_size;
//    }

    // set up addresses
    eth_hdr.s_addr = _tcp->nic->mac;
    eth_hdr.d_addr = _tcp->nic->dst_mac;
    eth_hdr.ether_type = rte_cpu_to_be_16(0x0800);
    ipv4Hdr = connection->rteIpv4Hdr;
    tcpHdr = connection->rteTcpHdr;
    // copy header to packet in mbuf
    rte_memcpy(rte_pktmbuf_mtod_offset(pkt, char *, 0),
               &eth_hdr, (size_t) sizeof(eth_hdr));
    rte_memcpy(rte_pktmbuf_mtod_offset(pkt, char *, sizeof(struct rte_ether_hdr)),
               &ipv4Hdr, (size_t) sizeof(ipv4Hdr));
    rte_memcpy(rte_pktmbuf_mtod_offset(pkt, char *,
                                       sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr)),
               &tcpHdr, (size_t) sizeof(tcpHdr));
    rte_memcpy(rte_pktmbuf_mtod_offset(pkt, char *,
                                       sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr) +
                                       (((tcpHdr.data_off) >> 4) << 2)),
               data, (size_t) data_size);

    // Add some pkt fields
    pkt->nb_segs = 1;
    pkt->pkt_len = pkt->data_len;
    pkt->ol_flags = 0;

    // Actually send the packet
//    printf("send packet\n");
//    DumpHex(rte_pktmbuf_mtod(pkt, char *), pkt->data_len);
    pkts_burst[0] = pkt;
    const uint16_t nb_tx = rte_eth_tx_burst(0, 0, pkts_burst, 1);
//    printf("%u\n", nb_tx);
    rte_mbuf_raw_free(pkt);
}