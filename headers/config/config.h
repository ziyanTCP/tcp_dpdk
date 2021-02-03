//
// Created by ziyan on 12/24/20.
//

#ifndef MULTICORE_FWDING_DPDK_CONFIG_H
#define MULTICORE_FWDING_DPDK_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <rte_timer.h>
#include "rte_cycles.h"
//#include "fwd.h"
//#include "ddio.h"
//#include "fwd.h"
//#include "cat.h"
#include <bits/types/FILE.h>
#include <stdlib.h>
#include <sys/io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>
#include "tcp/tcp.h"

struct config {
    int argc;
    char **argv;
    int max_pkt_burst;
    int mbuf_cache_size;
    int num_mbufs;
    int num_cores;
    uint64_t hz;
    volatile bool dp_quit;
    volatile bool cp_quit;
    struct fwder *f_list;

    struct pci_access *pacc;
    uint8_t nic_bus;
    struct rte_mempool * mbuf_pool;
    struct tcp* tcp_list;

    char src_ip[50];
    char src_mac[50];
    char dst_mac[50];
};
extern struct config *c;
struct config *config_init(char *config_file);
int parse_args(struct config *c);
int port_init(uint16_t port, struct rte_mempool *mbuf_pool, int nb_core, uint16_t nb_rxd, uint16_t nb_txd);
#endif //MULTICORE_FWDING_DPDK_CONFIG_H