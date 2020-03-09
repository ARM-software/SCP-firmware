/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     PL35x NAND Flash Memory Controller register definitions
 */

#ifndef PL35X_H
#define PL35X_H

#include "juno_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

/*
 * PL35X register definitions
 */
struct pl35x_reg {
    FWK_R   uint32_t      MEMC_STATUS;
    FWK_R   uint32_t      MEMIF_CONFIG;
    FWK_W   uint32_t      MEM_CFG_SET;
    FWK_W   uint32_t      MEM_CFG_CLR;
    FWK_W   uint32_t      DIRECT_CMD;
    FWK_W   uint32_t      SET_CYCLES;
    FWK_W   uint32_t      SET_OPMODE;
            uint8_t       RESERVED1[0x20 - 0x1C];
    FWK_RW  uint32_t      REFRESH0;
    FWK_RW  uint32_t      REFRESH1;
            uint8_t       RESERVED2[0x100 - 0x28];

    struct { /* CS registers */
        FWK_R  uint32_t  CYCLES;
        FWK_R  uint32_t  OPMODE;
               uint32_t  RESERVED[6];
    } CS[8];

    FWK_R   uint32_t      USER_STATUS;
    FWK_W   uint32_t      USER_CONFIG;
            uint8_t       RESERVED3[0x300 - 0x208];

    struct pl35x_ecc_reg { /* ECC registers */
        FWK_RW  uint32_t  STATUS;
        FWK_RW  uint32_t  CONFIG;
        FWK_RW  uint32_t  MEMCMD0;
        FWK_RW  uint32_t  MEMCMD1;
        FWK_W   uint32_t  ADDR0;
        FWK_W   uint32_t  ADDR1;
        FWK_RW  uint32_t  BLOCK0;
        FWK_RW  uint32_t  BLOCK1;
        FWK_RW  uint32_t  BLOCK2;
        FWK_RW  uint32_t  BLOCK3;
        FWK_RW  uint32_t  EXTRA_BLOCK;
                uint32_t  RESERVED[53];
    } ECC[2];

            uint8_t       RESERVED4[0xE00 - 0x500];
    FWK_RW  uint32_t      INT_CFG;
    FWK_RW  uint32_t      INT_INPUTS;
    FWK_RW  uint32_t      INT_OUTPUTS;
            uint8_t       RESERVED5[0xFE0 - 0xE0C];
    FWK_R   uint32_t      PERIPH_ID_0;
    FWK_R   uint32_t      PERIPH_ID_1;
    FWK_R   uint32_t      PERIPH_ID_2;
    FWK_R   uint32_t      PERIPH_ID_3;
    FWK_R   uint32_t      PCELL_ID_0;
    FWK_R   uint32_t      PCELL_ID_1;
    FWK_R   uint32_t      PCELL_ID_2;
    FWK_R   uint32_t      PCELL_ID_3;
};

#define SMC ((struct pl35x_reg *) SMC_BASE)

#endif /* PL35X_H */
