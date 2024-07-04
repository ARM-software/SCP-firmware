/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Register definitions for the DVFS handler controller in LCP.
 */

#ifndef DVFS_HANDLER_REG_INTERNAL_H
#define DVFS_HANDLER_REG_INTERNAL_H

#include <fwk_id.h>
#include <fwk_macros.h>

#include <stddef.h>
#include <stdint.h>

// clang-format off
struct dvfs_frame {
            uint8_t  reserved0[0x4 - 0x0];
    FWK_R   uint32_t status;
            uint8_t  reserved1[0x10 - 0x8];
    FWK_RW  uint32_t handshake_trigger;
            uint8_t  reserved2[0x20 - 0x14];
    FWK_RW  uint32_t policy_voltage;
            uint8_t  reserved3[0x30 - 0x24];
    FWK_R   uint32_t policy_voltage_status;
            uint8_t  reserved4[0x38 - 0x34];
    FWK_R   uint32_t policy_voltage_status_raw;
            uint8_t  reserved5[0x40 - 0x3C];
    FWK_RW  uint32_t policy_frequency;
            uint8_t  reserved6[0x50 - 0x44];
    FWK_R   uint32_t policy_frequency_status;
            uint8_t  reserved7[0x58 - 0x54];
    FWK_R   uint32_t policy_frequency_status_raw;
            uint8_t  reserved8[0x60 - 0x5C];
    FWK_RW  uint32_t policy_retention_voltage;
            uint8_t  reserved9[0x70 - 0x64];
    FWK_R   uint32_t policy_retention_voltage_status;
            uint8_t  reserved10[0x78 - 0x74];
    FWK_R   uint32_t policy_retention_voltage_status_raw;
            uint8_t  reserved11[0x80 - 0x7C];
    FWK_RW  uint32_t interrupt_status;
    FWK_RW  uint32_t interrupt_mask;
            uint8_t  reserved12[0xFC8 - 0x88];
    FWK_R   uint32_t impl_id;
    FWK_R   uint32_t arch_id;
};
// clang-format on

#endif /* DVFS_HANDLER_REG_INTERNAL_H */
