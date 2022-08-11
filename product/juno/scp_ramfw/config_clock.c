/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if (PLATFORM_VARIANT == JUNO_VARIANT_FVP)
#    include "config_mock_clock.h"

#    include <mod_mock_clock.h>
#endif

#include "config_power_domain.h"
#include "juno_clock.h"
#include "juno_id.h"

#include <mod_clock.h>
#include <mod_juno_cdcel937.h>
#include <mod_juno_hdlcd.h>
#include <mod_juno_soc_clock_ram.h>
#include <mod_power_domain.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct fwk_element clock_dev_desc_table[] = {
    [JUNO_CLOCK_IDX_I2SCLK] = {
        .name = "I2SCLK",
        .data = &((struct mod_clock_dev_config) {
            .pd_source_id = FWK_ID_NONE_INIT,
        }),
    },
    [JUNO_CLOCK_IDX_HDLCDREFCLK] = {
        .name = "HDLCDREFCLK",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
                JUNO_CLOCK_CDCEL937_IDX_HDLCDREFCLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
                                      MOD_JUNO_CDCEL937_API_IDX_CLOCK_DRIVER),
            .pd_source_id = FWK_ID_NONE_INIT,
        }),
    },
    [JUNO_CLOCK_IDX_HDLCDPXL] = {
        .name = "HDLCDPXL",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
                JUNO_CLOCK_CDCEL937_IDX_HDLCDPXL),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_CDCEL937,
                                      MOD_JUNO_CDCEL937_API_IDX_CLOCK_DRIVER),
            .pd_source_id = FWK_ID_NONE_INIT,
        }),
    },
    [JUNO_CLOCK_IDX_HDLCD0] = {
        .name = "HDLCD_0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_HDLCD,
                JUNO_CLOCK_HDLCD_IDX_HDLCD0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_HDLCD,
                                      MOD_JUNO_HDLCD_API_IDX_CLOCK_DRIVER),
            .pd_source_id = FWK_ID_NONE_INIT,
        }),
    },
    [JUNO_CLOCK_IDX_HDLCD1] = {
        .name = "HDLCD_1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_HDLCD,
                JUNO_CLOCK_HDLCD_IDX_HDLCD1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_HDLCD,
                                      MOD_JUNO_HDLCD_API_IDX_CLOCK_DRIVER),
            .pd_source_id = FWK_ID_NONE_INIT,
        }),
    },
    [JUNO_CLOCK_IDX_BIGCLK] = {
        .name = "BIG_CLK",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_SOC_CLOCK_RAM,
                JUNO_CLOCK_SOC_RAM_IDX_BIGCLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_SOC_CLOCK_RAM,
                                      MOD_JUNO_SOC_CLOCK_RAM_API_IDX_DRIVER),
            .pd_source_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                POWER_DOMAIN_IDX_BIG_SSTOP),
        }),
    },
    [JUNO_CLOCK_IDX_LITTLECLK] = {
        .name = "LITTLE_CLK",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_SOC_CLOCK_RAM,
                JUNO_CLOCK_SOC_RAM_IDX_LITTLECLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_SOC_CLOCK_RAM,
                                      MOD_JUNO_SOC_CLOCK_RAM_API_IDX_DRIVER),
            .pd_source_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                POWER_DOMAIN_IDX_LITTLE_SSTOP),
        }),
    },
    [JUNO_CLOCK_IDX_GPUCLK] = {
        .name = "GPU_CLK",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_SOC_CLOCK_RAM,
                JUNO_CLOCK_SOC_RAM_IDX_GPUCLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_SOC_CLOCK_RAM,
                                      MOD_JUNO_SOC_CLOCK_RAM_API_IDX_DRIVER),
            .pd_source_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                POWER_DOMAIN_IDX_GPUTOP),
        }),
    },
    [JUNO_CLOCK_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *clock_get_dev_desc_table(fwk_id_t module_id)
{
    struct mod_clock_dev_config *dev_cfg;

    /*
     * When running under an FVP the I2S clock does not fully function,
     * as it requires the I2C hardware interrupt to properly work. This
     * is replaced with the mock clock.
     *
     * Referencing and dereferencing is required to bypass the const qualifier.
     */
    dev_cfg = (struct mod_clock_dev_config
                   *)(clock_dev_desc_table[JUNO_CLOCK_IDX_I2SCLK].data);

#if (PLATFORM_VARIANT == JUNO_VARIANT_FVP)
    *((fwk_id_t *)(&dev_cfg->api_id)) = (fwk_id_t)FWK_ID_API_INIT(
        FWK_MODULE_IDX_MOCK_CLOCK, MOD_MOCK_CLOCK_API_TYPE_DRIVER);
    *((fwk_id_t *)(&dev_cfg->driver_id)) = (fwk_id_t)FWK_ID_ELEMENT_INIT(
        FWK_MODULE_IDX_MOCK_CLOCK, MOD_MOCK_CLOCK_ELEMENT_IDX_I2S);
#elif (PLATFORM_VARIANT == JUNO_VARIANT_BOARD)
    *((fwk_id_t *)(&dev_cfg->api_id)) = (fwk_id_t)FWK_ID_API_INIT(
        FWK_MODULE_IDX_JUNO_CDCEL937, MOD_JUNO_CDCEL937_API_IDX_CLOCK_DRIVER);
    *((fwk_id_t *)(&dev_cfg->driver_id)) = (fwk_id_t)FWK_ID_ELEMENT_INIT(
        FWK_MODULE_IDX_JUNO_CDCEL937, JUNO_CLOCK_CDCEL937_IDX_I2SCLK);
#endif

    return clock_dev_desc_table;
}

struct fwk_module_config config_clock = {
    .data =
        &(struct mod_clock_config){
            .pd_transition_notification_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_POWER_DOMAIN,
                MOD_PD_NOTIFICATION_IDX_POWER_STATE_TRANSITION),
            .pd_pre_transition_notification_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_POWER_DOMAIN,
                MOD_PD_NOTIFICATION_IDX_POWER_STATE_PRE_TRANSITION),
        },

    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(clock_get_dev_desc_table),
};
