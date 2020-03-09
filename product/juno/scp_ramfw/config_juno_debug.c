/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_power_domain.h>
#include <juno_id.h>
#include <system_clock.h>

#include <mod_debug.h>
#include <mod_juno_debug.h>

#include <fwk_element.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/* Debug clock settings for Juno with default values for R0 */
static struct juno_css_debug_dev clock_settings = {
    .div_atclk = (SYSINCLK / (533UL * FWK_MHZ)),
    .manual_reset_required = true,
    .div_pclk = 1,
    .div_traceclk = (SYSINCLK / (150UL * FWK_MHZ)),
};

static struct fwk_element juno_debug_element_table[] = {
    [0] = {
        .name = "JUNO DEBUG",
        .data = &((struct mod_juno_debug_dev_config){
            .pd_dbgsys_id =
                FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                                    POWER_DOMAIN_IDX_DBGSYS),
            .pd_big_sstop_id =
                FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                                    POWER_DOMAIN_IDX_BIG_SSTOP),
            .pd_little_sstop_id =
                FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                                    POWER_DOMAIN_IDX_LITTLE_SSTOP),
        }),
    },
    [1] = { 0 }, /* Termination description */
};

static const struct fwk_element *get_juno_debug_elem_table(fwk_id_t module_id)
{
    int status;
    enum juno_idx_revision revision;
    struct mod_juno_debug_dev_config *juno_debug_config;

    status = juno_id_get_revision(&revision);
    if (status != FWK_SUCCESS)
        return NULL;

    juno_debug_config = (struct mod_juno_debug_dev_config *)
        juno_debug_element_table[0].data;

    juno_debug_config->clk_settings = &clock_settings;

    /* Adjust Debug clock settings for Juno R1 & Juno R2 */
    if (revision != JUNO_IDX_REVISION_R0) {
        juno_debug_config->clk_settings->div_atclk =
            (SYSINCLK / (400UL * FWK_MHZ)),
        juno_debug_config->clk_settings->manual_reset_required = false;
    }

    return juno_debug_element_table;
}

struct fwk_module_config config_juno_debug = {
    .get_element_table = get_juno_debug_elem_table,
    .data = &((struct mod_juno_debug_config) {
        .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
    }),
};
