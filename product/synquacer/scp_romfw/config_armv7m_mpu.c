/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_armv7m_mpu.h>

#include <fwk_macros.h>
#include <fwk_module.h>

#include <fmw_cmsis.h>

#include <stdint.h>

#define ROM_BASE 0x00000000UL
#define AP_RAM_BASE UINT32_C(0xA4000000)

static const ARM_MPU_Region_t regions[] = {
    {
        /* 0x0000_0000 - 0xFFFF_FFFF */
        .RBAR = ARM_MPU_RBAR(0, ROM_BASE),
        .RASR = ARM_MPU_RASR(
            1,
            ARM_MPU_AP_FULL,
            0,
            1,
            0,
            0,
            0,
            ARM_MPU_REGION_SIZE_4GB),
    },
    {
        /* 0x0000_0000 - 0x1FFF_FFFF */
        .RBAR = ARM_MPU_RBAR(1, ROM_BASE),
        .RASR = ARM_MPU_RASR(
            0,
            ARM_MPU_AP_FULL,
            0,
            0,
            1,
            1,
            0,
            ARM_MPU_REGION_SIZE_512MB),
    },
    {
        /* 0xA400_0000 - 0xA407_FFFF */
        .RBAR = ARM_MPU_RBAR(2, AP_RAM_BASE),
        .RASR = ARM_MPU_RASR(
            0,
            ARM_MPU_AP_FULL,
            0,
            0,
            1,
            1,
            0,
            ARM_MPU_REGION_SIZE_512KB),
    },
};

const struct fwk_module_config config_armv7m_mpu = {
    .data = &((struct mod_armv7m_mpu_config){
        .region_count = FWK_ARRAY_SIZE(regions),
        .regions = regions,
    }),
};
