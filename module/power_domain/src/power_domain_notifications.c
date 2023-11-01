/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power domain notification handling.
 */

#include <internal/power_domain_notifications.h>

#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_notification.h>

#ifdef BUILD_HAS_NOTIFICATION
/*
 * Check whether a power state pre-transition notification must be sent.
 *
 * \param pd Description of the power domain
 * \param state Power state the power domain has to transit to
 *
 * \retval true A power state pre-transition notification must be sent.
 * \retval false A power state pre-transition notification doesn't have to be
 *      sent.
 */
static bool check_power_state_pre_transition_notification(
    struct pd_ctx *pd,
    unsigned int state)
{
    if ((state == pd->power_state_pre_transition_notification_ctx.state) &&
        pd->power_state_pre_transition_notification_ctx.valid) {
        return (
            pd->power_state_pre_transition_notification_ctx.response_status !=
            FWK_SUCCESS);
    }

    return true;
}

int pd_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    struct pd_ctx *pd;

    /* Only responses are expected. */
    if (!event->is_response) {
        fwk_unexpected();
        return FWK_E_SUPPORT;
    }

    if (fwk_id_is_equal(event->id, mod_pd_notification_id_pre_shutdown)) {
        return process_pre_shutdown_notification_response();
    }

    if (!fwk_module_is_valid_element_id(event->target_id)) {
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(event->target_id)];

    if (fwk_id_is_equal(
            event->id, mod_pd_notification_id_power_state_transition)) {
        return process_power_state_transition_notification_response(pd);
    }

    return process_power_state_pre_transition_notification_response(
        pd,
        (struct mod_pd_power_state_pre_transition_notification_resp_params *)
            event->params);
}

int process_pre_shutdown_notification_response(void)
{
    if (mod_pd_ctx.system_shutdown.ongoing) {
        mod_pd_ctx.system_shutdown.notifications_count--;

        if (mod_pd_ctx.system_shutdown.notifications_count == 0) {
            /* All notifications for system shutdown have been received */
            perform_shutdown(mod_pd_ctx.system_shutdown.system_shutdown);
        }
        return FWK_SUCCESS;
    } else {
        return FWK_E_PARAM;
    }
}

int process_power_state_pre_transition_notification_response(
    struct pd_ctx *pd,
    struct mod_pd_power_state_pre_transition_notification_resp_params *params)
{
    if (pd->power_state_pre_transition_notification_ctx.pending_responses ==
        0) {
        fwk_unexpected();
        return FWK_E_PANIC;
    }

    if (params->status != FWK_SUCCESS) {
        pd->power_state_pre_transition_notification_ctx.response_status =
            FWK_E_DEVICE;
    }

    pd->power_state_pre_transition_notification_ctx.pending_responses--;
    if (pd->power_state_pre_transition_notification_ctx.pending_responses !=
        0) {
        return FWK_SUCCESS;
    }

    if (pd->power_state_pre_transition_notification_ctx.valid == true) {
        /*
         * All the notification responses have been received, the requested
         * state for the power domain has not changed in the
         * meantime and all the notified entities agreed on the power state
         * transition, proceed with it.
         */
        if (pd->power_state_pre_transition_notification_ctx.response_status ==
            FWK_SUCCESS) {
            return initiate_power_state_transition(pd);
        }
    } else {
        /*
         * All the notification responses have been received but the
         * requested state for the power domain has changed, start the
         * processings for the new requested state.
         */
        if ((pd->requested_state == pd->state_requested_to_driver) ||
            (!is_allowed_by_parent_and_children(pd, pd->requested_state))) {
            return FWK_SUCCESS;
        }

        if (!initiate_power_state_pre_transition_notification(pd)) {
            return initiate_power_state_transition(pd);
        }
    }

    return FWK_SUCCESS;
}

int process_power_state_transition_notification_response(struct pd_ctx *pd)
{
    struct fwk_event notification_event = { 0 };
    struct mod_pd_power_state_transition_notification_params *params;

    if (pd->power_state_transition_notification_ctx.pending_responses == 0) {
        fwk_unexpected();
        return FWK_E_PANIC;
    }

    pd->power_state_transition_notification_ctx.pending_responses--;
    if (pd->power_state_transition_notification_ctx.pending_responses != 0) {
        return FWK_SUCCESS;
    }

    if (pd->power_state_transition_notification_ctx.state ==
        pd->current_state) {
        /* All notifications received, complete the transition report */

        unsigned int previous_state =
            pd->power_state_transition_notification_ctx.previous_state;

        pd->power_state_transition_notification_ctx.previous_state =
            pd->current_state;
        /*
         * Complete the report state change now that we have all notifications
         */
        if (is_deeper_state(pd->current_state, previous_state)) {
            process_power_state_transition_report_deeper_state(pd);
        } else if (is_shallower_state(pd->current_state, previous_state)) {
            process_power_state_transition_report_shallower_state(pd);
        }

        return FWK_SUCCESS;
    }

    /*
     * While receiving the responses, the power state of the power domain
     * has changed. Send a notification for the current power state.
     */
    notification_event.id = mod_pd_notification_id_power_state_transition;
    notification_event.response_requested = true;
    notification_event.source_id = FWK_ID_NONE;

    params = (struct mod_pd_power_state_transition_notification_params *)
                 notification_event.params;
    params->state = pd->current_state;

    pd->power_state_transition_notification_ctx.state = pd->current_state;
    return fwk_notification_notify(
        &notification_event,
        &pd->power_state_transition_notification_ctx.pending_responses);
}
#endif /* BUILD_HAS_NOTIFICATION */

bool initiate_power_state_pre_transition_notification(struct pd_ctx *pd)
{
#ifdef BUILD_HAS_NOTIFICATION
    unsigned int state;
    struct fwk_event notification_event = {
        .id = mod_pd_notification_id_power_state_pre_transition,
        .response_requested = true,
        .source_id = FWK_ID_NONE
    };
    struct mod_pd_power_state_pre_transition_notification_params *params;
    int status;

    if (pd->config->disable_state_transition_notifications == true) {
        return false;
    }

    state = pd->requested_state;
    if (!check_power_state_pre_transition_notification(pd, state)) {
        return false;
    }

    /*
     * If still waiting for some responses on the previous power state
     * pre-transition notification, wait for them before to issue the next one.
     */
    if (pd->power_state_pre_transition_notification_ctx.pending_responses !=
        0) {
        return true;
    }

    params = (struct mod_pd_power_state_pre_transition_notification_params *)
                 notification_event.params;
    params->current_state = pd->current_state;
    params->target_state = state;

    notification_event.source_id = pd->id;
    status = fwk_notification_notify(
        &notification_event,
        &pd->power_state_pre_transition_notification_ctx.pending_responses);
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[PD] %s @%d", __func__, __LINE__);
    }

    pd->power_state_pre_transition_notification_ctx.state = state;
    pd->power_state_pre_transition_notification_ctx.response_status =
        FWK_SUCCESS;
    pd->power_state_pre_transition_notification_ctx.valid = true;

    return (
        pd->power_state_pre_transition_notification_ctx.pending_responses != 0);
#else
    return false;
#endif
}

bool check_and_notify_system_shutdown(
    enum mod_pd_system_shutdown system_shutdown)
{
#ifdef BUILD_HAS_NOTIFICATION
    struct mod_pd_pre_shutdown_notif_params *params;
    int status;

    struct fwk_event notification = { .id = mod_pd_notification_id_pre_shutdown,
                                      .source_id = fwk_module_id_power_domain,
                                      .response_requested = true };

    params = (struct mod_pd_pre_shutdown_notif_params *)notification.params;
    params->system_shutdown = system_shutdown;

    status = fwk_notification_notify(
        &notification, &mod_pd_ctx.system_shutdown.notifications_count);
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[PD] %s @%d", __func__, __LINE__);
    }

    return (mod_pd_ctx.system_shutdown.notifications_count != 0);
#else
    return false;
#endif
}

int notify_warm_reset(void)
{
#ifdef BUILD_HAS_NOTIFICATION
    unsigned int count;
    struct fwk_event notification = {
        .id = mod_pd_notification_id_pre_warm_reset,
        .source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_POWER_DOMAIN),
    };

    return fwk_notification_notify(&notification, &count);
#else
    return FWK_SUCCESS;
#endif
}
