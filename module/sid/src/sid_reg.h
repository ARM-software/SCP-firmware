/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SID_REG_H
#define SID_REG_H

#include <mod_pcid.h>

#include <fwk_macros.h>

#include <stdint.h>

struct sid_reg {
            uint8_t   RESERVED0[0x40 - 0];
    FWK_R   uint32_t  SYSTEM_ID;
            uint8_t   RESERVED1[0x50 - 0x44];
    FWK_R   uint32_t  SOC_ID;
            uint8_t   RESERVED2[0x60 - 0x54];
    FWK_R   uint32_t  NODE_ID;
            uint8_t   RESERVED3[0x70 - 0x64];
    FWK_R   uint32_t  SYSTEM_CFG;
            uint8_t   RESERVED4[0xFD0 - 0x74];
    const struct mod_pcid_registers pcid;
};

#define SID_SYS_SOC_ID_PART_NUMBER_MASK UINT32_C(0xFFF)

#define SID_SYS_SOC_ID_DESIGNER_ID_MASK UINT32_C(0xFF000)
#define SID_SYS_SOC_ID_DESIGNER_ID_POS  UINT32_C(12)

#define SID_SYS_SOC_ID_MINOR_REVISION_MASK UINT32_C(0xF00000)
#define SID_SYS_SOC_ID_MINOR_REVISION_POS  UINT32_C(20)

#define SID_SYS_SOC_ID_MAJOR_REVISION_MASK UINT32_C(0xF000000)
#define SID_SYS_SOC_ID_MAJOR_REVISION_POS  UINT32_C(24)

#define SID_SYS_NODE_NUMBER_MASK        UINT32_C(0xFF)
#define SID_SYS_MULTI_CHIP_MODE_MASK    UINT32_C(0x100)
#define SID_SYS_MULTI_CHIP_MODE_POS     UINT32_C(8)

#endif /* SID_REG_H */
