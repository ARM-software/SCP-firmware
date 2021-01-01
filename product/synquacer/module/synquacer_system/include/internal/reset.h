/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_RESET_H
#define INTERNAL_RESET_H

typedef enum { RST_TYPE_ALL = 0, RST_TYPE_WO_BUS, RST_TYPE_BUS } RST_TYPE_t;

typedef enum {
    RST_PCIE_TOP = 0,
    RST_PCIE0,
    RST_PCIE1,
    RST_DMA,
    RST_DDR
} RST_BLOCK;

#define RST_ADDR_SYSOC_BUS 0x48300000
#define RST_ADDR_SYSOC_SCP 0x48300010
#define RST_ADDR_SYSOC_DMA 0x48300040
#define RST_ADDR_SYSOC_PCIE 0x48300050
#define RST_ADDR_SYSOC_DDR 0x48300060

#define RST_SYSOC_BUS_TOP 0x00000001
#define RST_SYSOC_BUS_PCIE0 0x00000004
#define RST_SYSOC_BUS_PCIE1 0x00000002
#define RST_SYSOC_BUS_PCIE_TOP 0x00000008

#define RST_SYSOC_SCP_SCB 0x00000001
#define RST_SYSOC_SCP_SCBM 0x00000002
#define RST_SYSOC_SCP_CSS_PORST 0x00000004
#define RST_SYSOC_SCP_CSS_SYSRST 0x00000008
#define RST_SYSOC_SCP_CSS_SRST 0x00000010

#define RST_SYSOC_DMA_DMA 0x00000001
#define RST_SYSOC_DMA_DMAB 0x00000002

#define RST_SYSOC_PCIE_PCIE0 0x00000002
#define RST_SYSOC_PCIE_PCIE1 0x00000001

#define RST_SYSOC_DDR 0x00000001

#define RST_ADDR_LPCM_SCB 0x48318400
#define RST_ADDR_LPCM_PCIE 0x48319400
#define RST_ADDR_LPCM_DMA 0x4831A400

#define RST_LPCM_SCB_BUS 0x00000001
#define RST_LPCM_PCIE0 0x00000002
#define RST_LPCM_PCIE1 0x00000001
#define RST_LPCM_PCIE0_BUS 0x00000020
#define RST_LPCM_PCIE1_BUS 0x00000010
#define RST_LPCM_PCIE_TOP 0x00000040
#define RST_LPCM_DMA_BUS 0x00000001
#define RST_LPCM_DMAB 0x00000002

#define RST_NO_USE 0

#define RESET_INFO                                                          \
    {                                                                       \
        {                                                                   \
            /* PCIe TOP */ /* block sosoc    */ RST_NO_USE,                 \
                           /* block lpcm     */ RST_NO_USE,                 \
                           /* bus   sosoc    */ RST_SYSOC_BUS_PCIE_TOP,     \
                           /* bus   lpcm     */ RST_LPCM_PCIE_TOP,          \
                           /* addr sysoc blk */ RST_NO_USE,                 \
                           /* addr sysoc bus */ RST_ADDR_SYSOC_BUS,         \
                           /* addr lpcm      */ RST_ADDR_LPCM_PCIE,         \
        },                                                                  \
            {                                                               \
                /* PCIe PCIe0 */ /* block sosoc    */ RST_SYSOC_PCIE_PCIE0, \
                                 /* block lpcm     */ RST_LPCM_PCIE0,       \
                                 /* bus   sosoc    */ RST_SYSOC_BUS_PCIE0,  \
                                 /* bus   lpcm     */ RST_LPCM_PCIE0_BUS,   \
                                 /* addr sysoc blk */ RST_ADDR_SYSOC_PCIE,  \
                                 /* addr sysoc bus */ RST_ADDR_SYSOC_BUS,   \
                                 /* addr lpcm      */ RST_ADDR_LPCM_PCIE,   \
            },                                                              \
            {                                                               \
                /* PCIe PCIe1 */ /* block sosoc    */ RST_SYSOC_PCIE_PCIE1, \
                                 /* block lpcm     */ RST_LPCM_PCIE1,       \
                                 /* bus   sosoc    */ RST_SYSOC_BUS_PCIE1,  \
                                 /* bus   lpcm     */ RST_LPCM_PCIE1_BUS,   \
                                 /* addr sysoc blk */ RST_ADDR_SYSOC_PCIE,  \
                                 /* addr sysoc bus */ RST_ADDR_SYSOC_BUS,   \
                                 /* addr lpcm      */ RST_ADDR_LPCM_PCIE,   \
            },                                                              \
            {                                                               \
                /* DMA */ /* block sosoc    */ RST_SYSOC_DMA_DMAB,          \
                          /* block lpcm     */ RST_LPCM_DMAB,               \
                          /* bus   sosoc    */ RST_SYSOC_DMA_DMA,           \
                          /* bus   lpcm     */ RST_LPCM_DMA_BUS,            \
                          /* addr sysoc blk */ RST_ADDR_SYSOC_DMA,          \
                          /* addr sysoc bus */ RST_ADDR_SYSOC_DMA,          \
                          /* addr lpcm      */ RST_ADDR_LPCM_DMA,           \
            },                                                              \
        { /* DDR */                                                         \
            /* block sosoc    */ RST_SYSOC_DDR,                             \
                /* block lpcm     */ RST_NO_USE,                            \
                /* bus   sosoc    */ RST_NO_USE,                            \
                /* bus   lpcm     */ RST_NO_USE,                            \
                /* addr sysoc blk */ RST_ADDR_SYSOC_DDR,                    \
                /* addr sysoc bus */ RST_NO_USE,                            \
                /* addr lpcm      */ RST_NO_USE,                            \
        }                                                                   \
    }

void lpcm_sysoc_reset(RST_TYPE_t type, RST_BLOCK block);
void lpcm_sysoc_reset_clear(RST_TYPE_t type, RST_BLOCK block);

#endif /* INTERNAL_RESET_H */
