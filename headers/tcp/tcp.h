//
// Created by ziyan
//

#ifndef TCP_TCP_H
#define TCP_TCP_H

#include <stdbool.h>
#include "rte_hash.h"
#include "rte_ip.h"
#include "config/config.h"
#include "nic/nic.h"
#include "time.h"
#include <unistd.h>

#ifndef IPv4_BYTES
#define IPv4_BYTES_FMT "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8
#define IPv4_BYTES(addr) \
        (uint8_t) (((addr) >> 24) & 0xFF),\
        (uint8_t) (((addr) >> 16) & 0xFF),\
        (uint8_t) (((addr) >> 8) & 0xFF),\
        (uint8_t) ((addr) & 0xFF)
#endif

//#DEFINE TCP_NON_OP 0x01
//#DEFINE TCP_TIMESTAMP 0x0a08
//#DEFINE TCP_MSS 0x0204
//const uint8_t no_op[] = {
//        0x01
//};
//const uint8_t tcp_timestamp[] = {
//        0x08, 0x0a, 0x00, 0x00, 0x00, 0x00,
//        0x00, 0x00, 0x00, 0x00
//};
//const uint8_t tcp_mss[] = {
//        0x02, 0x04, 0xff, 0xd7
//};
//
//const uint8_t sack_permitted[] = {
//        0x04, 0x02
//};
//
//const uint8_t
//struct tcp_option{
//    uint8_t option_type;
//    uint8_t option_length;
//};


enum TCP_STATE {
    TCP_ESTABLISHED = 1,
    TCP_SYN_SENT,
    TCP_SYN_RECV,
    TCP_FIN_WAIT1,
    TCP_FIN_WAIT2,
    TCP_TIME_WAIT,
    TCP_CLOSE,
    TCP_CLOSE_WAIT,
    TCP_LAST_ACK,
    TCP_LISTEN,
    TCP_CLOSING,    /* Now a valid state */
    TCP_MAX_STATES  /* Leave at the end! */
};

static char *TCP_STATE_string[] = {"TCP_ESTABLISHED", "TCP_SYN_SENT", "TCP_SYN_RECV", "TCP_FIN_WAIT1", "TCP_FIN_WAIT2",
                                  "TCP_TIME_WAIT", "TCP_CLOSE", "TCP_CLOSE_WAIT", "TCP_LAST_ACK", "TCP_LISTEN",
                                  "TCP_CLOSING",
                                  "TCP_MAX_STATES"};

struct tcp_statistics {
    u_long connection_count;
    clock_t first_connection;
    clock_t last_connection;
};


struct tcp {
    struct nic *nic;
    struct rte_hash *rteHash;
    struct tcp_statistics stats;
    int core_id;
    int queue_id;
    long rxn; // number of packets recieved
    long txn; // number of packets sent
    int max_pkt_burst;
    struct rte_timer timer;
    volatile bool dp_quit;
    struct rte_mempool *mbuf_pool;
};

// size = 2*(32+16)/8 = 12 byte
// compress into 64 bit
struct quad {
    rte_be32_t sip;
    rte_be16_t sport;
    rte_be32_t dip;
    rte_be16_t dport;
};

struct send_sequence_space {
    uint32_t una; // send unacknowledged
    uint32_t nxt; // send next
    uint16_t wnd; // send window
    bool up; // send urgent pointer
    uint64_t wl1; // segment sequence number used for last window update
    uint64_t wl2; // segment acknowledgment number used for last window update
    uint32_t iss; // initial send sequence number
};

struct receive_sequence_space {
    uint32_t nxt; // receive next
    uint16_t wnd; // receive window
    bool up; // receive urgent pointer
    uint32_t irs; // initial receive sequence number
};

struct connection {
    struct quad q;
    enum TCP_STATE tcpState;
    struct send_sequence_space sendSequenceSpace;
    struct receive_sequence_space receiveSequenceSpace;
    struct rte_ipv4_hdr rteIpv4Hdr;
    struct rte_tcp_hdr rteTcpHdr;
    char payload[100];
};

_Noreturn void tcp_rx_packets(struct tcp *_tcp);

void tcp_tx_packets(struct tcp *_tcp, struct connection *_connection, void *data, size_t size);

struct tcp *initialize_tcp(struct rte_mempool *mempool, struct tcp *_tcp, unsigned int lcore_id);

bool segment_check(struct connection *_connection, uint32_t slen, uint32_t seqn);

bool wrapping_lt(uint32_t lhs, uint32_t rhs);

bool is_between_wrapped(uint32_t start, uint32_t x, uint32_t end);

void active_connect(struct tcp *_tcp, rte_be32_t dip, rte_be16_t dport, rte_be16_t sport);

void active_close(struct tcp *_tcp, struct connection * c);

void dump_connection(struct connection *c);

void dump_hashtable(struct rte_hash *rteHash);

#endif //TCP_TCP_H
