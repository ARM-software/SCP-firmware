/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MHU2_H
#define MHU2_H

#include <fwk_macros.h>

#include <stdint.h>

#define CHANNEL_MAX 124

struct mhu2_id_reg {
    FWK_R  uint32_t  PID4;
           uint8_t   RESERVED1[0x10 - 0x4];
    FWK_R  uint32_t  PID0;
    FWK_R  uint32_t  PID1;
    FWK_R  uint32_t  PID2;
    FWK_R  uint32_t  PID3;
    FWK_R  uint32_t  COMPID0;
    FWK_R  uint32_t  COMPID1;
    FWK_R  uint32_t  COMPID2;
    FWK_R  uint32_t  COMPID3;
};

struct mhu2_send_channel_reg {
    FWK_R  uint32_t  STAT;
           uint8_t   RESERVED0[0xC - 0x4];
    FWK_W  uint32_t  STAT_SET;
           uint8_t   RESERVED1[0x20 - 0x10];
};

struct mhu2_send_reg {
    struct mhu2_send_channel_reg channel[CHANNEL_MAX];
    FWK_R  uint32_t  MSG_NO_CAP;
    FWK_RW uint32_t  RESP_CFG;
    FWK_RW uint32_t  ACCESS_REQUEST;
    FWK_R  uint32_t  ACCESS_READY;
    FWK_R  uint32_t  INT_ACCESS_STAT;
    FWK_W  uint32_t  INT_ACCESS_CLR;
    FWK_W  uint32_t  INT_ACCESS_EN;
           uint8_t   RESERVED0[0xFD0 - 0xF9C];
    struct mhu2_id_reg id;
};

struct mhu2_recv_channel_reg {
    FWK_R  uint32_t  STAT;
    FWK_R  uint32_t  STAT_PEND;
    FWK_W  uint32_t  STAT_CLEAR;
           uint8_t   RESERVED0[0x10 - 0x0C];
    FWK_R  uint32_t  MASK;
    FWK_W  uint32_t  MASK_SET;
    FWK_W  uint32_t  MASK_CLEAR;
           uint8_t   RESERVED1[0x20 - 0x1C];
};

struct mhu2_recv_reg {
    struct mhu2_recv_channel_reg channel[CHANNEL_MAX];
    FWK_R  uint32_t  MSG_NO_CAP;
           uint8_t   RESERVED0[0xFD0 - 0xF84];
    struct mhu2_id_reg id;
};

#endif /* MHU2_H */
