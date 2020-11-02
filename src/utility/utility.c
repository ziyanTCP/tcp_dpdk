//
// Created by ziyan
//

#include "utility/utility.h"
#include "rte_ip.h"

void DumpHex(const void *data, size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        printf("%02X ", ((const unsigned char *) data)[i]);
        if (((const unsigned char *) data)[i] >= ' ' && ((const unsigned char *) data)[i] <= '~') {
            ascii[i % 16] = ((const unsigned char *) data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i + 1) % 8 == 0 || i + 1 == size) {
            printf(" ");
            if ((i + 1) % 16 == 0) {
                printf("|  %s \n", ascii);
            } else if (i + 1 == size) {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8) {
                    printf(" ");
                }
                for (j = (i + 1) % 16; j < 16; ++j) {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}

// convert a quad-dot IP string to uint32_t IP address
rte_le32_t string_to_ip(char *s) {
    unsigned char a[4];
    int rc = sscanf(s, "%hhd.%hhd.%hhd.%hhd",a+0,a+1,a+2,a+3);
    if(rc != 4){
        fprintf(stderr, "bad source IP address format. Use like: -s 198.19.111.179\n");
        exit(1);
    }

    return
            (uint32_t)(a[0]) << 24 |
            (uint32_t)(a[1]) << 16 |
            (uint32_t)(a[2]) << 8 |
            (uint32_t)(a[3]);

}

// convert six colon separated hex bytes string to uint64_t Ethernet MAC address
rte_le64_t string_to_mac(char *s) {
    unsigned char a[6];
    int rc = sscanf(s, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                    a + 0, a + 1, a + 2, a + 3, a + 4, a + 5);
    if(rc !=6 ){
        fprintf(stderr, "bad MAC address format. Use like: -m 0a:38:ca:f6:f3:20\n");
        exit(1);
    }

    return
            (uint64_t)(a[0]) << 40 |
            (uint64_t)(a[1]) << 32 |
            (uint64_t)(a[2]) << 24 |
            (uint64_t)(a[3]) << 16 |
            (uint64_t)(a[4]) << 8 |
            (uint64_t)(a[5]);
}

uint32_t wrapping_add(uint32_t add1, uint32_t add2){
       return (add1+add2);
}