/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *    SIgnal definitions.
 */

#ifndef FWK_SIGNAL_H
#define FWK_SIGNAL_H

#include <fwk_id.h>

#include <stdbool.h>
#include <stdint.h>

/*
 * \def FWK_MODULE_SIGNAL_COUNT
 *
 * \brief Number of pending signals. If we run out of signals then the
 *      platform should revert to using events.
 */
#if FMW_SIGNAL_MAX > 8
#    define FWK_MODULE_SIGNAL_COUNT FMW_SIGNAL_MAX
#else
#    define FWK_MODULE_SIGNAL_COUNT 8
#endif

/*!
 * \brief Post a signal into the signal queue.
 *
 * \details The framework copies the signal description into its internal data.
 *      The signal will be sent to the target module or element before any
 * pending events, ie, signals will always take precedence. Note that signals
 * must not be used where a response is required.
 *
 * \param[in] source_id Identifier of the module or element sending the
 *      signal.
 * \param[in] target_id Identifier of the module or element targeted by the
 *      signal.
 * \param[in] signal_id Identifier of the signal.
 *
 * \retval ::FWK_SUCCESS The signal was queued.
 * \retval ::FWK_E_BUSY The signal could not be queued due to resource
 *          constraints.
 * \retval ::FWK_E_SUPPORT The platform does not implement the
 *          signal function.
 *
 * \return Status code representing the result of the operation.
 */
int fwk_thread_put_signal(
    const fwk_id_t source_id,
    const fwk_id_t target_id,
    const fwk_id_t signal_id);

#endif /* FWK_SIGNAL_H */
