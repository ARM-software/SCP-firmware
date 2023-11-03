/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Register definitions for the CMN-Cyprus module.
 */

#ifndef CMN_CYPRUS_REG_INTERNAL_H
#define CMN_CYPRUS_REG_INTERNAL_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * Global Configuration Completer register.
 */
// clang-format off
struct cmn_cyprus_cfgm_reg {
    FWK_R   uint64_t  NODE_INFO;
    FWK_RW  uint64_t  PERIPH_ID[4];
    FWK_RW  uint64_t  COMPONENT_ID[2];
            uint8_t   RESERVED0[0x80 - 0x38];
    FWK_R   uint64_t  CHILD_INFO;
            uint8_t   RESERVED1[0x100 - 0x88];
    FWK_R   uint64_t  CHILD_POINTER[256];
};

/*!
 * Node Configuration register.
 */
struct cmn_cyprus_node_cfg_reg {
    FWK_R   uint64_t  NODE_INFO;
            uint8_t   RESERVED0[0x80 - 0x8];
    FWK_R   uint64_t  CHILD_INFO;
            uint8_t   RESERVED1[0x100 - 0x88];
};
// clang-format on

#endif /* CMN_CYPRUS_REG_INTERNAL_H */
