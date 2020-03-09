/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_n1sdp_scp2pcc.h>

#include <fwk_module.h>

#include <stdint.h>

#define MSG_ALIVE_ADDRESS    0xB3FFF000
#define MSG_RX_BUF_ADDRESS   0xB3FFF104
#define MSG_TX_BUF_ADDRESS   0xB3FFF004
#define MSG_NUM_TX_MESSAGES  8
#define MSG_NUM_RX_MESSAGES  8

const struct fwk_module_config config_n1sdp_scp2pcc = {
    .data = &((struct mem_msg_config_st) {
            .shared_alive_address = (unsigned volatile int *)MSG_ALIVE_ADDRESS,
            .shared_tx_buffer = (uintptr_t)MSG_TX_BUF_ADDRESS,
            .shared_num_tx = MSG_NUM_TX_MESSAGES,
            .shared_rx_buffer = (uintptr_t)MSG_RX_BUF_ADDRESS,
            .shared_num_rx = MSG_NUM_RX_MESSAGES,
        }),
};
