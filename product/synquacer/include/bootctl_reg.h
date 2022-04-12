/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOOTCTL_REG_H
#define BOOTCTL_REG_H

#include <fwk_macros.h>

#include <stdint.h>

struct bootctl_reg {
    FWK_RW uint32_t BOOT_REMAP;
           uint32_t RESERVED1;
    FWK_RW uint32_t BOOT_HSSPI;
           uint32_t RESERVED2[2];
    FWK_R uint32_t BOOT_MODE;
};

/* BOOT_HSSPI register field */
#define ADRDEC_HSSPIx(val, bs) (((val)&0x1) << (bs))
#define ADRDEC_HSSPIx_MASK(bs) ADRDEC_HSSPIx(1, bs)
#define CMDSEL_HSSPIx(val, bs) (((val)&0x1) << (bs))
#define CMDSEL_HSSPIx_MASK(bs) CMDSEL_HSSPIx(1, bs)
#define ADRDEC_HSSPI1_BIT      0
#define CMDSEL_HSSPI1_BIT      1
#define ADRDEC_HSSPI2_BIT      4
#define CMDSEL_HSSPI2_BIT      5
#define HSEL_BMEM              0
#define HSEL_MEM               1
#define QUAD_READ_MODE         0
#define FAST_READ_MODE         1

#endif /* BOOTCTL_REG_H */
