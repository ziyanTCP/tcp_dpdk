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
struct conn *conn;
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
    int i= 0;
    unsigned int lcore_id;
    RTE_LCORE_FOREACH_SLAVE(lcore_id) {
        (c->tcp_list)[i].dp_quit = true;
        i++;
    }
}

// Q1: how much time to send and recieve?
int main(int argc, char *argv[]) {
    struct rte_mempool *mbuf_pool;
    unsigned nb_ports;
    uint16_t portid;

    c = config_init("/home/ziyan/project/tcp/src/config/run.txt");


    int status;
    /* Connectivity */
    conn = conn_init();
    if (conn == NULL) {
        printf("Error: Connectivity initialization failed (%d)\n",
               status);
        return status;
    }

    signal(SIGINT, exit_stats);
    signal(SIGTERM, exit_stats);

    while (!(c->cp_quit)) {
        conn_poll_for_conn(conn);

        conn_poll_for_msg(conn);
    }
    conn_free(conn);
    rte_eal_mp_wait_lcore();
    return 0;
}
