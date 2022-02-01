/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Notification definitions.
 */

#ifndef FWK_NOTIFICATION_H
#define FWK_NOTIFICATION_H

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>

#if FWK_HAS_INCLUDE(<fmw_notification.h>)
#    include <fmw_notification.h> /* cppcheck-suppress missingIncludeSystem */
#endif

/*!
 * \ingroup GroupLibFramework
 * \defgroup GroupNotification Notifications
 * \{
 */

/*!
 * \def FMW_NOTIFICATION_MAX
 *
 * \brief Maximum number of active notifications.
 *
 * \details This value represents the maximum number of notifications that can
 *      be active at any one time.
 */
#ifndef FMW_NOTIFICATION_MAX
#    define FMW_NOTIFICATION_MAX 64
#endif

/*!
 * \brief Subscribe to a notification.
 *
 * \param notification_id Identifier of the notification.
 * \param source_id Notification source identifier.
 * \param target_id Notification target identifier.
 *
 * \retval ::FWK_SUCCESS The subscription was successful.
 * \retval ::FWK_E_INIT The notification component has not been initialized.
 * \retval ::FWK_E_HANDLER The function was called from an interrupt handler.
 * \retval ::FWK_E_PARAM One or more identifiers were invalid.
 * \retval ::FWK_E_STATE The entity \p target_id has already subscribed to the
 *      notification \p notification_id from the entity \p source_id.
 * \retval ::FWK_E_NOMEM The maximum number of subscriptions has been reached.
 */
int fwk_notification_subscribe(fwk_id_t notification_id, fwk_id_t source_id,
                               fwk_id_t target_id);

/*!
 * \brief Unsubscribe from a notification.
 *
 * \param notification_id Identifier of the notification.
 * \param source_id Notification source identifier.
 * \param target_id Notification target identifier.
 *
 * \retval ::FWK_SUCCESS The subscription was successfully cancelled.
 * \retval ::FWK_E_INIT The notification component has not been initialized.
 * \retval ::FWK_E_HANDLER The function was called from an interrupt handler.
 * \retval ::FWK_E_PARAM One or more identifiers were invalid.
 * \retval ::FWK_E_STATE The entity \p target_id has not subscribed to the
 *      notification \p notification_id from the entity \p source_id.
 */
int fwk_notification_unsubscribe(fwk_id_t notification_id, fwk_id_t source_id,
                                 fwk_id_t target_id);

/*!
 * \brief Send a notification to all entities that are subscribed to it.
 *
 * \note During the runtime phase, if the source identifier of the notification
 *      event is not a valid entity identifier, the event source identifier is
 *      populated automatically by the framework with the identifier of the
 *      entity target of the event being currently processed.
 *
 * \param notification_event Pointer to the notification event. Must not be
 *      \c NULL.
 * \param [out] count Number of notification events that were sent. Must not be
 *      \c NULL.
 *
 * \retval ::FWK_SUCCESS All subscribers were notified successfully.
 * \retval ::FWK_E_INIT The notification component has not been initialized.
 * \retval ::FWK_E_PARAM One of more parameters were invalid.
 */
int fwk_notification_notify(struct fwk_event *notification_event,
                            unsigned int *count);

/*!
 * \}
 */

#endif /* FWK_NOTIFICATION_H */
