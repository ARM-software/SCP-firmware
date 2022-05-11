/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <mod_reset_domain.h>
#include <mod_scmi_reset_domain.h>
#include <mod_juno_reset_domain.h>

static const struct mod_reset_domain_config reset_domain_config = {
#ifdef BUILD_HAS_NOTIFICATION
    .notification_id = FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_RESET_DOMAIN,
        MOD_RESET_DOMAIN_NOTIFICATION_AUTORESET),
#endif
};

/* Configuration of the reset elements */
static const struct fwk_element reset_domain_element_table[] = {
    [JUNO_RESET_DOMAIN_IDX_UART] = {
        .name = "JUNO_UART",
        .data = &((const struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_RESET_DOMAIN,
                                             JUNO_RESET_DOMAIN_IDX_UART),
            .driver_api_id = FWK_ID_API_INIT(
                                 FWK_MODULE_IDX_JUNO_RESET_DOMAIN,
                                 MOD_JUNO_RESET_DOMAIN_API_IDX_DRIVER),
            .modes = MOD_RESET_DOMAIN_AUTO_RESET,
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
            .capabilities = MOD_RESET_DOMAIN_CAP_NOTIFICATION,
#endif
            .latency = 0xFFFFFFFF
        }),
    },
    [JUNO_RESET_DOMAIN_IDX_COUNT] = { 0 }, /* Termination description */
};

static const struct fwk_element *get_reset_domain_element_table(
                                     fwk_id_t module_id)
{
    return reset_domain_element_table;
}

/* Configuration of the reset domain module */
struct fwk_module_config config_reset_domain = {
    .data = &reset_domain_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_reset_domain_element_table),
};
