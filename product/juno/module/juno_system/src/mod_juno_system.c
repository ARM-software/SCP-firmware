/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fmw_cmsis.h>
#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <mod_juno_system.h>
#include <mod_juno_xrp7724.h>
#include <mod_scmi.h>
#include <mod_sds.h>
#include <mod_system_power.h>
#include <juno_id.h>
#include <juno_scmi.h>
#include <juno_sds.h>

static fwk_id_t sds_feature_availability_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, JUNO_SDS_RAM_FEATURES_IDX);

/* SCMI services required to enable the messaging stack */
static unsigned int scmi_notification_table[] = {
    JUNO_SCMI_SERVICE_IDX_PSCI,
    JUNO_SCMI_SERVICE_IDX_OSPM_0,
    JUNO_SCMI_SERVICE_IDX_OSPM_1,
};

struct juno_system_ctx {
    /* SDS API */
    struct mod_sds_api *sds_api;

    /* XRP7724 PMIC API */
    const struct mod_juno_xrp7724_api_system_mode *juno_xrp7724_api;

    /* Running platform identifier */
    enum juno_idx_platform platform_id;
};

static struct juno_system_ctx juno_system_ctx;

/*
 * Static helpers
 */

static int messaging_stack_ready(void)
{
    const uint32_t feature_flags = JUNO_SDS_FEATURE_FIRMWARE_MASK;

    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_feature_availability_id);

    /*
     * Write SDS Feature Availability to signal the completion of the messaging
     * stack
     */
    return juno_system_ctx.sds_api->struct_write(sds_structure_desc->id,
        0, (void *)(&feature_flags), sds_structure_desc->size);
}

/*
 * Functions fulfilling the System Power's API
 */
static int juno_system_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    if (juno_system_ctx.platform_id == JUNO_IDX_PLATFORM_FVP) {
        NVIC_SystemReset();

        fwk_unreachable();
    }

    switch (system_shutdown) {
    case MOD_PD_SYSTEM_SHUTDOWN:
        juno_system_ctx.juno_xrp7724_api->shutdown();
        break;

    case MOD_PD_SYSTEM_COLD_RESET:
        juno_system_ctx.juno_xrp7724_api->reset();
        break;

    default:
        return FWK_E_SUPPORT;
    }

    return FWK_SUCCESS;
}

static const struct mod_system_power_driver_api
    juno_system_system_power_driver_api = {
        .system_shutdown = juno_system_shutdown,
};

/*
 * Framework API
 */
static int juno_system_module_init(fwk_id_t module_id,
                                   unsigned int element_count,
                                   const void *data)
{
    int status;

    fwk_assert(element_count == 0);

    status = juno_id_get_platform(&juno_system_ctx.platform_id);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int juno_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round > 0)
        return FWK_SUCCESS;

    status = fwk_module_bind(fwk_module_id_sds,
                             FWK_ID_API(FWK_MODULE_IDX_SDS, 0),
                             &juno_system_ctx.sds_api);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    if (juno_system_ctx.platform_id != JUNO_IDX_PLATFORM_RTL)
        return FWK_SUCCESS;
    else {
        return fwk_module_bind(fwk_module_id_juno_xrp7724,
            mod_juno_xrp7724_api_id_system_mode,
            &juno_system_ctx.juno_xrp7724_api);
    }
}

static int juno_system_process_bind_request(fwk_id_t source_id,
                                            fwk_id_t target_id,
                                            fwk_id_t api_id,
                                            const void **api)
{
    *api = &juno_system_system_power_driver_api;

    return FWK_SUCCESS;
}

static int juno_system_start(fwk_id_t id)
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

static int juno_system_process_notification(const struct fwk_event *event,
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

const struct fwk_module module_juno_system = {
    .name = "Juno SYSTEM",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = juno_system_module_init,
    .bind = juno_system_bind,
    .process_bind_request = juno_system_process_bind_request,
    .start = juno_system_start,
    .process_notification = juno_system_process_notification,
};

/* No elements, no module configuration data */
struct fwk_module_config config_juno_system = { 0 };
