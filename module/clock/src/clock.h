/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLOCK_H
#define CLOCK_H

#include <mod_clock.h>

#include <fwk_id.h>

/*
 * Clock event indexes.
 */
enum clock_event_idx {
    CLOCK_EVENT_IDX_REQUEST = MOD_CLOCK_EVENT_IDX_REQUEST,
    CLOCK_EVENT_IDX_RESPONSE,
    CLOCK_EVENT_IDX_COUNT
};

/*
 * Event identifiers.
 */

/* Identifier of the response event */
static const fwk_id_t mod_clock_event_id_response =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_EVENT_IDX_RESPONSE);

#endif /* CLOCK_H */
