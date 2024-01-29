/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'dvfs_handler'.
 */

#include <lcp_css_mmap.h>
#include <lcp_platform_transport.h>

#include <mod_dvfs_handler.h>
#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct mod_dvfs_handler_opp dvfs_handler_opps[] = {
    { .level = 85UL, .frequency = 1700 * FWK_MHZ, .voltage = 850000 },
    { .level = 100UL, .frequency = 2000 * FWK_MHZ, .voltage = 900000 },
    { .level = 115UL, .frequency = 2300 * FWK_MHZ, .voltage = 950000 },
    { .level = 130UL, .frequency = 2600 * FWK_MHZ, .voltage = 950000 },
    { .level = 145UL, .frequency = 2900 * FWK_MHZ, .voltage = 1000000 },
    { .level = 160UL, .frequency = 3200 * FWK_MHZ, .voltage = 1050000 },
    { 0 }
};

static const struct mod_dvfs_handler_element_config lcp_cpu0_domain_config = {
    .sustained_idx = 4,
    .dvfs_frame_addr = LCP_DVFS_FRAME_BASE,
    .opps = dvfs_handler_opps,
    .opp_count = FWK_ARRAY_SIZE(dvfs_handler_opps) - 1,
    .status_check_max = 10,
    .transport_element_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_TRANSPORT,
        LCP_PLATFORM_TRANSPORT_IDX_DVFS_SET_LVL),
    .transport_fch_api_id = FWK_ID_API(
        FWK_MODULE_IDX_TRANSPORT,
        MOD_TRANSPORT_API_IDX_FAST_CHANNELS),
};

static const struct fwk_element element_table[] = {
    [0] = {
        .name = "LCP-CPU0-Domain",
        .data =
            (struct mod_dvfs_handler_element_config *)&lcp_cpu0_domain_config,
    },
    [1] = { 0 },
};

static const struct fwk_element *dvfs_handler_get_element_table(
    fwk_id_t module_id)
{
    return element_table;
}

const struct fwk_module_config config_dvfs_handler = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_handler_get_element_table)
};
