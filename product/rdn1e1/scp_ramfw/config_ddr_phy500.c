/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "scp_system_mmap.h"

#include <mod_ddr_phy500.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stdbool.h>

/* Default configuration values for DDR PHY500 devices. */
static struct mod_ddr_phy500_reg ddr_reg_val = {
    .INIT_COMPLETE  = 0x00000001,
    .READ_DELAY     = 0x00000001,
    .CAPTURE_MASK   = 0x00000002,
    .T_CTRL_DELAY   = 0x0C000000,
    .T_WRLAT        = 0x00000001,
    .T_RDDATA_EN    = 0x00001600,
    .T_RDLAT        = 0x00000011,
    .DFI_LP_ACK     = 0x00030000,
    .DFI_WR_PREMBL  = 0x00000001,
    .DELAY_SEL      = 0x0000000D,
    .REF_EN         = 0x00000000,
    .T_CTRL_UPD_MIN = 0x00000000,
};

/* Table of DDR PHY500 element descriptions. */
static struct fwk_element ddr_phy500_element_table[] = {
    [0] = { .name = "DDR_PHY500-0",
            .data = &((struct mod_ddr_phy500_element_config) {
                .ddr = SCP_DDR_PHY0,
        }),
    },
    [1] = { .name = "DDR_PHY500-1",
            .data = &((struct mod_ddr_phy500_element_config) {
                .ddr = SCP_DDR_PHY1,
        }),
    },
    [2] = { 0 }, /* Termination description. */
};

static const struct fwk_element *ddr_phy500_get_element_table
    (fwk_id_t module_id)
{
    return ddr_phy500_element_table;
}

/* Configuration of the DDR PHY500 module. */
const struct fwk_module_config config_ddr_phy500 = {
    .get_element_table = ddr_phy500_get_element_table,
    .data = &((struct mod_ddr_phy500_module_config) {
            .ddr_reg_val = &ddr_reg_val,
            .initialize_init_complete = true,
            .initialize_ref_en = true,
        }),
};
