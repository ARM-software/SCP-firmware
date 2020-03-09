/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_mhu.h"
#include "synquacer_irq.h"
#include "synquacer_mmap.h"

#include <mod_mhu.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

static const struct fwk_element mhu_element_table[] = {
    [SCP_SYNQUACER_MHU_DEVICE_IDX_SCP_AP_S] = {
        .name = "MHU_SCP_AP_S",
        .sub_element_count = 1,
        .data = &((struct mod_mhu_device_config) {
            .irq = MHU_SEC_SCP_AP_IRQn,
            .in = MHU_AP_TO_SCP_S(0),
            .out = MHU_SCP_TO_AP_S(0),
        }) },
    [SCP_SYNQUACER_MHU_DEVICE_IDX_SCP_AP_NS] = {
        .name = "MHU_SCP_AP_NS",
        .sub_element_count = 1,
        .data = &((struct mod_mhu_device_config) {
            .irq = MHU_NON_SEC_SCP_AP_IRQn,
            .in = MHU_AP_TO_SCP_NS(0),
            .out = MHU_SCP_TO_AP_NS(0),
        }) },
    [SCP_SYNQUACER_MHU_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mhu_get_element_table(fwk_id_t module_id)
{
    return mhu_element_table;
}

const struct fwk_module_config config_mhu = {
    .get_element_table = mhu_get_element_table,
};
