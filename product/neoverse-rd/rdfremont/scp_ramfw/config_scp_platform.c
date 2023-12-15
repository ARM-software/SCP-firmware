/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'scp_platform'.
 */

#include "scp_cfgd_transport.h"

#include <mod_scp_platform.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define RSS_SYNC_WAIT_TIMEOUT_US (800 * 1000)

static const struct mod_scp_platform_config platform_config_data = {
    .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
    .rss_sync_wait_us = RSS_SYNC_WAIT_TIMEOUT_US,
    .transport_id = FWK_ID_ELEMENT_INIT(
        FWK_MODULE_IDX_TRANSPORT,
        SCP_CFGD_MOD_TRANSPORT_EIDX_SYSTEM),
};

struct fwk_module_config config_scp_platform = {
    .data = &platform_config_data,
};
