/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#define MOD_NAME "[LCP_PLATFORM] "

static int mod_lcp_platform_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    /* No elements support */
    if (element_count > 0) {
        return FWK_E_DATA;
    }

    return FWK_SUCCESS;
}

static int mod_lcp_platform_start(fwk_id_t id)
{
    FWK_LOG_INFO(MOD_NAME "LCP RAM firmware initialized\n");
    return FWK_SUCCESS;
}

const struct fwk_module module_lcp_platform = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = mod_lcp_platform_init,
    .start = mod_lcp_platform_start,
};

const struct fwk_module_config config_lcp_platform = { 0 };
