/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_mmap.h"
#include "scp_software_mmap.h"

#include <mod_armv8m_mpu.h>

#include <fwk_macros.h>
#include <fwk_module.h>

#include <fmw_cmsis.h>

static const uint8_t attributes[3] = {
    /* Device memory, non Gathering, non Re-ordering, non Early Write
       Acknowledgement */
    [MPU_ATTR_0] =
        ARM_MPU_ATTR(ARM_MPU_ATTR_DEVICE, ARM_MPU_ATTR_DEVICE_nGnRnE),

    /* Normal memory, non Cacheable */
    [MPU_ATTR_1] =
        ARM_MPU_ATTR(ARM_MPU_ATTR_NON_CACHEABLE, ARM_MPU_ATTR_NON_CACHEABLE),

    /* Normal memory, Inner Cacheable */
    [MPU_ATTR_2] = ARM_MPU_ATTR(
        ARM_MPU_ATTR_MEMORY_(0, 1, 1, 1),
        ARM_MPU_ATTR_NON_CACHEABLE),
};

static const ARM_MPU_Region_t regions[] = {
    {
        /* 0x0000_0000 - 0x007F_FFFF */
        .RBAR = ARM_MPU_RBAR(
            0x00000000, /* BASE */
            ARM_MPU_SH_INNER, /* SH */
            0, /* RO */
            0, /* NP */
            1 /* XN */),
        .RLAR = ARM_MPU_RLAR(0x7fffff, MPU_ATTR_1),
    },
    {
        /* 0x0080_0000 - 0x00FF_FFFF*/
        .RBAR = ARM_MPU_RBAR(
            SCP_ITC_RAM_BASE, /* BASE */
            ARM_MPU_SH_NON, /* SH */
            1, /* RO */
            0, /* NP */
            0 /* XN */),
        .RLAR = ARM_MPU_RLAR(0xffffff, MPU_ATTR_2),
    },
    {
        /* 0x2000_0000 - 0x20FF_FFFF */
        .RBAR = ARM_MPU_RBAR(
            SCP_DTC_RAM_BASE, /* BASE */
            ARM_MPU_SH_NON, /* SH */
            0, /* RO */
            0, /* NP */
            1 /* XN */),
        .RLAR = ARM_MPU_RLAR(0xffffff, MPU_ATTR_2),
    },
    {
        /* 0xA400_0000 - 0xA400_7FFF*/
        .RBAR = ARM_MPU_RBAR(
            SCP_TRUSTED_RAM_BASE, /* BASE */
            ARM_MPU_SH_INNER, /* SH */
            0, /* RO */
            0, /* NP */
            1 /* XN */),
        .RLAR = ARM_MPU_RLAR(0xA4007FFF, MPU_ATTR_2),
    },
    {
        /* 0xA600_0000 - 0xA600_7FFF*/
        .RBAR = ARM_MPU_RBAR(
            SCP_NONTRUSTED_RAM_BASE, /* BASE */
            ARM_MPU_SH_INNER, /* SH */
            0, /* RO */
            0, /* NP */
            1 /* XN */),
        .RLAR = ARM_MPU_RLAR(0xA6007FFF, MPU_ATTR_2),
    },
};

const struct fwk_module_config config_armv8m_mpu = {
    .data = &((struct mod_armv8m_mpu_config){
        .region_count = FWK_ARRAY_SIZE(regions),
        .regions = regions,
        .attributes_count = FWK_ARRAY_SIZE(attributes),
        .attributes = attributes,
    }),
};