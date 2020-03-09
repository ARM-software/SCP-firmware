/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "low_level_access.h"
#include "synquacer_mmap.h"

#include <stdint.h>

#define REG_ADDR_OFFSET_BOOT_REMAP UINT32_C(0x000)
#define REG_ADDR_OFFSET_BOOT_FE UINT32_C(0x004)
#define REG_ADDR_OFFSET_BOOT_HSSPI UINT32_C(0x008)
#define REG_ADDR_OFFSET_BOOT_EMMC UINT32_C(0x010)
#define REG_ADDR_OFFSET_BOOT_MODE UINT32_C(0x014)
#define REG_ADDR_OFFSET_DSW3_STATUS UINT32_C(0x018)

void set_memory_remap(uint32_t value)
{
    writel(
        CONFIG_SOC_REG_ADDR_BOOT_CTL_TOP + REG_ADDR_OFFSET_BOOT_REMAP, value);
}

uint8_t get_dsw3_status(uint8_t bit_mask)
{
    uint8_t value =
        readb(CONFIG_SOC_REG_ADDR_BOOT_CTL_TOP + REG_ADDR_OFFSET_DSW3_STATUS);

    return (value & bit_mask);
}
