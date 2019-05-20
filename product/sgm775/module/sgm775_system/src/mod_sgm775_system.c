/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SGM775 System Support.
 */

#include <fmw_cmsis.h>
#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <mod_scmi.h>
#include <mod_sds.h>
#include <mod_system_power.h>
#include <mod_sgm775_system.h>
#include <sgm775_scmi.h>

/* SCMI services required to enable the messaging stack */
static unsigned int scmi_notification_table[] = {
    SGM775_SCMI_SERVICE_IDX_PSCI,
    SGM775_SCMI_SERVICE_IDX_OSPM_0,
    SGM775_SCMI_SERVICE_IDX_OSPM_1,
};

/*
 * Functions fulfilling the framework's module interface
 */

static int sgm775_system_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    NVIC_SystemReset();

    return FWK_E_DEVICE;
}

static const struct mod_system_power_driver_api
    sgm775_system_system_power_driver_api = {
    .system_shutdown = sgm775_system_shutdown
};

/*
 * Functions fulfilling the framework's module interface
 */

static int sgm775_system_init(fwk_id_t module_id, unsigned int unused,
                              const void *unused2)
{
    return FWK_SUCCESS;
}

static int sgm775_system_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    *api = &sgm775_system_system_power_driver_api;

    return FWK_SUCCESS;
}

static int sgm775_system_start(fwk_id_t id)
{
    int status;
    unsigned int i;

    /*
     * Subscribe to these SCMI channels in order to know when they have all
     * initialized.
     * At that point we can consider the SCMI stack to be initialized from
     * the point of view of the PSCI agent.
     */
    for (i = 0; i < FWK_ARRAY_SIZE(scmi_notification_table); i++) {
        status = fwk_notification_subscribe(
            mod_scmi_notification_id_initialized,
            fwk_id_build_element_id(fwk_module_id_scmi,
                scmi_notification_table[i]),
            id);
        if (status != FWK_SUCCESS)
            return status;
    }

    /*
     * Subscribe to the SDS initialized notification so we can correctly let the
     * PSCI agent know that the SCMI stack is initialized.
     */
    return fwk_notification_subscribe(
        mod_sds_notification_id_initialized,
        fwk_module_id_sds,
        id);
}

static int sgm775_system_process_notification(const struct fwk_event *event,
                                              struct fwk_event *resp_event)
{
    if (!fwk_expect(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE)))
        return FWK_E_PARAM;

    if (fwk_id_is_equal(event->id,
                               mod_scmi_notification_id_initialized)) {
        return FWK_SUCCESS;
    } else if (fwk_id_is_equal(event->id,
                               mod_sds_notification_id_initialized)) {
        return FWK_SUCCESS;
    }

    return FWK_E_PARAM;
}

const struct fwk_module module_sgm775_system = {
    .name = "SGM775_SYSTEM",
    .api_count = MOD_SGM775_SYSTEM_API_COUNT,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = sgm775_system_init,
    .process_bind_request = sgm775_system_process_bind_request,
    .start = sgm775_system_start,
    .process_notification = sgm775_system_process_notification,
};

/* No elements, no module configuration data */
struct fwk_module_config config_sgm775_system = { 0 };
