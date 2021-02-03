/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2018 Intel Corporation
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_ethdev.h>

#include "cli/cli.h"

#include "cli/parser.h"
#include "config/config.h"
#include "utility/utility.h"

#ifndef CMD_MAX_TOKENS
#define CMD_MAX_TOKENS     256
#endif

#define MSG_OUT_OF_MEMORY   "Not enough memory.\n"
#define MSG_CMD_UNKNOWN     "Unknown command \"%s\".\n"
#define MSG_CMD_UNIMPLEM    "Command \"%s\" not implemented.\n"
#define MSG_ARG_NOT_ENOUGH  "Not enough arguments for command \"%s\".\n"
#define MSG_ARG_TOO_MANY    "Too many arguments for command \"%s\".\n"
#define MSG_ARG_MISMATCH    "Wrong number of arguments for command \"%s\".\n"
#define MSG_ARG_NOT_FOUND   "Argument \"%s\" not found.\n"
#define MSG_ARG_INVALID     "Invalid value for argument \"%s\".\n"
#define MSG_FILE_ERR        "Error in file \"%s\" at line %u.\n"
#define MSG_FILE_NOT_ENOUGH "Not enough rules in file \"%s\".\n"
#define MSG_CMD_FAIL        "Command \"%s\" failed.\n"

static int
is_comment(char *in) {
    if ((strlen(in) && index("!#%;", in[0])) ||
        (strncmp(in, "//", 2) == 0) ||
        (strncmp(in, "--", 2) == 0))
        return 1;

    return 0;
}

static const char cmd_mempool_help[] =
        "mempool <mempool_name>\n"
        "   buffer <buffer_size>\n"
        "   pool <pool_size>\n"
        "   cache <cache_size>\n"
        "   cpu <cpu_id>\n";


static const char cmd_pipeline_table_meter_profile_add_help[] =
        "pipeline <pipeline_name> table <table_id> meter profile <meter_profile_id>\n"
        "   add srtcm cir <cir> cbs <cbs> ebs <ebs>\n"
        "   | trtcm cir <cir> pir <pir> cbs <cbs> pbs <pbs>\n";


static const char cmd_pipeline_table_meter_profile_delete_help[] =
        "pipeline <pipeline_name> table <table_id>\n"
        "   meter profile <meter_profile_id> delete\n";


static const char cmd_pipeline_table_rule_meter_read_help[] =
        "pipeline <pipeline_name> table <table_id> rule read meter [clear]\n"
        "     match <match>\n";

static const char cmd_pipeline_table_dscp_help[] =
        "pipeline <pipeline_name> table <table_id> dscp <file_name>\n"
        "\n"
        " File <file_name>:\n"
        "   - exactly 64 lines\n"
        "   - line format: <tc_id> <tc_queue_id> <color>, with <color> as: g | y | r\n";


static const char cmd_pipeline_table_rule_ttl_read_help[] =
        "pipeline <pipeline_name> table <table_id> rule read ttl [clear]\n"
        "     match <match>\n";

static const char cmd_pipeline_table_rule_time_read_help[] =
        "pipeline <pipeline_name> table <table_id> rule read time\n"
        "     match <match>\n";


static const char cmd_thread_pipeline_enable_help[] =
        "thread <thread_id> pipeline <pipeline_name> enable\n";

static const char cmd_thread_pipeline_disable_help[] =
        "thread <thread_id> pipeline <pipeline_name> disable\n";


static void
cmd_help(char **tokens, uint32_t n_tokens, char *out, size_t out_size) {
    tokens++;
    n_tokens--;

    if (n_tokens == 0) {
        snprintf(out, out_size,
                 "Type 'help <command>' for details on each command.\n\n"
                 "List of commands:\n"
                 "\tddio\n"
                 "\tddio-disable\n\n");
        return;
    }

    if (strcmp(tokens[0], "mempool") == 0) {
        snprintf(out, out_size, "\n%s\n", cmd_mempool_help);
        return;
    }
    if ((n_tokens == 5) &&
        (strcmp(tokens[2], "meter") == 0) &&
        (strcmp(tokens[3], "profile") == 0) &&
        (strcmp(tokens[4], "add") == 0)) {
        snprintf(out, out_size, "\n%s\n",
                 cmd_pipeline_table_meter_profile_add_help);
        return;
    }

    if ((n_tokens == 5) &&
        (strcmp(tokens[2], "meter") == 0) &&
        (strcmp(tokens[3], "profile") == 0) &&
        (strcmp(tokens[4], "delete") == 0)) {
        snprintf(out, out_size, "\n%s\n",
                 cmd_pipeline_table_meter_profile_delete_help);
        return;
    }

    if ((n_tokens == 5) &&
        (strcmp(tokens[2], "rule") == 0) &&
        (strcmp(tokens[3], "meter") == 0) &&
        (strcmp(tokens[4], "read") == 0)) {
        snprintf(out, out_size, "\n%s\n",
                 cmd_pipeline_table_rule_meter_read_help);
        return;
    }

    if ((n_tokens == 5) &&
        (strcmp(tokens[2], "rule") == 0) &&
        (strcmp(tokens[3], "ttl") == 0) &&
        (strcmp(tokens[4], "read") == 0)) {
        snprintf(out, out_size, "\n%s\n",
                 cmd_pipeline_table_rule_ttl_read_help);
        return;
    }

    if ((n_tokens == 5) &&
        (strcmp(tokens[2], "rule") == 0) &&
        (strcmp(tokens[3], "time") == 0) &&
        (strcmp(tokens[4], "read") == 0)) {
        snprintf(out, out_size, "\n%s\n",
                 cmd_pipeline_table_rule_time_read_help);
        return;
    }
}

void
cli_process(char *in, char *out, size_t out_size, int fd_client) {
    char *tokens[CMD_MAX_TOKENS];
    uint32_t n_tokens = RTE_DIM(tokens);
    int status;

    if (is_comment(in))
        return;

    status = parse_tokenize_string(in, tokens, &n_tokens);
    if (status) {
        snprintf(out, out_size, MSG_ARG_TOO_MANY, "");
        return;
    }

    if (n_tokens == 0)
        return;

    if (strcmp(tokens[0], "help") == 0) {
        cmd_help(tokens, n_tokens, out, out_size);
        return;
    }

    if (strcmp(tokens[0], "ddio") == 0) {
//        snprintf(out, out_size, "enable DDIO!\n");
//        printf("enable ddio\n");
//        c->pacc = init_pci_access();
//
//        /* Define nic_bus and ddio_state */
//        uint8_t nic_bus = 0xaf;
//
//        struct pci_dev *dev = find_ddio_device(nic_bus, c->pacc);
//        print_dev_info(dev, c->pacc);
//        ddio_enable(nic_bus, c->pacc);
//        pci_cleanup(c->pacc);        /* Close everything */
        return;
    }

    if (strcmp(tokens[0], "ddio-disable") == 0) {
//        snprintf(out, out_size, "disable DDIO!\n");
//        printf("enable ddio\n");
//        c->pacc = init_pci_access();
//
//        /* Define nic_bus and ddio_state */
//        uint8_t nic_bus = 0xaf;
//
//        struct pci_dev *dev = find_ddio_device(nic_bus, c->pacc);
//        print_dev_info(dev, c->pacc);
//        ddio_disable(nic_bus, c->pacc);
//        pci_cleanup(c->pacc);        /* Close everything */
        return;
    }

    if (strcmp(tokens[0], "stop") == 0) {
//        snprintf(out, out_size, "stop the forwarding!\n");
//        c->dp_quit = true;
        return;
    }

    if (strcmp(tokens[0], "start") == 0) {
        snprintf(out, out_size, "start the tcp thread!\n");
        int i = 0;
        unsigned int lcore_id;
        RTE_LCORE_FOREACH_SLAVE(lcore_id) {
            rte_eal_remote_launch((lcore_function_t *) tcp_rx_packets, &(c->tcp_list[i]), lcore_id);
            i++;
        }
        return;
    }

    if (strcmp(tokens[0], "configure") == 0) {
//        snprintf(out, out_size, "start the forwarding!\n");
//        c->dp_quit = true;
//        rte_eth_dev_stop(0);
//        port_init( 0, c->mbuf_pool, 1 , 2048, 1024);
//        c->dp_quit = false;
//        rte_eal_mp_wait_lcore();
//        for (int i = 0; i < 1; i++) {
//            c->f_list[i].core_id = 1;
//            rte_eal_remote_launch((lcore_function_t *) fwder_action, &(c->f_list[i]), 1);
//        }
        return;
    }

    if (strcmp(tokens[0], "adjust") == 0) {
//        snprintf(out, out_size, "adjust descriptors!\n");
//        c->dp_quit = true;
//        rte_eth_dev_stop(0);
//        port_init( 0, c->mbuf_pool, 1 , 256, 128);
//        c->dp_quit = false;
//        rte_eal_mp_wait_lcore();
//        for (int i = 0; i < 1; i++) {
//            c->f_list[i].core_id = 1;
//            rte_eal_remote_launch((lcore_function_t *) fwder_action, &(c->f_list[i]), 1);
//        }
//        return;
    }
//    if (strcmp(tokens[0], "close") == 0) {
//
//        close(fd_client);
//        return;
//    }

    if (strcmp(tokens[0], "exit") == 0) {
        int i = 0;
        unsigned int lcore_id;
        RTE_LCORE_FOREACH_SLAVE(lcore_id) {
            c->tcp_list[i].dp_quit = true;
            i++;
        }
        c->cp_quit = true;
//        close(fd_client);
        return;
    }

    if (strcmp(tokens[0], "dump") == 0) {
        snprintf(out, out_size, "dump!\n");
        int i = 0;
        unsigned int lcore_id;
        RTE_LCORE_FOREACH_SLAVE(lcore_id) {
            dump_hashtable(c->tcp_list[i].rteHash);
            i++;
        }
        return;
    }

    // flow source_ip source_port destination_ip destination_port
    // example: flow 192.168.11.111 34962 192.168.11.11 2000
    if (strcmp(tokens[0], "flow") == 0) {
        if (n_tokens != 5) {
            snprintf(out, out_size, "we need 5 tokens!\n");
            return;
        }
        snprintf(out, out_size, "look up the flow...\n");
        struct quad q;
        q.sip = rte_cpu_to_be_32(string_to_ip(tokens[1]));
        q.sport = rte_cpu_to_be_16(atoi(tokens[2]));
        q.dip = rte_cpu_to_be_32(string_to_ip(tokens[3]));
        q.dport = rte_cpu_to_be_16(atoi(tokens[4]));
//        snprintf(out, out_size, "flow!\n");
        struct connection *connection = NULL;
        int i = 0;
        unsigned int lcore_id;
        RTE_LCORE_FOREACH_SLAVE(lcore_id) {
            int result = rte_hash_lookup_data(c->tcp_list[i].rteHash, &q, (void **) &connection);
            if (result == -ENOENT) {
                i++;
                continue;
            } else {
                dump_connection(connection);
                return;
            }
        }
        printf("connection not found\n");
        return;
    }

    // flow source_ip source_port destination_ip destination_port
    // example: close 192.168.11.111 37736 192.168.11.11 2000
    if (strcmp(tokens[0], "close") == 0) {
        if (n_tokens != 5) {
            snprintf(out, out_size, "we need 5 tokens!\n");
            return;
        }
        snprintf(out, out_size, "look up the flow...\n");
        struct quad q;
        q.sip = rte_cpu_to_be_32(string_to_ip(tokens[1]));
        q.sport = rte_cpu_to_be_16(atoi(tokens[2]));
        q.dip = rte_cpu_to_be_32(string_to_ip(tokens[3]));
        q.dport = rte_cpu_to_be_16(atoi(tokens[4]));
//        snprintf(out, out_size, "flow!\n");
        struct connection *connection = NULL;
        int i = 0;
        unsigned int lcore_id;
        RTE_LCORE_FOREACH_SLAVE(lcore_id) {
            int result = rte_hash_lookup_data(c->tcp_list[i].rteHash, &q, (void **) &connection);
            if (result == -ENOENT) {
                i++;
                continue;
            } else {
                active_close(&(c->tcp_list[i]), connection);
                return;
            }
        }
        printf("connection not found\n");
        return;
    }

    if (strcmp(tokens[0], "connect") == 0) {
        snprintf(out, out_size, "connect!\n");
        int i = 0;
        unsigned int lcore_id;
        rte_be32_t dip = rte_cpu_to_be_32(string_to_ip("192.168.11.111"));
        rte_be32_t dport = rte_cpu_to_be_16(3000); //
        rte_be32_t sport = rte_cpu_to_be_16(8000); // the port of this program
        active_connect(&(c->tcp_list[0]), dip, dport, sport);
        return;
    }


    snprintf(out, out_size, MSG_CMD_UNKNOWN, tokens[0]);
}
