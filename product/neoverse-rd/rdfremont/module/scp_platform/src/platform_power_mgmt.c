/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP Platform Support - Power Management
 */

#include <internal/scp_platform.h>

#include <mod_power_domain.h>
#include <mod_system_power.h>

#include <fwk_module.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

/* Module 'power_domain' restricted API pointer */
static struct mod_pd_restricted_api *pd_restricted_api;

/* System shutdown function */
static int platform_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    while (1) {
        __WFI();
    }

    return FWK_E_DEVICE;
}

/* Module 'system_power' driver interface */
const struct mod_system_power_driver_api platform_system_pwr_drv_api = {
    .system_shutdown = platform_shutdown,
};

const void *get_platform_system_power_driver_api(void)
{
    return &platform_system_pwr_drv_api;
}

int platform_power_mgmt_bind(void)
{
    return fwk_module_bind(
        fwk_module_id_power_domain,
        mod_pd_api_id_restricted,
        &pd_restricted_api);
}

int init_ap_core(uint8_t core_idx)
{
    bool resp_requested;
    uint32_t pd_state;
    fwk_id_t pd_id;

    pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, core_idx);

    /* Notification event at the end of request processing is not required */
    resp_requested = false;

    /* Composite Power Domain state to be set for the AP */
    pd_state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    return pd_restricted_api->set_state(pd_id, resp_requested, pd_state);
}
