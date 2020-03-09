/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SGM775 System Support.
 */

#include "sgm775_scmi.h"
#include "sgm775_sds.h"

#include <mod_power_domain.h>
#include <mod_scmi.h>
#include <mod_sds.h>
#include <mod_sgm775_system.h>
#include <mod_system_power.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stdint.h>

static const uint32_t feature_flags = SGM775_SDS_FEATURE_FIRMWARE_MASK;
static fwk_id_t sds_feature_availability_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, 1);

/* SCMI services required to enable the messaging stack */
static unsigned int scmi_notification_table[] = {
    SGM775_SCMI_SERVICE_IDX_PSCI,
    SGM775_SCMI_SERVICE_IDX_OSPM_0,
    SGM775_SCMI_SERVICE_IDX_OSPM_1,
};

static struct mod_sds_api *sds_api;

/*
 * Static helpers
 */

static int messaging_stack_ready(void)
{
    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_feature_availability_id);

    /*
     * Write SDS Feature Availability to signal the completion of the messaging
     * stack
     */
    return sds_api->struct_write(sds_structure_desc->id,
        0, (void *)(&feature_flags), sds_structure_desc->size);
}

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

static int sgm775_system_bind(fwk_id_t id, unsigned int round)
{
    if (round > 0)
        return FWK_SUCCESS;

    return fwk_module_bind(fwk_module_id_sds,
                           FWK_ID_API(FWK_MODULE_IDX_SDS, 0),
                           &sds_api);
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
    static unsigned int scmi_notification_count = 0;
    static bool sds_notification_received = false;

    if (!fwk_expect(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE)))
        return FWK_E_PARAM;

    if (fwk_id_is_equal(event->id,
                               mod_scmi_notification_id_initialized)) {
        scmi_notification_count++;
    } else if (fwk_id_is_equal(event->id,
                               mod_sds_notification_id_initialized)) {
        sds_notification_received = true;
    } else
        return FWK_E_PARAM;

    if ((scmi_notification_count == FWK_ARRAY_SIZE(scmi_notification_table)) &&
        sds_notification_received) {
        messaging_stack_ready();

        scmi_notification_count = 0;
        sds_notification_received = false;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_sgm775_system = {
    .name = "SGM775_SYSTEM",
    .api_count = MOD_SGM775_SYSTEM_API_COUNT,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = sgm775_system_init,
    .bind = sgm775_system_bind,
    .process_bind_request = sgm775_system_process_bind_request,
    .start = sgm775_system_start,
    .process_notification = sgm775_system_process_notification,
};

/* No elements, no module configuration data */
struct fwk_module_config config_sgm775_system = { 0 };
