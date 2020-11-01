# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2010-2014 Intel Corporation

# binary name
APP = mytcp

# all source are stored in SRCS-y
#SRCS-y := main.c tcp/tcp.c nic/nic.c utility/utility.c

SRCS-y := $(shell find /home/ziyan/project/tcp/src -name "*.c")

RTE_SDK = /home/ziyan/project/dpdk-19.11.5/
# Default target, can be overridden by command line or environment
RTE_TARGET = x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

INC := -I/home/ziyan/project/tcp/headers
CFLAGS += -O0 -g
CFLAGS += $(WERROR_FLAGS)
CFLAGS += $(INC)

include $(RTE_SDK)/mk/rte.extapp.mk