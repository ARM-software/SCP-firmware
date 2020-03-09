/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions for the Juno PVT Sensors
 */

#ifndef JUNO_PVT_H
#define JUNO_PVT_H

#include "juno_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

/*
 * PVT-Group register definitions.
 */
struct juno_pvt_reg {
    FWK_R   uint32_t    GROUP_INFO;
    FWK_RW  uint32_t    MEASUREMENT_ENABLE;
            uint8_t     RESERVED1[0x10 - 0x08];
    FWK_RW  uint32_t    SENSOR_ENABLE;
            uint8_t     RESERVED2[0x60 - 0x14];
    FWK_RW  uint32_t    SSI_RATE_DIV;
            uint8_t     RESERVED3[0x94 - 0x64];
    FWK_RW  uint32_t    SAMPLE_WINDOW;
            uint8_t     RESERVED4[0xA0 - 0x98];
    FWK_R   uint32_t    SENSOR_DATA_VALID;
            uint8_t     RESERVED5[0xC0 - 0xA4];
    FWK_RW  uint32_t    IRQ_CLEAR;
    FWK_RW  uint32_t    IRQ_MASK_SET;
    FWK_W   uint32_t    IRQ_MASK_CLEAR;
            uint8_t     RESERVED6[0x100 - 0xCC];
    FWK_R   uint32_t    SENSOR_DATA[32];
};

#define PVT_REG_STDCELL     ((struct juno_pvt_reg *) PVT_STDCELL_BASE)
#define PVT_REG_SOC         ((struct juno_pvt_reg *) PVT_SOC_BASE)
#define PVT_REG_GPU         ((struct juno_pvt_reg *) PVT_GPU_BASE)
#define PVT_REG_BIG         ((struct juno_pvt_reg *) PVT_BIG_BASE)
#define PVT_REG_LITTLE      ((struct juno_pvt_reg *) PVT_LITTLE_BASE)

#endif /* JUNO_PVT_H */
