/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_mmap.h"

#include <mod_juno_ddr_phy400.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct fwk_element juno_ddr_phy400_element_table[] = {
    [0] = {
        .name = "",
        .data = &((struct mod_juno_ddr_phy400_element_config) {
            .ddr_phy_ptm = (DDR_PHY0_BASE + OFFSET_DDR_PHY400_PTM_REGS),
            .ddr_phy_c3a = (DDR_PHY0_BASE + OFFSET_DDR_PHY400_C3A_REGS),
            .ddr_phy_bl0 = (DDR_PHY0_BASE + OFFSET_DDR_PHY400_BL0_REGS),
            .ddr_phy_bl1 = (DDR_PHY0_BASE + OFFSET_DDR_PHY400_BL1_REGS),
            .ddr_phy_bl2 = (DDR_PHY0_BASE + OFFSET_DDR_PHY400_BL2_REGS),
            .ddr_phy_bl3 = (DDR_PHY0_BASE + OFFSET_DDR_PHY400_BL3_REGS),
        })
    },
    [1] = {
        .name = "",
        .data = &((struct mod_juno_ddr_phy400_element_config) {
            .ddr_phy_ptm = (DDR_PHY1_BASE + OFFSET_DDR_PHY400_PTM_REGS),
            .ddr_phy_c3a = (DDR_PHY1_BASE + OFFSET_DDR_PHY400_C3A_REGS),
            .ddr_phy_bl0 = (DDR_PHY1_BASE + OFFSET_DDR_PHY400_BL0_REGS),
            .ddr_phy_bl1 = (DDR_PHY1_BASE + OFFSET_DDR_PHY400_BL1_REGS),
            .ddr_phy_bl2 = (DDR_PHY1_BASE + OFFSET_DDR_PHY400_BL2_REGS),
            .ddr_phy_bl3 = (DDR_PHY1_BASE + OFFSET_DDR_PHY400_BL3_REGS),
        })
    },
    [2] = { 0 }, /* Termination description. */
};

static const struct fwk_element *juno_ddr_phy400_get_element_table
    (fwk_id_t module_id)
{
    return juno_ddr_phy400_element_table;
}

struct fwk_module_config config_juno_ddr_phy400 = {
    .get_element_table = juno_ddr_phy400_get_element_table,
    .data = &((struct mod_juno_ddr_phy400_config) {
        .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
    })
};
