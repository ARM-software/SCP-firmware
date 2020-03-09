/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_TIMER_SYNC_H
#define INTERNAL_TIMER_SYNC_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief Timer Synchronization Register Definitions
 */
struct timer_sync_reg {
    /*! Master Generic Counter Synchronization Control Register */
    FWK_RW uint32_t MST_GCNT_SYNC_CTRL;

    /*! Slave Generic Counter Synchronization Control Register */
    FWK_RW uint32_t SLVCHIP_GCNT_SYNC_CTRL;

    /*! Slave Offset Threshold Register */
    FWK_RW uint32_t SLVCHIP_GCNT_OFF_THRESHOLD;

    /*! Slave Interrupt Status Register */
    FWK_R  uint32_t SLVCHIP_GCNT_INT_STATUS;

    /*! Generic Counter Timeout Register */
    FWK_RW uint32_t GCNT_TIMEOUT;

    /*! Slave Synchronization Interval Register */
    FWK_RW uint32_t SLVCHIP_GCNT_SYNC_INTERVAL;

    /*! Slave Timer Value Register (Low) */
    FWK_R  uint32_t SLVCHIP_GCNT_UPD_TVALUE_L;

    /*! Slave Timer Value Register (High) */
    FWK_R  uint32_t SLVCHIP_GCNT_UPD_TVALUE_H;

    /*! Generic Counter Synchronization Status Register */
    FWK_RW uint32_t GCNT_SYNC_STATUS;

    /*! Slave Network Delay Register (CCIX Delay) */
    FWK_RW uint32_t SLVCHIP_GCNT_NW_DELAY;

    /*! Slave Interrupt Clear Register */
    FWK_W  uint32_t SLVCHIP_GCNT_INT_CLR;
};

#define MST_GCNT_SYNC_CTRL_EN_MASK        UINT32_C(0x1)
#define MST_GCNT_SYNC_CTRL_EN_IMM_MASK    UINT32_C(0x2)

#define SLV_GCNT_SYNC_CTRL_EN_MASK        UINT32_C(0x1)
#define SLV_GCNT_SYNC_CTRL_EN_IMM_MASK    UINT32_C(0x2)

#define SLVCHIP_GCNT_INT_STATUS_INT_MASK  UINT32_C(0x1)
#define SLVCHIP_GCNT_INT_CLR_MASK         UINT32_C(0x1)

#endif /* INTERNAL_TIMER_SYNC_H */
