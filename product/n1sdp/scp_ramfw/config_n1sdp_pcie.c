/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_scp_mmap.h"

#include <mod_n1sdp_pcie.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stdbool.h>

static const struct fwk_element n1sdp_pcie_element_table[] = {
    [0] = {
        .name = "Generic-PCIe",
        .data = &((struct n1sdp_pcie_dev_config) {
            .ctrl_base = PCIE_IP_CFG_REG_SCP_BASE,
            .global_config_base = PCIE_RC_CFG_REG_SCP_BASE,
            .msg_base = PCIE_MSG_CFG_REG_SCP_BASE,
            .axi_slave_base32 = PCIE_AXI_SLAVE_SCP_BASE,
            .axi_slave_base64 = PCIE_AXI64_SLAVE_AP_BASE,
            .ccix_capable = false,
        }),
    },
    [1] = {
        .name = "CCIX-PCIe",
        .data = &((struct n1sdp_pcie_dev_config) {
            .ctrl_base = CCIX_IP_CFG_REG_SCP_BASE,
            .global_config_base = CCIX_RC_CFG_REG_SCP_BASE,
            .msg_base = CCIX_MSG_CFG_REG_SCP_BASE,
            .axi_slave_base32 = CCIX_AXI_SLAVE_SCP_BASE,
            .axi_slave_base64 = CCIX_AXI64_SLAVE_AP_BASE,
            .ccix_capable = true,
        }),
    },
    [2] = { 0 }, /* Termination description. */
};

static const struct fwk_element *n1sdp_pcie_get_element_table
    (fwk_id_t module_id)
{
    return n1sdp_pcie_element_table;
}

const struct fwk_module_config config_n1sdp_pcie = {
    .get_element_table = n1sdp_pcie_get_element_table,
};
