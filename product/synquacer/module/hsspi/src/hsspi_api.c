/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_mmap.h"

#include <internal/hsspi_api.h>
#include <internal/hsspi_driver.h>
#include <internal/reg_HSSPI.h>

#include <mod_synquacer_system.h>

#include <fwk_log.h>

#include <stdbool.h>

#define FILE_GRP_ID DBG_DRV_HSSPI
#define xcpb_bridge_mode_set(enable)

static const struct HSSPI_clk_config clk_config = CONFIG_SOC_HSSPI_CLK_CONFIG;

#define HSSPI_WINDOW_SIZE_INDEX_256MB (0xF)
#define HSSPI_WINDOW_SIZE_INDEX_128MB (0xE)
#define HSSPI_WINDOW_SIZE_INDEX_64MB (0xD)
#define HSSPI_WINDOW_SIZE_INDEX_32MB (0xC)
#define HSSPI_WINDOW_SIZE_INDEX_16MB (0xB)
#define HSSPI_WINDOW_SIZE_INDEX_8MB (0xA)
#define HSSPI_WINDOW_SIZE_INDEX_4MB (0x9)
#define HSSPI_WINDOW_SIZE_INDEX_2MB (0x8)

#define CONFIG_SCB_FORCE_HSSPI_RESOURCE_ALLOCATION_MODEL \
    HSSPI_WINDOW_SIZE_INDEX_256MB

void HSSPI_init(void)
{
    int model_index;

    xcpb_bridge_mode_set(true);

    model_index = CONFIG_SCB_FORCE_HSSPI_RESOURCE_ALLOCATION_MODEL;

    FWK_LOG_INFO(
        "[HS-SPI] Configuring HS-SPI controller with "
        "clk_sel=%d clk_div=%d syncon=%d use_hsspi_cs1_flag=%d msel=%d",
        clk_config.clk_sel,
        clk_config.clk_div,
        clk_config.syncon,
        false,
        model_index);

    /* Initialize HS-SPI controller and external serial flash memory */
    hsspi_command_switch(
        (volatile REG_ST_HSSPI_t *)HSSPI_REG_BASE,
        (volatile void *)HSSPI_MEM_BASE,
        (volatile void *)CONFIG_SOC_REG_ADDR_BOOT_CTL_TOP,
        clk_config.clk_sel,
        clk_config.clk_div,
        clk_config.syncon,
        0, /* use_hsspi_cs1_flag disable*/
        (HSSPI_EN_CSCFG_MSEL_t)model_index);

    xcpb_bridge_mode_set(false);
}

void HSSPI_exit(void)
{
    int model_index;

    xcpb_bridge_mode_set(true);

    model_index = CONFIG_SCB_FORCE_HSSPI_RESOURCE_ALLOCATION_MODEL;

    /* Initialize HS-SPI controller and external serial flash memory */
    hsspi_exit(
        (volatile REG_ST_HSSPI_t *)HSSPI_REG_BASE,
        (volatile void *)HSSPI_MEM_BASE,
        (volatile void *)CONFIG_SOC_REG_ADDR_BOOT_CTL_TOP,
        clk_config.clk_sel,
        clk_config.clk_div,
        clk_config.syncon,
        0, /* use_hsspi_cs1_flag disable*/
        (HSSPI_EN_CSCFG_MSEL_t)model_index);
}
