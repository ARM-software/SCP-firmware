/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_MULTI_THREAD_H
#define FWK_MULTI_THREAD_H

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_thread.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * @{
 */

/*!
 * \defgroup GroupThread Threading
 *
 * @{
 */

/*!
 * \brief Create a thread for a module or element
 *
 * \details Create a thread for a module or element as well as its event queue.
 *      Modules and elements bound to the module or element can then place
 *      events in its queue using the 'fwk_thread_put_event()' function from
 *      the framework API.
 *
 *      Events will be processed by the created thread when the framework calls
 *      the module or element's 'process_event()' function. Note that the thread
 *      will process only a single event from the queue each time the
 *      'process_event()' function is called.
 *
 * \param id Identifier of the module or element to create the thread for.
 *
 * \retval FWK_SUCCESS The module/element thread was created.
 * \retval FWK_E_PARAM The identifier is not a valid module or element
 *     identifier.
 * \retval FWK_E_INIT The thread framework component is not initialized.
 * \retval FWK_E_STATE The execution has already started, or it is not possible
 *      to create a thread anymore, or a thread has already been created for the
 *      given module or element.
 * \retval FWK_E_NOMEM A memory allocation failed.
 * \retval FWK_E_OS Operating system error.
 */
int fwk_thread_create(fwk_id_t id);

/*!
 * \brief Put an event in a module or element thread queue and wait for it to
 *      be processed.
 *
 * \details This framework API function can only be called from a module or
 *      element's thread. The calling thread is suspended until the event
 *      has been completely processed. As a consequence, this function cannot
 *      be called during the pre-runtime phases.
 *
 *      The identifier of the event's source is filled in by the framework
 *      with the identifier of the entity calling the function. Thus there is
 *      no need for the caller to fill this event's field in.
 *
 *      The event identifier and target identifier are validated and must
 *      belong to the same module.
 *
 * \param event Event to put into the queue for processing. Must not be \c NULL.
 * \param resp_event Pointer to storage for the response event. Must not be \c
 *      NULL.
 *
 * \retval FWK_SUCCESS The event was successfully processed.
 * \retval FWK_E_STATE The execution is not started.
 * \retval FWK_E_PARAM One or more of the parameters were invalid.
 * \retval FWK_E_PARAM One or more fields in the \p event parameter were
 *      invalid.
 * \retval FWK_E_ACCESS The API is called from an ISR, called from the common
 *      thread, or the event targets the calling thread.
 */
int fwk_thread_put_event_and_wait(struct fwk_event *event,
                                  struct fwk_event *resp_event);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* FWK_MULTI_THREAD_H */
