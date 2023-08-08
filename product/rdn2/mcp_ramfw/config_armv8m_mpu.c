/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mcp_mmap.h"
#include "mod_armv8m_mpu.h"

#include <fwk_macros.h>
#include <fwk_module.h>

#include <fmw_cmsis.h>

static const uint8_t attributes[2] = {
    /* Device memory, non Gathering, non Re-ordering, non Early Write
       Acknowledgement */
    /*
    [MPU_ATTR_0] =
        ARM_MPU_ATTR(ARM_MPU_ATTR_DEVICE, ARM_MPU_ATTR_DEVICE_nGnRnE),
    */
    /* Normal memory, non Cacheable */
    /*
    [MPU_ATTR_1] =
        ARM_MPU_ATTR(ARM_MPU_ATTR_NON_CACHEABLE, ARM_MPU_ATTR_NON_CACHEABLE),
    */
};

static const ARM_MPU_Region_t regions[] = {
    {
        /* 0x0000_0000 - 0xFFFF_FFFF */
        /*
        .RBAR = ...
        .RLAR = ...
        */
    },
    {
        /* 0x0080_0000 - 0x00A0_0000 */
        /*
        .RBAR = ...
        .RLAR = ...
        */
    },
    {
        /* 0x2000_0000 - 2020_0000 */
        /*
        .RBAR = ...
        .RLAR = ...
        */
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
