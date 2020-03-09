/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "n1sdp_scp_mmap.h"

#include <mod_n1sdp_dmc620.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/* Table of DMC620 elements descriptions. */
static struct fwk_element dmc620_element_table[] = {
    [0] = { .name = "DMC620-0",
            .data = &((struct mod_dmc620_element_config) {
                .dmc = SCP_DMC0,
                .ddr_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_N1SDP_DDR_PHY, 0),
                .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT),
        }),
    },
    [1] = { .name = "DMC620-1",
            .data = &((struct mod_dmc620_element_config) {
                .dmc = SCP_DMC1,
                .ddr_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_N1SDP_DDR_PHY, 1),
                .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT),
        }),
    },
    [2] = { 0 }, /* Termination description. */
};

static const struct fwk_element *dmc620_get_element_table(fwk_id_t module_id)
{
    return dmc620_element_table;
}

/* Configuration of the DMC620 module. */
const struct fwk_module_config config_n1sdp_dmc620 = {
    .get_element_table = dmc620_get_element_table,
    .data = &((struct mod_dmc620_module_config) {
            .ddr_module_id = FWK_ID_MODULE_INIT(
                FWK_MODULE_IDX_N1SDP_DDR_PHY),
            .ddr_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_DDR_PHY,
                0),
            .ddr_speed = DDR_CLOCK_MHZ,
        }),
};
