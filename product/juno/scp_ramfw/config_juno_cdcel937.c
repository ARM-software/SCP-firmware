/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_clock.h"
#include "juno_id.h"
#include "juno_mmap.h"
#include "juno_scc.h"

#include <mod_clock.h>
#include <mod_juno_cdcel937.h>
#include <mod_juno_hdlcd.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

static struct juno_clock_lookup i2s_lookup_table[] = {
    {
        .rate_hz = 1536000,
        .preset = {.N = 384, .M = 48, .PDIV = 125},
    },
    {
        .rate_hz = 2116800,
        .preset = {.N = 441, .M = 50, .PDIV = 100},
    },
    {
        .rate_hz = 2304000,
        .preset = {.N = 228, .M = 25, .PDIV = 95},
    },
    {
        .rate_hz = 4233600,
        .preset = {.N = 441, .M = 50, .PDIV = 50},
    },
    {
        .rate_hz = 4608000,
        .preset = {.N = 864, .M = 100, .PDIV = 45},
    },
    {
        .rate_hz = 8467200,
        .preset = {.N = 3528, .M = 400, .PDIV = 25},
    },
    {
        .rate_hz = 9216000,
        .preset = {.N = 3312, .M = 375, .PDIV = 23},
    },
};

static const struct fwk_element juno_cdcel937_element_table[] = {
    [JUNO_CLOCK_CDCEL937_IDX_I2SCLK] = {
        .name = "",
        .data = &(struct mod_juno_cdcel937_dev_config) {
            .slave_address = 0x6D,
            .xin_mhz = 24,
            .clock_hal_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                JUNO_CLOCK_IDX_I2SCLK),
            .clock_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CLOCK,
                MOD_CLOCK_API_TYPE_DRIVER_RESPONSE),
            .min_rate = 0,
            .max_rate = 0,
            .min_step = 0,
            .rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE,
            .output_id = MOD_JUNO_CDCEL937_OUTPUT_ID_Y4,
            .lookup_table = i2s_lookup_table,
            .lookup_table_count = FWK_ARRAY_SIZE(i2s_lookup_table),
        }
    },
    [JUNO_CLOCK_CDCEL937_IDX_HDLCDREFCLK] = {
        .name = "",
        .data = &(struct mod_juno_cdcel937_dev_config) {
            .slave_address = 0x6C,
            .xin_mhz = 24,
            .clock_hal_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                JUNO_CLOCK_IDX_HDLCDREFCLK),
            .clock_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CLOCK,
                MOD_CLOCK_API_TYPE_DRIVER_RESPONSE),
            .min_rate = 25 * FWK_MHZ,
            .max_rate = 120 * FWK_MHZ,
            .min_step = 250 * FWK_KHZ,
            .rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS,
            .output_id = MOD_JUNO_CDCEL937_OUTPUT_ID_Y6,
        }
    },
    [JUNO_CLOCK_CDCEL937_IDX_HDLCDPXL] = {
        .name = "",
        .data = &(struct mod_juno_cdcel937_dev_config) {
            .slave_address = 0x6D,
            .xin_mhz = 24,
            .clock_hal_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                JUNO_CLOCK_IDX_HDLCDPXL),
            .clock_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CLOCK,
                MOD_CLOCK_API_TYPE_DRIVER_RESPONSE),
            .min_rate = 47500 * FWK_KHZ,
            .max_rate = 100 * FWK_MHZ,
            .min_step = 250 * FWK_KHZ,
            .rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS,
            .output_id = MOD_JUNO_CDCEL937_OUTPUT_ID_Y2,
        }
    },
    [JUNO_CLOCK_CDCEL937_IDX_HDLCD0] = {
        .name = "",
        .data = &(struct mod_juno_cdcel937_dev_config) {
            .slave_address = 0x6C,
            .xin_mhz = 24,
            .clock_hal_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_HDLCD,
                JUNO_CLOCK_HDLCD_IDX_HDLCD0),
            .clock_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_HDLCD,
                MOD_JUNO_HDLCD_API_IDX_HDLCD_DRIVER_RESPONSE),
            .min_rate = 25 * FWK_MHZ,
            .max_rate = 120 * FWK_MHZ,
            .min_step = 250 * FWK_KHZ,
            .rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS,
            .output_id = MOD_JUNO_CDCEL937_OUTPUT_ID_Y6,
        }
    },
    [JUNO_CLOCK_CDCEL937_IDX_HDLCD1] = {
        .name = "",
        .data = &(struct mod_juno_cdcel937_dev_config) {
            .slave_address = 0x6C,
            .xin_mhz = 24,
            .clock_hal_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_HDLCD,
                JUNO_CLOCK_HDLCD_IDX_HDLCD1),
            .clock_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_HDLCD,
                MOD_JUNO_HDLCD_API_IDX_HDLCD_DRIVER_RESPONSE),
            .min_rate = 25 * FWK_MHZ,
            .max_rate = 120 * FWK_MHZ,
            .min_step = 250 * FWK_KHZ,
            .rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS,
            .output_id = MOD_JUNO_CDCEL937_OUTPUT_ID_Y6,
        }
    },
    [JUNO_CLOCK_CDCEL937_IDX_COUNT] = { 0 },
};

static void hdlcd_setup_lookup_table(
    struct mod_juno_cdcel937_dev_config *config,
    enum juno_idx_platform platform_id)
{
    uintptr_t hdlcd_lookup = HDLCD_PRESET_TABLE_BASE;

    if (SCC->GPR0 & SCC_GPR0_HIGH_PXLCLK_ENABLE) {
        config->lookup_table_count =
            JUNO_CLOCK_HDLCD_LOOKUP_HIGH_PXCLK_ENABLE_COUNT;
        if (platform_id == JUNO_IDX_PLATFORM_RTL) {
            /* Check that the signatures are present */
            fwk_assert(((struct juno_clock_hdlcd_lookup_high_pxlclk_enable *)
                hdlcd_lookup)->SIGNATURE_A == 0x9DCA7B7A);
            fwk_assert(((struct juno_clock_hdlcd_lookup_high_pxlclk_enable *)
                hdlcd_lookup)->SIGNATURE_B == 0x5C7852A9);
            }
        config->lookup_table =
                         ((struct juno_clock_hdlcd_lookup_high_pxlclk_enable *)
                         HDLCD_PRESET_TABLE_BASE)->CLK;
    } else {
        config->lookup_table_count = JUNO_CLOCK_HDLCD_LOOKUP_COUNT;
        if (platform_id == JUNO_IDX_PLATFORM_RTL) {
            /* Check that the signatures are present */
            fwk_assert(((struct juno_clock_hdlcd_lookup *)
                hdlcd_lookup)->SIGNATURE_A == 0x9DCA7B7A);
            fwk_assert(((struct juno_clock_hdlcd_lookup *)
                hdlcd_lookup)->SIGNATURE_B == 0x5C7852A9);
        }
        config->lookup_table = ((struct juno_clock_hdlcd_lookup *)
            HDLCD_PRESET_TABLE_BASE)->CLK;
    }
}

static const struct fwk_element *juno_cdcel937_get_element_table(
        fwk_id_t module_id)
{
    int status;
    enum juno_idx_platform platform_id;
    struct mod_juno_cdcel937_dev_config *config;

    status = juno_id_get_platform(&platform_id);
    if (status != FWK_SUCCESS)
        return NULL;

    /* Add the lookup table to the HDLCD elements */
    for (int i = 0; i < JUNO_CLOCK_CDCEL937_IDX_COUNT; i++) {

        if (i == JUNO_CLOCK_CDCEL937_IDX_I2SCLK ||
            i == JUNO_CLOCK_CDCEL937_IDX_HDLCDPXL)
            continue;

        config = (struct mod_juno_cdcel937_dev_config *)
                    (juno_cdcel937_element_table[i].data);

        hdlcd_setup_lookup_table(config, platform_id);
    }

    return juno_cdcel937_element_table;
}

struct fwk_module_config config_juno_cdcel937 = {
    .get_element_table = juno_cdcel937_get_element_table,
    .data = &(struct mod_juno_cdcel937_config) {
        .i2c_hal_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_I2C, 0),
    },
};
