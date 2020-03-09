/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "n1sdp_scp_mmap.h"

#include <mod_n1sdp_ddr_phy.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

/* Table of N1SDP DDR PHY element descriptions. */
static struct fwk_element n1sdp_ddr_phy_element_table[] = {
    [0] = { .name = "DDR_PHY-0",
            .data = &((struct mod_n1sdp_ddr_phy_element_config) {
                .ddr = SCP_DDR_PHY0,
        }),
    },
    [1] = { .name = "DDR_PHY-1",
            .data = &((struct mod_n1sdp_ddr_phy_element_config) {
                .ddr = SCP_DDR_PHY1,
        }),
    },
    [2] = { 0 }, /* Termination description. */
};

static const struct fwk_element *n1sdp_ddr_phy_get_element_table
    (fwk_id_t module_id)
{
    return n1sdp_ddr_phy_element_table;
}

/* Configuration of the N1SDP DDR PHY module. */
const struct fwk_module_config config_n1sdp_ddr_phy = {
    .get_element_table = n1sdp_ddr_phy_get_element_table,
};
