/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPMM_H
#define MPMM_H

#include <fwk_macros.h>

#include <stdint.h>

/*! MPMM registers */
struct mpmm_reg {
    FWK_R uint32_t PPMCR;
    uint32_t RESERVED0[3];
    FWK_RW uint32_t MPMMCR;
};

/* PPMCR bits definitions */
#define MPMM_PPMCR_NUM_GEARS_MASK UINT32_C(0x00000007)
#define MPMM_PPMCR_NUM_GEARS_POS  8

/* MPMMCR bits definitions */
#define MPMM_MPMMCR_EN_MASK   UINT32_C(0x00000001)
#define MPMM_MPMMCR_GEAR_MASK UINT32_C(0x00000003)
#define MPMM_MPMMCR_EN_POS    0
#define MPMM_MPMMCR_GEAR_POS  1

#endif /* MPMM_H */
