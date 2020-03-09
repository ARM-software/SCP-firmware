/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
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
 * \retval FWK_SUCCESS The thread framework component was initialized.
 * \retval FWK_E_NOMEM Insufficient memory available for event queues.
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
 * \retval FWK_SUCCESS The event was queued.
 * \retval FWK_E_PARAM The source identifier is not valid.
 * \retval FWK_E_NOMEM No memory space to copy the event data.
 */
int __fwk_thread_put_notification(struct fwk_event *event);

#endif /* FWK_INTERNAL_THREAD_H */
