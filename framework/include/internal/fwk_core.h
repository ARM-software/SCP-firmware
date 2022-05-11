/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_INTERNAL_CORE_H
#define FWK_INTERNAL_CORE_H

#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_noreturn.h>

#include <stddef.h>

/*
 * \brief Initialize the core framework component.
 *
 * \param event_count The maximum number of events in all queues at all time.
 *
 * \retval ::FWK_SUCCESS The core framework component was initialized.
 * \retval ::FWK_E_NOMEM Insufficient memory available for event queues.
 */
int __fwk_init(size_t event_count);

/*
 * \brief Loop forever, processing events raised by modules and interrupt
 *      handlers. This function will suspend execution if the queue is empty and
 *      resume on an interrupt.
 *
 * \return The function does not return.
 */
noreturn void __fwk_run_main_loop(void);

/*
 * \brief Get the event being currently processed.
 *
 * \return The event being currently processed, or \c NULL if event processing
 *      has not yet begun.
 */
const struct fwk_event *__fwk_get_current_event(void);

/*
 * \brief Put a notification event in one of the event queues.
 *
 * \details The core component copies the notification event description into
 *      its internal data and so does not keep track of the pointer passed as a
 *      parameter.
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
int __fwk_put_notification(struct fwk_event *event);

/*!
 * \brief Put an event in one of the event queues.
 *
 * \details The framework copies the event description into its internal data
 *      and so does not keep track of the pointer passed as a parameter.
 *
 *      In the runtime phase, the source identifier of the event is populated
 *      with the identifier of the caller, and it is therefore unnecessary for
 *      the caller to do so manually. Note that this does not occur in the
 *      pre-runtime phase.
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
 *
 * \param[in] event Pointer to the event to queue. Must not be \c NULL.
 *
 * \retval ::FWK_SUCCESS The event was queued.
 * \retval ::FWK_E_INIT The core framework component is not initialized.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `event` parameter was a null pointer value.
 *      - One or more fields of the event were invalid.
 * \retval ::FWK_E_OS Operating system error.
 *
 * \return Status code representing the result of the operation.
 */
int __fwk_put_event(struct fwk_event *event);

/*!
 * \brief Put a light event by converting to a normal event in one of the event
 *     queues.
 *
 * \details The framework copies the light event description into its internal
 *      data(<tt> struct fwk_event </tt>) and so does not keep track of the
 *      pointer passed as a parameter.
 *
 *      In the runtime phase, the source identifier of the event is populated
 *      with the identifier of the caller, and it is therefore unnecessary for
 *      the caller to do so manually. Note that this does not occur in the
 *      pre-runtime phase.
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
 * \retval ::FWK_E_INIT The core framework component is not initialized.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `event` parameter was a null pointer value.
 *      - One or more fields of the event were invalid.
 * \retval ::FWK_E_OS Operating system error.
 *
 * \return Status code representing the result of the operation.
 */
int __fwk_put_event_light(struct fwk_event_light *event);

#endif /* FWK_INTERNAL_CORE_H */
