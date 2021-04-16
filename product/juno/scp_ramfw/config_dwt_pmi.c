/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "system_mmap.h"

#include <mod_dwt_pmi.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

struct mod_dwt_pmi_config arm_dwt_pmi_juno = {
    .debug_sys_demcr_addr = (uint32_t *)SCS_DEMCR_ADDR,
    .dwt_ctrl_addr = (uint32_t *)DWT_CTRL_ADDR,
    .dwt_cyccnt = (uint32_t *)DWT_CYCCNT,
    .hw_timer = (struct cntbase_reg *)REFCLK_CNTBASE0_BASE,
};

struct fwk_module_config config_dwt_pmi = {
    .elements = { 0 },
    .data = &arm_dwt_pmi_juno,
};
