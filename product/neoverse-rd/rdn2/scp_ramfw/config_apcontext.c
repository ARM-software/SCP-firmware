/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "scp_software_mmap.h"

#include <mod_apcontext.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/*
 * AP Context module configuration
 */
static const struct mod_apcontext_config apcontext_data = {
    .base = SCP_AP_CONTEXT_BASE,
    .size = SCP_AP_CONTEXT_SIZE,
    .clock_id = FWK_ID_NONE_INIT,
};

struct fwk_module_config config_apcontext = {
    .data = &apcontext_data,
};
