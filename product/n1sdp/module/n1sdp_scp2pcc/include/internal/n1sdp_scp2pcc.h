/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      N1SDP SCP to PCC message transfer prototypes.
 */

#ifndef INTERNAL_N1SDP_SCP2PCC_H
#define INTERNAL_N1SDP_SCP2PCC_H

#include <stdint.h>

#define MSG_PAYLOAD_SIZE         16
#define MSG_UNUSED_MESSAGE_TYPE  0xFFFF
#define MSG_ALIVE_VALUE          0xDEADBEEF

#define SCP2PCC_TYPE_SHUTDOWN    0x0001
#define SCP2PCC_TYPE_REBOOT      0x0002

struct mem_msg_packet_st {
    /* Message type, lower 16 bits only. */
    unsigned int type;
    /* Valid payload size, lower 16 bits only. */
    unsigned int size;
    /* Sequence field used to process packets in proper order. */
    unsigned int sequence;
    /* Data payload. */
    uint8_t payload[MSG_PAYLOAD_SIZE];
} __attribute__((packed));

#endif /* INTERNAL_N1SDP_SCP2PCC_H */
