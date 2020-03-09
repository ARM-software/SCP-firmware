/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "system_mmap.h"

#include <mod_sgm775_dmc500.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct fwk_element sgm775_dmc500_element_table[] = {
    [0] = { .name = "DMC500-0",
            .data = &((struct mod_sgm775_dmc500_element_config) {
                .dmc = DMC_INTERNAL0,
                .ddr_phy_id =
                    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SGM775_DDR_PHY500, 0),
        }),
    },
    [1] = { .name = "DMC500-1",
            .data = &((struct mod_sgm775_dmc500_element_config) {
                .dmc = DMC_INTERNAL1,
                .ddr_phy_id =
                    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SGM775_DDR_PHY500, 1),
        }),
    },
    [2] = { .name = "DMC500-2",
            .data = &((struct mod_sgm775_dmc500_element_config) {
                .dmc = DMC_INTERNAL2,
                .ddr_phy_id =
                    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SGM775_DDR_PHY500, 2),
        }),
    },
    [3] = { .name = "DMC500-3",
            .data = &((struct mod_sgm775_dmc500_element_config) {
                .dmc = DMC_INTERNAL3,
                .ddr_phy_id =
                    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SGM775_DDR_PHY500, 3),
        }),
    },
    [4] = { 0 }, /* Termination description. */
};

static const struct fwk_element *sgm775_dmc500_get_element_table(
    fwk_id_t module_id)
{
    return sgm775_dmc500_element_table;
}

/* Configuration of the DMC500 module. */
struct fwk_module_config config_sgm775_dmc500 = {
    .get_element_table = sgm775_dmc500_get_element_table,
    .data = &((struct mod_sgm775_dmc500_module_config) {
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
        }),
};
