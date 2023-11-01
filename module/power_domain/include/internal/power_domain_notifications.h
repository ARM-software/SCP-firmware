/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power domain notification handling.
 */

#ifndef POWER_DOMAIN_NOTIFICATIONS_H
#define POWER_DOMAIN_NOTIFICATIONS_H

#include <internal/power_domain.h>

#include <mod_power_domain.h>

#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>

/*
 * Process a notification recieved by the module.
 *
 * \param event Event to process.
 * \param resp Response event.
 *
 * \returns One of the standard response codes.
 */
int pd_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp);

/*
 * Process a response to the pre-shutdown notification.
 *
 * \returns One of the standard response codes.
 */
int process_pre_shutdown_notification_response(void);

/*
 * Process a response to the pre-transition notification.
 *
 * \param pd Power domain description.
 * \param params Pre-transition notification parameters.
 *
 * \returns One of the standard response codes.
 */
int process_power_state_pre_transition_notification_response(
    struct pd_ctx *pd,
    struct mod_pd_power_state_pre_transition_notification_resp_params *params);

/*
 * Process a response to the state transition notification.
 *
 * \param pd Power domain description.
 *
 * \returns One of the standard response codes.
 */
int process_power_state_transition_notification_response(struct pd_ctx *pd);

/*
 * Initiate a power state pre-transition notification if necessary.
 *
 * \param pd Description of the power domain to initiate the notification
 *      for.
 *
 * \retval true Waiting for notification responses.
 * \retval false Not waiting for any notification response.
 */
bool initiate_power_state_pre_transition_notification(struct pd_ctx *pd);

/*
 * Send pre-shutdown notification.
 *
 * \param system_shutdown Type of shutdown being initiated.
 *
 * \retval true At least one notification sent.
 * \retval false No notifications sent.
 */
bool check_and_notify_system_shutdown(
    enum mod_pd_system_shutdown system_shutdown);

/*
 * Send a warm reset notification.
 *
 * \return One of the standard return codes.
 */
int notify_warm_reset(void);

#endif /* POWER_DOMAIN_NOTIFICATIONS_H */
