/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "system_mmap.h"

#include <mod_ddr_phy500.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

/* Default configuration values for DDR PHY500 devices. */
static const struct mod_ddr_phy500_reg ddr_reg_val = {
    .T_CTRL_DELAY = 0x00000000,
    .READ_DELAY = 0x00000003,
    .T_CTRL_UPD_MIN = 0x00000000,
    .DELAY_SEL = 0x0000000A,
    .CAPTURE_MASK = 0x0000001f,
    .T_RDDATA_EN = 0x00001C00,
    .T_RDLAT = 0x00000016,
    .T_WRLAT = 0x01000000,
    .DFI_WR_PREMBL = 0x00000002,
    .DFI_LP_ACK = 0x00641300,
};

/* Table of DDR PHY500 element descriptions. */
static struct fwk_element ddr_phy500_element_table[] = {
    [0] = { .name = "DDR_PHY500-0",
            .data = &((struct mod_ddr_phy500_element_config) {
                .ddr = DDR_PHY0,
        }),
    },
    [1] = { .name = "DDR_PHY500-1",
            .data = &((struct mod_ddr_phy500_element_config) {
                .ddr = DDR_PHY1,
        }),
    },
    [2] = { .name = "DDR_PHY500-2",
            .data = &((struct mod_ddr_phy500_element_config) {
                .ddr = DDR_PHY2,
        }),
    },
    [3] = { .name = "DDR_PHY500-3",
            .data = &((struct mod_ddr_phy500_element_config) {
                .ddr = DDR_PHY3,
        }),
    },
    [4] = { 0 }, /* Termination description. */
};

static const struct fwk_element *ddr_phy500_get_element_table(
    fwk_id_t module_id)
{
    return ddr_phy500_element_table;
}

/* Configuration of the DDR PHY500 module. */
const struct fwk_module_config config_ddr_phy500 = {
    .get_element_table = ddr_phy500_get_element_table,
    .data = &((struct mod_ddr_phy500_module_config) {
        .ddr_reg_val = &ddr_reg_val,
    }),
};
