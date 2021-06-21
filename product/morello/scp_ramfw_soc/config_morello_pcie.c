/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "morello_scp_mmap.h"

#include <mod_morello_pcie.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>

static struct morello_pcie_axi_ob_region_map pcie_axi_ob_mmap[13] = {
    /* ECAM region */
    {
        .base = PCIE_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(127) * FWK_GIB),
        .size = UINT64_C(256) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_ECAM,
    },
    /* Prefetchable MMIO64 region */
    {
        .base = PCIE_AXI64_SUBORDINATE_AP_BASE,
        .size = UINT64_C(4) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = PCIE_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(4) * FWK_GIB),
        .size = UINT64_C(8) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = PCIE_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(12) * FWK_GIB),
        .size = UINT64_C(16) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = PCIE_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(28) * FWK_GIB),
        .size = UINT64_C(64) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = PCIE_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(92) * FWK_GIB),
        .size = UINT64_C(32) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = PCIE_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(124) * FWK_GIB),
        .size = UINT64_C(2) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = PCIE_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(126) * FWK_GIB),
        .size = UINT64_C(1) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    /* Non-Prefetchable MMIO32 region*/
    {
        .base = PCIE_AXI32_SUBORDINATE_AP_BASE,
        .size = UINT64_C(128) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = PCIE_AXI32_SUBORDINATE_AP_BASE + (UINT64_C(128) * FWK_MIB),
        .size = UINT64_C(64) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = PCIE_AXI32_SUBORDINATE_AP_BASE + (UINT64_C(192) * FWK_MIB),
        .size = UINT64_C(32) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = PCIE_AXI32_SUBORDINATE_AP_BASE + (UINT64_C(224) * FWK_MIB),
        .size = UINT64_C(16) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    /* IO region*/
    {
        .base = PCIE_AXI32_SUBORDINATE_AP_BASE + (UINT64_C(240) * FWK_MIB),
        .size = UINT64_C(16) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_IO,
    },
};

static struct morello_pcie_axi_ob_region_map ccix_axi_ob_mmap[13] = {
    /* ECAM region */
    {
        .base = CCIX_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(127) * FWK_GIB),
        .size = UINT64_C(256) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_ECAM,
    },
    /* Prefetchable MMIO64 region */
    {
        .base = CCIX_AXI64_SUBORDINATE_AP_BASE,
        .size = UINT64_C(64) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = CCIX_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(64) * FWK_GIB),
        .size = UINT64_C(32) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = CCIX_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(96) * FWK_GIB),
        .size = UINT64_C(16) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = CCIX_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(112) * FWK_GIB),
        .size = UINT64_C(8) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = CCIX_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(120) * FWK_GIB),
        .size = UINT64_C(4) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = CCIX_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(124) * FWK_GIB),
        .size = UINT64_C(2) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = CCIX_AXI64_SUBORDINATE_AP_BASE + (UINT64_C(126) * FWK_GIB),
        .size = UINT64_C(1) * FWK_GIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    /* Non-Prefetchable MMIO32 region*/
    {
        .base = CCIX_AXI32_SUBORDINATE_AP_BASE,
        .size = UINT64_C(128) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = CCIX_AXI32_SUBORDINATE_AP_BASE + (UINT64_C(128) * FWK_MIB),
        .size = UINT64_C(64) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = CCIX_AXI32_SUBORDINATE_AP_BASE + (UINT64_C(192) * FWK_MIB),
        .size = UINT64_C(32) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    {
        .base = CCIX_AXI32_SUBORDINATE_AP_BASE + (UINT64_C(224) * FWK_MIB),
        .size = UINT64_C(16) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_MMIO,
    },
    /* IO region*/
    {
        .base = CCIX_AXI32_SUBORDINATE_AP_BASE + (UINT64_C(240) * FWK_MIB),
        .size = UINT64_C(16) * FWK_MIB,
        .type = PCIE_AXI_OB_REGION_TYPE_IO,
    },
};

static const struct fwk_element morello_pcie_element_table[3] = {
    [0] = {
        .name = "Generic-PCIe",
        .data = &((struct morello_pcie_dev_config) {
            .ctrl_base = PCIE_IP_CFG_REG_SCP_BASE,
            .global_config_base = PCIE_RC_CFG_REG_SCP_BASE,
            .msg_base = PCIE_MSG_CFG_REG_SCP_BASE,
            .axi_subordinate_base32 = PCIE_AXI_SUBORDINATE_SCP_BASE,
            .axi_subordinate_base64 = PCIE_AXI64_SUBORDINATE_AP_BASE,
            .ccix_capable = false,
            .axi_ob_table = pcie_axi_ob_mmap,
            .axi_ob_count = FWK_ARRAY_SIZE(pcie_axi_ob_mmap),
            .pri_bus_num = 0,
        }),
    },
    [1] = {
        .name = "CCIX-PCIe",
        .data = &((struct morello_pcie_dev_config) {
            .ctrl_base = CCIX_IP_CFG_REG_SCP_BASE,
            .global_config_base = CCIX_RC_CFG_REG_SCP_BASE,
            .msg_base = CCIX_MSG_CFG_REG_SCP_BASE,
            .axi_subordinate_base32 = CCIX_AXI_SUBORDINATE_SCP_BASE,
            .axi_subordinate_base64 = CCIX_AXI64_SUBORDINATE_AP_BASE,
            .ccix_capable = true,
            .axi_ob_table = ccix_axi_ob_mmap,
            .axi_ob_count = FWK_ARRAY_SIZE(ccix_axi_ob_mmap),
            .pri_bus_num = 0,
        }),
    },
    [2] = { 0 }, /* Termination description. */
};

static const struct fwk_element *morello_pcie_get_element_table(
    fwk_id_t module_id)
{
    return morello_pcie_element_table;
}

const struct fwk_module_config config_morello_pcie = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(morello_pcie_get_element_table),
};
