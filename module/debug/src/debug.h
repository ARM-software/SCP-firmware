/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <mod_debug.h>

#include <fwk_id.h>

#include <stdint.h>

enum debug_state {
    DEBUG_IDLE,
    DEBUG_GET,
    DEBUG_SET,
};

/*
 * Debug element context
 */
struct debug_dev_ctx {
    struct mod_debug_dev_config *config;
    struct mod_debug_driver_api *driver_api;
    uint32_t debug_users_mask;
    enum scp_debug_user requester;
    enum debug_state state;
    uint32_t cookie;
};

/*
 * Request event parameters
 */
struct mod_debug_request_params {
    enum scp_debug_user user_id;
    bool enable;
};

/*
 * Debug events indices
 */
enum mod_debug_event_idx {
    MOD_DEBUG_EVENT_IDX_REQ_DRV = MOD_DEBUG_PUBLIC_EVENT_IDX_COUNT,
    MOD_DEBUG_EVENT_IDX_SET_COMPLETE,
    MOD_DEBUG_EVENT_IDX_GET_COMPLETE,
    MOD_DEBUG_EVENT_COUNT,
};

/*
 * Debug event identifiers
 */
static const fwk_id_t mod_debug_event_id_req_drv =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_DEBUG, MOD_DEBUG_EVENT_IDX_REQ_DRV);

static const fwk_id_t mod_debug_event_id_set_complete =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_DEBUG, MOD_DEBUG_EVENT_IDX_SET_COMPLETE);

static const fwk_id_t mod_debug_event_id_get_complete =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_DEBUG, MOD_DEBUG_EVENT_IDX_GET_COMPLETE);

#endif /* DEBUG_H */
