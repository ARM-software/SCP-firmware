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

enum juno_hdlcd_element_idx {
    JUNO_HDLCD_ELEMENT_IDX_HDLCD0,
    JUNO_HDLCD_ELEMENT_IDX_HDLCD1,
    JUNO_HDLCD_ELEMENT_COUNT,
};

static const struct fwk_element juno_hdlcd_element_table[] = {
    [JUNO_CLOCK_HDLCD_IDX_HDLCD0] = {
        .name = "",
        .data = &(struct mod_juno_hdlcd_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
                JUNO_CLOCK_CDCEL937_IDX_HDLCD0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
                MOD_JUNO_CDCEL937_API_IDX_HDLCD_DRIVER),
            .clock_hal_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                JUNO_CLOCK_IDX_HDLCD0),
            .clock_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CLOCK,
                MOD_CLOCK_API_TYPE_DRIVER_RESPONSE),
            .scc_control = &SCC->HDLCD0_CONTROL,
            .min_rate = 23750 * FWK_KHZ,
            .min_step = 250 * FWK_KHZ,
            .rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS,
        }
    },
    [JUNO_CLOCK_HDLCD_IDX_HDLCD1] = {
        .name = "",
        .data = &(struct mod_juno_hdlcd_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
                JUNO_CLOCK_CDCEL937_IDX_HDLCD1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
                MOD_JUNO_CDCEL937_API_IDX_HDLCD_DRIVER),
            .clock_hal_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                JUNO_CLOCK_IDX_HDLCD1),
            .clock_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CLOCK,
                MOD_CLOCK_API_TYPE_DRIVER_RESPONSE),
            .scc_control = &SCC->HDLCD1_CONTROL,
            .min_rate = 23750 * FWK_KHZ,
            .min_step = 250 * FWK_KHZ,
            .rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS,
        }
    },
    [JUNO_CLOCK_HDLCD_IDX_COUNT] = { 0 },
};

static const struct fwk_element *juno_hdlcd_get_element_table(
        fwk_id_t module_id)
{
    int status;
    struct mod_juno_hdlcd_dev_config *config;
    unsigned int i;
    enum juno_idx_platform platform_id;
    uintptr_t hdlcd_lookup = HDLCD_PRESET_TABLE_BASE;

    status = juno_id_get_platform(&platform_id);
    if (status != FWK_SUCCESS)
        return NULL;

    for (i = 0; i < JUNO_HDLCD_ELEMENT_COUNT; i++) {
        config = (struct mod_juno_hdlcd_dev_config *)
            (juno_hdlcd_element_table[i].data);
        if (SCC->GPR0 & SCC_GPR0_HIGH_PXLCLK_ENABLE) {
            config->lookup_table_count =
                JUNO_CLOCK_HDLCD_LOOKUP_HIGH_PXCLK_ENABLE_COUNT;
            config->max_rate = 210 * FWK_MHZ;
            if (platform_id == JUNO_IDX_PLATFORM_RTL) {
                /* Check that the signatures are present */
                fwk_assert(
                    ((struct juno_clock_hdlcd_lookup_high_pxlclk_enable *)
                        hdlcd_lookup)->SIGNATURE_A == 0x9DCA7B7A);
                fwk_assert(
                    ((struct juno_clock_hdlcd_lookup_high_pxlclk_enable *)
                        hdlcd_lookup)->SIGNATURE_B == 0x5C7852A9);
            }
            config->lookup_table =
                ((struct juno_clock_hdlcd_lookup_high_pxlclk_enable *)
                    HDLCD_PRESET_TABLE_BASE)->CLK;
        } else {
            config->lookup_table_count = JUNO_CLOCK_HDLCD_LOOKUP_COUNT;
            config->max_rate = 165 * FWK_MHZ;
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

    return juno_hdlcd_element_table;
}

struct fwk_module_config config_juno_hdlcd = {
    .get_element_table = juno_hdlcd_get_element_table,
};
