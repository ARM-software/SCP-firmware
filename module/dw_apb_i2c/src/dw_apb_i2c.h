/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for the I2C controller module.
 */

#ifndef DW_APB_I2C_H
#define DW_APB_I2C_H

#include <fwk_macros.h>

#include <stdint.h>

#define I2C_TRANSMIT_BUFFER_LENGTH       16
#define I2C_RECEIVE_BUFFER_LENGTH        16
#define I2C_TIMEOUT_US                   250

/*
 * I2C controller register definitions
 */
struct dw_apb_i2c_reg {
           uint8_t        RESERVED0[0x04 - 0x00];
    FWK_RW uint32_t       IC_TAR;
           uint8_t        RESERVED1[0x10 - 0x08];
    FWK_RW uint32_t       IC_DATA_CMD;
           uint8_t        RESERVED2[0x2C - 0x14];
    FWK_R  uint32_t       IC_INTR_STAT;
    FWK_RW uint32_t       IC_INTR_MASK;
           uint8_t        RESERVED3[0x54 - 0x34];
    FWK_R  uint32_t       IC_CLR_TX_ABRT;
           uint8_t        RESERVED4[0x60 - 0x58];
    FWK_R  uint32_t       IC_CLR_STOP_DET;
           uint8_t        RESERVED5[0x6C - 0x64];
    FWK_RW uint32_t       IC_ENABLE;
    FWK_R  uint32_t       IC_STATUS;
           uint8_t        RESERVED6[0x9C - 0x74];
    FWK_R  uint32_t       IC_ENABLE_STATUS;
           uint8_t        RESERVED7[0x100 - 0xA0];
};

#define IC_TAR_ADDRESS                  UINT32_C(0x000003FF)

#define IC_ENABLE_STATUS_MASK           UINT32_C(0x00000001)
#define IC_ENABLE_STATUS_DISABLED       0x0
#define IC_ENABLE_STATUS_ENABLED        0x1

#define IC_STATUS_MST_ACTIVITY_MASK     UINT32_C(0x00000020)
#define IC_STATUS_TFNF_MASK             UINT32_C(0x00000002)

#define IC_DATA_CMD_CMD_MASK            UINT32_C(0x00000100)
#define IC_DATA_CMD_DATA_MASK           UINT32_C(0x000000FF)

/*
 * Command modes for IC_DATA_CMD
 * Note: Bit [8] will be cleared when a new byte is written into the IC_DATA_CMD
 *    register, setting write mode automatically.
 */
#define IC_DATA_CMD_READ                0x100

/* IRQ Masks */
#define IC_INTR_TX_ABRT_POS             6
#define IC_INTR_TX_ABRT_MASK            (UINT32_C(1) << IC_INTR_TX_ABRT_POS)

#define IC_INTR_STOP_DET_POS            9
#define IC_INTR_STOP_DET_MASK           (UINT32_C(1) << IC_INTR_STOP_DET_POS)

#endif /* DW_APB_I2C_H */
