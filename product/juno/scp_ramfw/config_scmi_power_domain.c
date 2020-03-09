/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_power_domain.h>

#include <mod_scmi_power_domain.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#if BUILD_HAS_MOD_DEBUG
struct fwk_module_config config_scmi_power_domain = {
    .data = &((struct mod_scmi_pd_config) {
        .debug_pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                                           POWER_DOMAIN_IDX_DBGSYS),
        .debug_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DEBUG, 0),
    }),
};
#else
/* No elements, no module configuration data */
struct fwk_module_config config_scmi_power_domain = { 0 };
#endif
