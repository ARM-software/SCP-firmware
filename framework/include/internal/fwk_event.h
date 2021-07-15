/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Framework internal header for Events as described in fwk_event.h.
 */

#ifndef FWK_INTERNAL_EVENT_H
#define FWK_INTERNAL_EVENT_H

/*!
 * \internal Event types.
 */
enum fwk_event_type {

    /*!
     * Event that includes 'params' and other information, usually
     * queued in event/isr queues and represented by
     * <tt> struct fwk_event' </tt>
     */
    FWK_EVENT_TYPE_STD,

    /*!
     *  Light event does not include 'params' and few other attributes as
     *  <tt> struct fwk_event </tt> type object and are only used to send
     *  directional event to target module. The object of these types
     *  are converted to \c FWK_EVENT_TYPE_STD type objects in a preallocated
     *  <tt> struct fwk_event </tt> type event before processing by the
     *  target module's <tt> process_event </tt> function.
     */
    FWK_EVENT_TYPE_LIGHT,

    /*! Count of Event types */
    FWK_EVENT_TYPE_COUNT,
};

#endif /* FWK_INTERNAL_EVENT_H */
