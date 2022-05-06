/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      MORELLO SCP to PCC message transfer prototypes.
 */

#ifndef INTERNAL_MORELLO_SCP2PCC_H
#define INTERNAL_MORELLO_SCP2PCC_H

#include <fwk_attributes.h>

#include <stdint.h>

#define MORELLO_SCP2PCC_I2C_ADDRESS 0x24

#define MORELLO_SCP2PCC_MSG_LEN      24U
#define MORELLO_SCP2PCC_MSG_DATA_LEN 16U

/* SCP2PCC Data buffer */
struct FWK_PACKED scp2pcc_msg_st {
    uint8_t opcode;
    union {
        struct FWK_PACKED {
            uint8_t len;
            uint8_t rsvd[6];
            uint8_t data[MORELLO_SCP2PCC_MSG_DATA_LEN];
        } req;
        struct FWK_PACKED {
            uint8_t len;
            uint8_t rsvd[5];
            uint8_t status;
            uint8_t data[MORELLO_SCP2PCC_MSG_DATA_LEN];
        } resp;
    };
};

#endif /* INTERNAL_MORELLO_SCP2PCC_H */
