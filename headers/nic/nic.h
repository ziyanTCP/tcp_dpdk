//
// Created by ziyan
//

#ifndef TCP_NIC_H
#define TCP_NIC_H
#include "rte_ip.h"
#include "rte_ethdev.h"
#include "tcp/tcp.h"
struct nic{
	rte_be32_t ip;
    struct rte_ether_addr mac;
    struct rte_ether_addr dst_mac;
};

struct nic *initialize_nic();
void recieve();
void send_p(struct tcp* _tcp, struct connection* _connection);
#endif //TCP_NIC_H
