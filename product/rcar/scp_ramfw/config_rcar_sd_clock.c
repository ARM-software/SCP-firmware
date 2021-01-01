/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clock_sd_devices.h>
#include <system_clock.h>

#include <mod_rcar_clock.h>
#include <mod_rcar_sd_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

/*
 * Rate lookup tables
 */
static const struct mod_rcar_sd_clock_rate rate_table_sys_ztr[] = {
    {
        /* 1/24 */
        .divider_num = 24,
        .divider = CPG_FRQCRB_ZTRFC_24,
        .divider_mask = CPG_FRQCRB_ZTRFC_MASK,
    },
    {
        /* 1/18 */
        .divider_num = 18,
        .divider = CPG_FRQCRB_ZTRFC_18,
        .divider_mask = CPG_FRQCRB_ZTRFC_MASK,
    },
    {
        /* 1/16 */
        .divider_num = 16,
        .divider = CPG_FRQCRB_ZTRFC_16,
        .divider_mask = CPG_FRQCRB_ZTRFC_MASK,
    },
    {
        /* 1/12 */
        .divider_num = 12,
        .divider = CPG_FRQCRB_ZTRFC_12,
        .divider_mask = CPG_FRQCRB_ZTRFC_MASK,
    },
    {
        /* 1/8 */
        .divider_num = 8,
        .divider = CPG_FRQCRB_ZTRFC_8,
        .divider_mask = CPG_FRQCRB_ZTRFC_MASK,
    },
    {
        /* 1/6 */
        .divider_num = 6,
        .divider = CPG_FRQCRB_ZTRFC_6,
        .divider_mask = CPG_FRQCRB_ZTRFC_MASK,
    },
};

static const struct mod_rcar_sd_clock_rate rate_table_sys_ztrd2[] = {
    {
        /* 1/24 */
        .divider_num = 24,
        .divider = CPG_FRQCRB_ZTRD2FC_24,
        .divider_mask = CPG_FRQCRB_ZTRD2FC_MASK,
    },
    {
        /* 1/18 */
        .divider_num = 18,
        .divider = CPG_FRQCRB_ZTRD2FC_18,
        .divider_mask = CPG_FRQCRB_ZTRD2FC_MASK,
    },
    {
        /* 1/16 */
        .divider_num = 16,
        .divider = CPG_FRQCRB_ZTRD2FC_16,
        .divider_mask = CPG_FRQCRB_ZTRD2FC_MASK,
    },
    {
        /* 1/12 */
        .divider_num = 12,
        .divider = CPG_FRQCRB_ZTRD2FC_12,
        .divider_mask = CPG_FRQCRB_ZTRD2FC_MASK,
    },
};

static const struct mod_rcar_sd_clock_rate rate_table_sys_zt[] = {
    {
        /* 1/24 */
        .divider_num = 24,
        .divider = CPG_FRQCRB_ZTFC_24,
        .divider_mask = CPG_FRQCRB_ZTFC_MASK,
    },
    {
        /* 1/18 */
        .divider_num = 18,
        .divider = CPG_FRQCRB_ZTFC_18,
        .divider_mask = CPG_FRQCRB_ZTFC_MASK,
    },
    {
        /* 1/16 */
        .divider_num = 16,
        .divider = CPG_FRQCRB_ZTFC_16,
        .divider_mask = CPG_FRQCRB_ZTFC_MASK,
    },
    {
        /* 1/12 */
        .divider_num = 12,
        .divider = CPG_FRQCRB_ZTFC_12,
        .divider_mask = CPG_FRQCRB_ZTFC_MASK,
    },
    {
        /* 1/8 */
        .divider_num = 8,
        .divider = CPG_FRQCRB_ZTFC_8,
        .divider_mask = CPG_FRQCRB_ZTFC_MASK,
    },
    {
        /* 1/6 */
        .divider_num = 6,
        .divider = CPG_FRQCRB_ZTFC_6,
        .divider_mask = CPG_FRQCRB_ZTFC_MASK,
    },
    {
        /* 1/4 */
        .divider_num = 4,
        .divider = CPG_FRQCRB_ZTFC_4,
        .divider_mask = CPG_FRQCRB_ZTFC_MASK,
    },
};

static const struct mod_rcar_sd_clock_rate rate_table_sys_sd[] = {
    {
        /* 1/64 */
        .divider_num = 64,
        .divider = CPG_SDNCKCR_SD_64,
        .divider_mask = CPG_SDNCKCR_MASK,
    },
    {
        /* 1/32 */
        .divider_num = 32,
        .divider = CPG_SDNCKCR_SD_32,
        .divider_mask = CPG_SDNCKCR_MASK,
    },
    {
        /* 1/16 */
        .divider_num = 16,
        .divider = CPG_SDNCKCR_SD_16,
        .divider_mask = CPG_SDNCKCR_MASK,
    },
    {
        /* 1/8 */
        .divider_num = 8,
        .divider = CPG_SDNCKCR_SD_8,
        .divider_mask = CPG_SDNCKCR_MASK,
    },
    {
        /* 1/4 */
        .divider_num = 4,
        .divider = CPG_SDNCKCR_SD_4,
        .divider_mask = CPG_SDNCKCR_MASK,
    },
    {
        /* 1/2 */
        .divider_num = 2,
        .divider = CPG_SDNCKCR_SD_2,
        .divider_mask = CPG_SDNCKCR_MASK,
    },
};

static const struct mod_rcar_sd_clock_rate rate_table_sys_ckcr[] = {
    /* min rate*/
    {
        .divider_num = 64,
    },
    /* max rate*/
    {
        .divider_num = 1,
    },
    /* step rate*/
    {
        .divider_num = 64,
    },
};

static const struct fwk_element pik_clock_element_table[] = {
    {
        .name = "ztr",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .control_reg = CPG_FRQCRB,
            .stop_clk = false,
            .parent = CLK_PLL1_DIV2,
            .div = 6,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
            .rate_table = rate_table_sys_ztr,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_ztr),
        }),
    },
    {
        .name = "ztrd2",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .control_reg = CPG_FRQCRB,
            .stop_clk = false,
            .parent = CLK_PLL1_DIV2,
            .div = 12,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
            .rate_table = rate_table_sys_ztrd2,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_ztrd2),
        }),
    },
    {
        .name = "zt",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .control_reg = CPG_FRQCRB,
            .stop_clk = false,
            .parent = CLK_PLL1_DIV2,
            .div = 4,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
            .rate_table = rate_table_sys_zt,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_zt),
        }),
    },
    {
        .name = "zx",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .control_reg = CPG_FRQCRB,
            .parent = CLK_PLL1_DIV2,
            .div = 2,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s0d1",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S0,
            .div = 1,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s0d2",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S0,
            .div = 2,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s0d3",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S0,
            .div = 3,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s0d4",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S0,
            .div = 4,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s0d6",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S0,
            .div = 6,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s0d8",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S0,
            .div = 8,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s0d12",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S0,
            .div = 12,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s1d1",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S1,
            .div = 1,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s1d2",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S1,
            .div = 2,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s1d4",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S1,
            .div = 4,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s2d1",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S2,
            .div = 1,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s2d2",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S2,
            .div = 2,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s2d4",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S2,
            .div = 4,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s3d1",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S3,
            .div = 1,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s3d2",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S3,
            .div = 2,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s3d4",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_S3,
            .div = 4,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "sd0",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .control_reg = CPG_SD0CKCR,
            .stop_clk = true,
            .stop_clk_bit = 8,
            .parent = CLK_SDSRC,
            .need_hardware_init = true,
            .div = 4,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
            .rate_table = rate_table_sys_sd,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_sd),
        }),
    },
    {
        .name = "sd1",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .control_reg = CPG_SD1CKCR,
            .stop_clk = true,
            .stop_clk_bit = 8,
            .parent = CLK_SDSRC,
            .need_hardware_init = true,
            .div = 4,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
            .rate_table = rate_table_sys_sd,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_sd),
        }),
    },
    {
        .name = "sd2",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .control_reg = CPG_SD2CKCR,
            .stop_clk = true,
            .stop_clk_bit = 8,
            .parent = CLK_SDSRC,
            .need_hardware_init = true,
            .div = 4,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
            .rate_table = rate_table_sys_sd,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_sd),
        }),
    },
    {
        .name = "sd3",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .control_reg = CPG_SD3CKCR,
            .stop_clk = true,
            .stop_clk_bit = 8,
            .parent = CLK_SDSRC,
            .need_hardware_init = true,
            .div = 4,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
            .rate_table = rate_table_sys_sd,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_sd),
        }),
    },
    {
        .name = "cl",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_PLL1_DIV2,
            .div = 48,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "cr",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_PLL1_DIV4,
            .div = 2,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "cp",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_EXTAL,
            .div = 2,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "cpex",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_EXTAL,
            .div = 2,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "canfd",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .control_reg = CPG_CANFDCKCR,
            .stop_clk = true,
            .stop_clk_bit = 8,
            .parent = CLK_PLL1_DIV4,
            .div = 64,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS,
            .rate_table = rate_table_sys_ckcr,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_ckcr),
        }),
    },
    {
        .name = "csi0",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .control_reg = CPG_CSI0CKCR,
            .stop_clk = true,
            .stop_clk_bit = 8,
            .parent = CLK_PLL1_DIV4,
            .div = 64,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS,
            .rate_table = rate_table_sys_ckcr,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_ckcr),
        }),
    },
    {
        .name = "mso",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .control_reg = CPG_MSOCKCR,
            .stop_clk = true,
            .stop_clk_bit = 8,
            .parent = CLK_PLL1_DIV4,
            .div = 63,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS,
            .rate_table = rate_table_sys_ckcr,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_ckcr),
        }),
    },
    {
        .name = "hdmi",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .control_reg = CPG_HDMICKCR,
            .stop_clk = true,
            .stop_clk_bit = 8,
            .parent = CLK_PLL1_DIV4,
            .div = 63,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS,
            .rate_table = rate_table_sys_ckcr,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_ckcr),
        }),
    },
    {
        .name = "osc",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_OSC_EXTAL,
            .div = 8,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "r",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_OSC_EXTAL,
            .div = 32,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s0",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_PLL1_DIV2,
            .div = 2,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s1",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_PLL1_DIV2,
            .div = 3,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s2",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_PLL1_DIV2,
            .div = 4,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "s3",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_PLL1_DIV2,
            .div = 6,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "sdsrc",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_PLL1_DIV2,
            .div = 2,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    {
        .name = "rint",
        .data = &((struct mod_rcar_sd_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
            .parent = CLK_OSC_EXTAL,
            .div = 32,
            .multi = 1,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
        }),
    },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *pik_clock_get_element_table(fwk_id_t module_id)
{
    return pik_clock_element_table;
}

struct fwk_module_config config_rcar_sd_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(pik_clock_get_element_table),
    .data = &((struct mod_ext_clock_rate){
        .ext_clk_rate = PLL_BASE_CLOCK,
    }),
};
