/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'apcontext'.
 */

#include "scp_clock.h"
#include "scp_fw_mmap.h"

#include <mod_apcontext.h>
#include <mod_scp_platform.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct mod_apcontext_config apcontext_data = {
    .base = SCP_AP_CONTEXT_BASE,
    .size = SCP_AP_CONTEXT_SIZE,
    .clock_id = FWK_ID_NONE_INIT,
    .platform_notification = {
        .notification_id = FWK_ID_NOTIFICATION_INIT(
            FWK_MODULE_IDX_SCP_PLATFORM,
            MOD_SCP_PLATFORM_NOTIFICATION_IDX_SUBSYS_INITIALIZED),
        .source_id = FWK_ID_MODULE_INIT(
            FWK_MODULE_IDX_SCP_PLATFORM),
    },
};

struct fwk_module_config config_apcontext = {
    .data = &apcontext_data,
};
