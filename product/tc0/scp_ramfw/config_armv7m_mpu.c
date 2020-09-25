/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_mmap.h"
#include "scp_software_mmap.h"

#include <mod_armv7m_mpu.h>

#include <fwk_macros.h>
#include <fwk_module.h>

#include <fmw_cmsis.h>

static const ARM_MPU_Region_t regions[] = {
    {
        /* 0x0000_0000 - 0xFFFF_FFFF */
        .RBAR = ARM_MPU_RBAR(0, 0x00000000),
        .RASR = ARM_MPU_RASR(
            1,
            ARM_MPU_AP_PRIV,
            0,
            1,
            0,
            1,
            0,
            ARM_MPU_REGION_SIZE_4GB),
    },
    {
        /* 0x1000_0000 - 0x1007_FFFF */
        .RBAR = ARM_MPU_RBAR(1, SCP_RAM_BASE),
        .RASR = ARM_MPU_RASR(
            0,
            ARM_MPU_AP_PRIV,
            0,
            0,
            1,
            1,
            0,
            ARM_MPU_REGION_SIZE_512KB),
    },
    {
        /* 0xA400_0000 - 0xA400_7FFF*/
        .RBAR = ARM_MPU_RBAR(2, SCP_TRUSTED_RAM_BASE),
        .RASR = ARM_MPU_RASR(
            1,
            ARM_MPU_AP_PRIV,
            0,
            1,
            1,
            1,
            0,
            ARM_MPU_REGION_SIZE_4KB),
    },
    {
        /* 0xA600_0000 - 0xA600_7FFF */
        .RBAR = ARM_MPU_RBAR(3, SCP_NONTRUSTED_RAM_BASE),
        .RASR = ARM_MPU_RASR(
            1,
            ARM_MPU_AP_PRIV,
            0,
            1,
            1,
            1,
            0,
            ARM_MPU_REGION_SIZE_256B),
    },
};

const struct fwk_module_config config_armv7m_mpu = {
    .data = &((struct mod_armv7m_mpu_config){
        .region_count = FWK_ARRAY_SIZE(regions),
        .regions = regions,
    }),
};
