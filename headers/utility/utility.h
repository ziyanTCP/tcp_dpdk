//
// Created by ziyan
//

#ifndef TCP_UTILITY_H
#define TCP_UTILITY_H

#define IP_DEFTTL  64   /* from RFC 1340. */
#define IP_VERSION 0x40
#define IP_HDRLEN  0x05 /* default IP header length == five 32-bits words. */
#define IP_VHL_DEF (IP_VERSION | IP_HDRLEN)


#include <glob.h>
#include "rte_byteorder.h"
void DumpHex(const void *data, size_t size);

// convert a quad-dot IP string to uint32_t IP address
rte_le32_t string_to_ip(char *s);

// convert six colon separated hex bytes string to uint64_t Ethernet MAC address
rte_le64_t string_to_mac(char *s);

uint32_t wrapping_add(uint32_t add1, uint32_t add2);

#endif //TCP_UTILITY_H
