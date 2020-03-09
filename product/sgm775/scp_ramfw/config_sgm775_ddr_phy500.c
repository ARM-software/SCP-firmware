/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "system_mmap.h"

#include <mod_sgm775_ddr_phy500.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stddef.h>

/* Table of DDR PHY500 element descriptions. */
static struct fwk_element sgm775_ddr_phy500_element_table[] = {
    [0] = { .name = "DDR_PHY500-0",
            .data = &((struct mod_sgm775_ddr_phy500_element_config) {
                .ddr = DDR_PHY0,
        }),
    },
    [1] = { .name = "DDR_PHY500-1",
            .data = &((struct mod_sgm775_ddr_phy500_element_config) {
                .ddr = DDR_PHY1,
        }),
    },
    [2] = { .name = "DDR_PHY500-2",
            .data = &((struct mod_sgm775_ddr_phy500_element_config) {
                .ddr = DDR_PHY2,
        }),
    },
    [3] = { .name = "DDR_PHY500-3",
            .data = &((struct mod_sgm775_ddr_phy500_element_config) {
                .ddr = DDR_PHY3,
        }),
    },
    [4] = { 0 }, /* Termination description. */
};

static const struct fwk_element *sgm775_ddr_phy500_get_element_table
    (fwk_id_t module_id)
{
    return sgm775_ddr_phy500_element_table;
}

/* Configuration of the DDR PHY500 module. */
struct fwk_module_config config_sgm775_ddr_phy500 = {
    .get_element_table = sgm775_ddr_phy500_get_element_table,
    .data = NULL,
};
