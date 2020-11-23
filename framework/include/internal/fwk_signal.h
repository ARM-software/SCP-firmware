/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_INTERNAL_SIGNAL_H
#define FWK_INTERNAL_SIGNAL_H

#include <fwk_id.h>
#include <fwk_signal.h>

/*
 * Framework signal support
 *
 * Signals always take precedence over events and will be processed before
 * any events on the event queues are processed.
 */
struct signal {
    FWK_RW fwk_id_t source_id; /* Module or element source of signal */
    FWK_RW fwk_id_t target_id; /* Module or element target for signal */
    FWK_RW fwk_id_t signal_id; /* Signal ID */
};

struct __fwk_signal_ctx {
    /* number of pending signals */
    int pending_signals;

#ifndef BUILD_HAS_MULTITHREADING
    /* signal we are currently handling */
    struct signal current_signal;
#endif

    /* array of pending signals */
    struct signal signals[FWK_MODULE_SIGNAL_COUNT];
};

#endif /* FWK_INTERNAL_SIGNAL_H */
