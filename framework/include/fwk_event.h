/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Events.
 */

#ifndef FWK_EVENT_H
#define FWK_EVENT_H

#include <fwk_align.h>
#include <fwk_id.h>
#include <fwk_list.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * @{
 */

/*!
 * \defgroup GroupEvent Events
 * @{
 */

/*!
 * \brief Number of bytes available within each event definition for
 *     event-specific parameters.
 */
#define FWK_EVENT_PARAMETERS_SIZE 16

/*!
 * \brief Event.
 *
 * \details Events are used to facilitate event-based inter-process
 *      communication between modules. Each event represents an asynchronous
 *      message to one module from another, to which the target module can
 *      respond if necessary.
 */
struct fwk_event {
    /*!
     * \internal
     * \brief Linked list node.
     */
    struct fwk_slist_node slist_node;

    /*! Identifier of the event source */
    fwk_id_t source_id;

    /*! Identifier of the event target */
    fwk_id_t target_id;

    /*!
     *  Unique event number used to identify an event. The cookie is
     *  automatically set by the framework.
     */
    uint32_t cookie;

    /*! Flag indicating whether the event is a response to another event */
    bool is_response;

    /*! Flag indicating whether the event source expects a response */
    bool response_requested;

    /*! Flag indicating whether the event is a notification */
    bool is_notification;

    /*!
     * \brief Flag indicating whether the event is a delayed response
     */
    bool is_delayed_response;

    /*!
     * \internal
     * \brief Flag indicating whether the event is a response event that a
     *      thread is waiting for to resume execution.
     */
    bool is_thread_wakeup_event;

    /*!
     * \brief Event identifier.
     *
     * \details Each module or element may define its own set of events. The
     *      event identifier must therefore be interpreted in the context of its
     *      target.
     */
    fwk_id_t id;

    /*! Table of event parameters */
    alignas(max_align_t) uint8_t params[FWK_EVENT_PARAMETERS_SIZE];
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* FWK_EVENT_H */
