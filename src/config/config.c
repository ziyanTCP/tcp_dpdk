#include <rte_ethdev.h>
#include "config/config.h"

static uint8_t rss_intel_key[40] = {0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x5B, 0x5A,
                                    0x6D, 0x5A, 0x6A, 0x5A, 0x6F, 0x5A, 0x6D, 0x5A,
                                    0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
                                    0x6C, 0x5A, 0x6D, 0x5A, 0x7D, 0x5A, 0x6D, 0x5A,
                                    0x6D, 0x2A, 0x6D, 0x5A, 0x6D, 0x5A, 0x4D, 0x5A
};

#define ETH_RSS_E1000_IGB (\
    ETH_RSS_IPV4 | \
    ETH_RSS_NONFRAG_IPV4_TCP| \
    ETH_RSS_NONFRAG_IPV4_UDP| \
    ETH_RSS_IPV6 | \
    ETH_RSS_NONFRAG_IPV6_TCP | \
    ETH_RSS_NONFRAG_IPV6_UDP | \
    ETH_RSS_IPV6_EX | \
    ETH_RSS_IPV6_TCP_EX | \
    ETH_RSS_IPV6_UDP_EX)

#define ETH_RSS_IXGBE ETH_RSS_E1000_IGB
#define RTE_LOGTYPE_FWD 2
static const struct rte_eth_conf port_conf_default = {
        .rxmode = {
                .split_hdr_size = 0,
                .mq_mode = ETH_MQ_RX_RSS,
        },
        .rx_adv_conf = {
                .rss_conf = {
                        .rss_key = rss_intel_key,
                        .rss_key_len = 40,
                        .rss_hf = ETH_RSS_IXGBE,
                },
        },
        .txmode = {
        },
};

// memory pool
// number of cores
// port number
// number of descriptors
int port_init(uint16_t port, struct rte_mempool *mbuf_pool, int nb_core, uint16_t nb_rxd, uint16_t nb_txd) {
    struct rte_eth_conf port_conf = port_conf_default;
    struct rte_eth_txconf txconf;
    struct rte_eth_dev_info dev_info;
    const uint16_t rx_rings = nb_core, tx_rings = nb_core;
    int retval;
    uint16_t q;

    rte_eth_dev_info_get(port, &dev_info);
    if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
        port_conf.txmode.offloads |=
                DEV_TX_OFFLOAD_MBUF_FAST_FREE;

    if (!rte_eth_dev_is_valid_port(port))
        return -1;

    /* Configure the Ethernet device. */
    retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
    if (retval != 0)
        return retval;

    retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
    if (retval != 0)
        return retval;

    for (q = 0; q < rx_rings; q++) {
        retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
                                        rte_eth_dev_socket_id(port), NULL, mbuf_pool);
        if (retval < 0)
            return retval;
    }
    txconf = dev_info.default_txconf;
    txconf.offloads = port_conf.txmode.offloads;
    for (q = 0; q < tx_rings; q++) {
        retval = rte_eth_tx_queue_setup(port, q, nb_txd,
                                        rte_eth_dev_socket_id(port), &txconf);
        if (retval < 0)
            return retval;
    }

    /* Start the Ethernet port. */
    retval = rte_eth_dev_start(port);
    if (retval < 0)
        return retval;

    /* Display the port MAC address. */
    struct rte_ether_addr addr;
    rte_eth_macaddr_get(port, &addr);
    printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
           " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
           port,
           addr.addr_bytes[0], addr.addr_bytes[1],
           addr.addr_bytes[2], addr.addr_bytes[3],
           addr.addr_bytes[4], addr.addr_bytes[5]);

    /* Enable RX in promiscuous mode for the Ethernet device. */
    rte_eth_promiscuous_enable(port);
    return 0;
}

int parse_args(struct config *c) {
    // printf("the remaining command is: ");
    int argc = c->argc;
    char **argv = c->argv;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            //copy the remaining ones
            strcpy(argv[i], argv[0]);
            return i;
        } else {
            if (strcmp(argv[i], "ddio") == 0) {
//                c->pacc = init_pci_access();
//
//                /* Define nic_bus and ddio_state */
//                uint8_t nic_bus=0xaf, ddio_state=1;
//
//                struct pci_dev *dev=find_ddio_device(nic_bus, c->pacc);
//                print_dev_info(dev, c->pacc);
//
//                if(ddio_state) {
//                    ddio_enable(nic_bus, c->pacc);
//                } else {
//                    ddio_disable(nic_bus, c->pacc);
//                }
//
//                pci_cleanup(c->pacc);		/* Close everything */
                continue;
            }

            if (strcmp(argv[i], "src_ip") == 0) {
                printf("src_ip: %s\n", argv[i + 1]);
                strcpy(c->src_ip, argv[i + 1]);
                i++;
                continue;
            }
            if (strcmp(argv[i], "src_mac") == 0) {
                printf("src_mac: %s\n", argv[i + 1]);
                strcpy(c->src_mac, argv[i + 1]);
                i++;
                continue;
            }
            if (strcmp(argv[i], "dst_mac") == 0) {
                printf("dst_mac: %s\n", argv[i + 1]);
                strcpy(c->dst_mac, argv[i + 1]);
                i++;
                continue;
            } else {
                printf("argument: %s\n", argv[i]);
            }
        }
    }
    // printf("\n");
    return -1;
}

struct config *config_init(char *config_file) {
    struct config *c = malloc(sizeof(struct config));

    FILE *fp;
    char *line = NULL;
    int length = 0;
    size_t len = 0;
    size_t read;
    unsigned int lcore_id;

    fp = fopen(config_file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        length += 1;
    }
    fclose(fp);

    printf("the length of the file is %d\n", length);

    fp = fopen(config_file, "r");
    len = 0;
    c->argc = length;
    c->argv = malloc(sizeof(char *) * c->argc);

    for (int i = 0; i < c->argc; i++) {
        (c->argv)[i] = malloc(sizeof(char) * 100);
    }
    fclose(fp);

    fp = fopen(config_file, "r");
    len = 0;
    if (fp == NULL)
        exit(EXIT_FAILURE);

    int i = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        strcpy((c->argv)[i], line);
        i += 1;
    }
    fclose(fp);

    printf("the running command is: ");
    for (int i = 0; i < c->argc; i++) {
        printf("%s ", c->argv[i]);
    }
    printf("\n");

    // initialize EAL
    int ret = rte_eal_init(c->argc, c->argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");

    c->argc -= ret;
    c->argv += ret;

    parse_args(c);
//
//    ret = parse_args(c);
//    if (ret < 0)
//        rte_exit(EXIT_FAILURE, "Invalid l2fwd arguments\n");
//    c->argc -= ret;
//    c->argv += ret;
//
//    ret = cat_init(c->argc, c->argv);
//    if (ret < 0)
//        rte_exit(EXIT_FAILURE, "PQOS: L3CA init failed!\n");
//
//    // The number of cycles in one second.
//    // the value would be 27000000000, since it is Intel(R) Xeon(R) Platinum 8168 CPU @ 2.70GHz
//
//
//    c->hz = rte_get_timer_hz();
//    printf("The number of cycles in one second is %lu\n", c->hz);
    c->max_pkt_burst = 256; // seems to be optimal
    c->mbuf_cache_size = 250;
    c->num_mbufs = 8191 * 16 * 5; // cannot exceed the maximum size of the
    c->dp_quit = false;
    c->cp_quit = false;

    int nb_ports = rte_eth_dev_count_avail();
    if (nb_ports == 0)
        rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");

    c->mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", c->num_mbufs * nb_ports,
                                           c->mbuf_cache_size, 0, RTE_MBUF_DEFAULT_BUF_SIZE,
                                           0);

    int nb_lcore = 0;
    RTE_LCORE_FOREACH_SLAVE(lcore_id) {
        nb_lcore += 1;
    }
    c->num_cores = nb_lcore;

    port_init(0, c->mbuf_pool, nb_lcore, 2048, 1024);

    rte_timer_subsystem_init();

    // initialize tcp
    // each core has one tcp thread
    c->tcp_list = malloc(sizeof(struct tcp) * nb_lcore);
    i = 0;
    lcore_id = 0;
    RTE_LCORE_FOREACH_SLAVE(lcore_id) {
        (c->tcp_list)[i].core_id = lcore_id;
        rte_timer_init(&((c->tcp_list)[i].timer));
        (c->tcp_list)[i].rxn = 0;
        (c->tcp_list)[i].txn = 0;
        (c->tcp_list)[i].dp_quit = false;
        (c->tcp_list)[i].max_pkt_burst = c->max_pkt_burst;
        (c->tcp_list)[i].queue_id = i;
        (c->tcp_list)[i].mbuf_pool = c->mbuf_pool;
        initialize_tcp(c->mbuf_pool, &(c->tcp_list)[i], lcore_id, c->src_ip, c->src_mac, c->dst_mac);
        i++;
    }

    i = 0;
    RTE_LCORE_FOREACH_SLAVE(lcore_id) {
        rte_eal_remote_launch((lcore_function_t *) tcp_rx_packets, &(c->tcp_list[i]), lcore_id);
        i++;
    }
    return c;
}