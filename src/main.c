#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <rte_mbuf.h>
#include "rte_byteorder.h"
#include "config/config.h"
#include "tcp/tcp.h"
#include <unistd.h>
#include "config/conn.h"

struct config *c;

struct tcp *_tcp;

void exit_stats(int sig);

void exit_stats(int sig) {
//    printf("Caught signal %d\n", sig);
//    printf("Total received packets: %lu\n", packet_count);
//    printf("Total connections: %lu\n", _tcp->stats.connection_count);
//    printf("Connections first: %lu\n", _tcp->stats.first_connection);
//    printf("Connections last: %lu\n", _tcp->stats.last_connection);
//    printf("Connections per second: %f\n", (double) (_tcp->stats.connection_count) /
//                                         ((double) (_tcp->stats.last_connection - _tcp->stats.first_connection) /
//                                          (CLOCKS_PER_SEC)));
    c->cp_quit = true;
}

// Q1: how much time to send and recieve?
int main(int argc, char *argv[]) {
    struct rte_mempool *mbuf_pool;
    unsigned nb_ports;
    uint16_t portid;

    c = config_init("/home/ziyan/project/tcp/src/config/run.txt");

    struct conn *conn;
    int status;
    /* Connectivity */
    conn = conn_init();
    if (conn == NULL) {
        printf("Error: Connectivity initialization failed (%d)\n",
               status);
        return status;
    }

    signal(SIGINT, exit_stats);

    while (!(c->cp_quit)){
        conn_poll_for_conn(conn);

        conn_poll_for_msg(conn);

//		kni_handle_request();
    }

    close(conn->fd_server);

//    /* Initialize the Environment Abstraction Layer (EAL). */
//    int ret = rte_eal_init(argc, argv);
//    if (ret < 0)
//        rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");
//
//    argc -= ret;
//    argv += ret;
//
//    nb_ports = rte_eth_dev_count_avail();
//    printf("rte_eth_dev_count_avail()=%d\n", nb_ports);
//
//    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
//                                        MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE,
//                                        rte_socket_id());
//    if (mbuf_pool == NULL)
//        rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
//
//    /* Initialize all ports. */
//    RTE_ETH_FOREACH_DEV(portid)if (port_init(portid, mbuf_pool) != 0)
//            rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu16 "\n",
//                     portid);
//
//    _tcp = initialize_tcp(mbuf_pool);
//    signal(SIGINT, exit_stats);





//    rte_be32_t dip = rte_cpu_to_be_32(string_to_ip("192.168.11.12"));
//    rte_be32_t dport = rte_cpu_to_be_16(2000); //
//    rte_be32_t sport = rte_cpu_to_be_16(8000); // the port of this program
//    sleep(3);
//
//    active_connect(_tcp, dip, dport, sport);
//    tcp_rx_packets(_tcp);
    return 0;
}
