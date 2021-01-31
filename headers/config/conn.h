/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2018 Intel Corporation
 */

#ifndef __INCLUDE_CONN_H__
#define __INCLUDE_CONN_H__

#include <stdint.h>
#include <stddef.h>
#include "conn.h"
typedef void (*conn_msg_handle_t)(char *msg_in,
                                  char *msg_out,
                                  size_t msg_out_len_max, int fd_client);

static const char welcome[] =
        "\n"
        "Welcome to TCP control panel!\n"
        "\n";

static const char prompt[] = "TCP> ";

struct conn {
    char *welcome;
    char *prompt;
    char *buf;
    char *msg_in;
    char *msg_out;
    size_t buf_size;
    size_t msg_in_len_max;
    size_t msg_out_len_max;
    size_t msg_in_len;
    int fd_server;
    int fd_client_group;
    conn_msg_handle_t msg_handle;
};


#ifndef CONN_WELCOME_LEN_MAX
#define CONN_WELCOME_LEN_MAX                               1024
#endif

#ifndef CONN_PROMPT_LEN_MAX
#define CONN_PROMPT_LEN_MAX                                16
#endif



struct conn_params {
	const char *welcome;
	const char *prompt;
	const char *addr;
	uint16_t port;
	size_t buf_size;
	size_t msg_in_len_max;
	size_t msg_out_len_max;
	conn_msg_handle_t msg_handle;
};

struct conn *
conn_init();

void
conn_free(struct conn *conn);

int
conn_poll_for_conn(struct conn *conn);

int
conn_poll_for_msg(struct conn *conn);

#endif
