/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_mcp_mmap.h"
#include "n1sdp_mcp_system_mmap.h"

#include <mod_armv7m_mpu.h>

#include <fwk_macros.h>
#include <fwk_module.h>

#include <fmw_cmsis.h>

static const ARM_MPU_Region_t regions[4] = {
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
        /* 0x0080_0000 - 0x0087_FFFF */
        .RBAR = ARM_MPU_RBAR(1, MCP_RAM0_BASE),
        .RASR = ARM_MPU_RASR(
            0,
            ARM_MPU_AP_PRO,
            0,
            0,
            1,
            0,
            0,
            ARM_MPU_REGION_SIZE_512KB),
    },
    {
        /* 0x2000_0000 - 0x2003_FFFF */
        .RBAR = ARM_MPU_RBAR(2, MCP_RAM1_BASE),
        .RASR = ARM_MPU_RASR(
            1,
            ARM_MPU_AP_PRIV,
            0,
            0,
            1,
            1,
            0,
            ARM_MPU_REGION_SIZE_256KB),
    },
    {
        /* 0xA600_0000 - 0xA600_7FFF */
        .RBAR = ARM_MPU_RBAR(3, MCP_NONTRUSTED_RAM_BASE),
        .RASR = ARM_MPU_RASR(
            1,
            ARM_MPU_AP_PRIV,
            0,
            /* Shared memory, Strongly Ordered */
            1,
            0,
            0,
            0,
            ARM_MPU_REGION_SIZE_32KB),
    },
};

const struct fwk_module_config config_armv7m_mpu = {
    .data = &((struct mod_armv7m_mpu_config){
        .region_count = FWK_ARRAY_SIZE(regions),
        .regions = regions,
    }),
};
