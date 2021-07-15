/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_INTERNAL_THREAD_H
#define FWK_INTERNAL_THREAD_H

#include <fwk_event.h>
#include <fwk_noreturn.h>
#include <fwk_thread.h>

#include <stddef.h>

/*
 * \brief Initialize the thread framework component.
 *
 * \param event_count The maximum number of events in all queues at all time.
 *
 * \retval ::FWK_SUCCESS The thread framework component was initialized.
 * \retval ::FWK_E_NOMEM Insufficient memory available for event queues.
 */
int __fwk_thread_init(size_t event_count);

/*
 * \brief Begin waiting for and processing events raised by modules and
 *      interrupt handlers.
 *
 * \return The function does not return.
 */
noreturn void __fwk_thread_run(void);

/*
 * \brief Get the event being currently processed.
 *
 * \return The event being currently processed, or \c NULL if event processing
 *      has not yet begun.
 */
const struct fwk_event *__fwk_thread_get_current_event(void);

/*
 * \brief Put a notification event in one of the event queues.
 *
 * \details The thread component copies the notification event description into
 *      its internal data and so does not keep track of the pointer passed as a
 *      parameter.
 *
 *      If the function is called from a thread, the event is put into the
 *      targeted thread queue.
 *
 *      If the function is called from an ISR, the validity of the event source
 *      identifier is checked and the event is put in the ISR event queue.
 *
 * \param event Pointer to the notification event to queue.
 *
 * \retval ::FWK_SUCCESS The event was queued.
 * \retval ::FWK_E_PARAM The source identifier is not valid.
 * \retval ::FWK_E_NOMEM No memory space to copy the event data.
 */
int __fwk_thread_put_notification(struct fwk_event *event);

/*!
 * \brief Put an event in one of the event queues.
 *
 * \details The framework copies the event description into its internal data
 *      and so does not keep track of the pointer passed as a parameter.
 *
 *      If the function is called from a thread the event is put into the
 *      targeted thread queue. Furthermore, in the runtime phase, the source
 *      identifier of the event is populated with the identifier of the caller,
 *      and it is therefore unnecessary for the caller to do so manually. Note
 *      that this does not occur in the pre-runtime phase.
 *
 *      If the function is called from an ISR, the event is put in the ISR
 *      event queue.
 *
 *      In both cases, the event identifier and target identifier are checked
 *      to be valid and to refer to the same module.
 *
 *      In the case of a delayed response event, the event's 'is_response' flag
 *      must be set.
 *
 *      If multi-threading is enabled, and \p event is a delayed response, the
 *      ::fwk_event::cookie field of \p event must match that of the event
 *      it is responding to.
 *
 * \param[in] event Pointer to the event to queue. Must not be \c NULL.
 *
 * \retval ::FWK_SUCCESS The event was queued.
 * \retval ::FWK_E_INIT The thread framework component is not initialized.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `event` parameter was a null pointer value.
 *      - One or more fields of the event were invalid.
 * \retval ::FWK_E_OS Operating system error.
 *
 * \return Status code representing the result of the operation.
 */
int __fwk_thread_put_event(struct fwk_event *event);

/*!
 * \brief Put a light event by converting to a normal event in one of the event
 *     queues.
 *
 * \details The framework copies the light event description into its internal
 *      data(<tt> struct fwk_event </tt>) and so does not keep track of the
 *      pointer passed as a parameter.
 *
 *      If the function is called from a thread the event is put into the
 *      targeted thread queue. Furthermore, in the runtime phase, the source
 *      identifier of the event is populated with the identifier of the caller,
 *      and it is therefore unnecessary for the caller to do so manually. Note
 *      that this does not occur in the pre-runtime phase.
 *
 *      If the function is called from an ISR, the event is put in the ISR
 *      event queue.
 *
 *      In both cases, the event identifier and target identifier are checked
 *      to be valid and to refer to the same module.
 *
 *
 * \param[in] event Pointer to the event to queue. Must not be \c NULL.
 *
 * \retval ::FWK_SUCCESS The event was queued.
 * \retval ::FWK_E_INIT The thread framework component is not initialized.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `event` parameter was a null pointer value.
 *      - One or more fields of the event were invalid.
 * \retval ::FWK_E_OS Operating system error.
 *
 * \return Status code representing the result of the operation.
 */
int __fwk_thread_put_event_light(struct fwk_event_light *event);

#endif /* FWK_INTERNAL_THREAD_H */
