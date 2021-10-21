/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "morello_scp_mmap.h"

#include <mod_dmc_bing.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>

/* Table of DMC_BING elements descriptions. */
static struct fwk_element dmc_bing_element_table[] = {
    [0] =
        {
            .name = "DMC_BING-0",
            .data = &((struct mod_dmc_bing_element_config){
                .dmc_bing_base = SCP_DMC0,
                .ddr_phy_base = SCP_DDR_PHY0,
                .clock_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT),
            }),
        },
    [1] =
        {
            .name = "DMC_BING-1",
            .data = &((struct mod_dmc_bing_element_config){
                .dmc_bing_base = SCP_DMC1,
                .ddr_phy_base = SCP_DDR_PHY1,
                .clock_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT),
            }),
        },
    [2] = { 0 }, /* Termination description. */
};

static const struct fwk_element *dmc_bing_get_element_table(fwk_id_t module_id)
{
    return dmc_bing_element_table;
}

/* Configuration of the DMC_BING module. */
const struct fwk_module_config config_dmc_bing = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dmc_bing_get_element_table),
    .data = &((struct mod_dmc_bing_module_config){
        .ddr_speed = DDR_CLOCK_MHZ,
    }),
};
