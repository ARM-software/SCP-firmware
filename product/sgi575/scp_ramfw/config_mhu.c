/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_sgi575_irq.h"
#include "scp_sgi575_mhu.h"
#include "scp_sgi575_mmap.h"

#include <mod_mhu.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

static const struct fwk_element mhu_element_table[] = {
    [SCP_SGI575_MHU_DEVICE_IDX_SCP_AP_S] = {
        .name = "MHU_SCP_AP_S",
        .sub_element_count = 1,
        .data = &((struct mod_mhu_device_config) {
            .irq = MHU_AP_SEC_IRQ,
            .in = SCP_MHU_AP_SCP_S(0),
            .out = SCP_MHU_SCP_AP_S(0),
        })
    },
    [SCP_SGI575_MHU_DEVICE_IDX_SCP_AP_NS] = {
        .name = "MHU_SCP_AP_NS",
        .sub_element_count = 1,
        .data = &((struct mod_mhu_device_config) {
            .irq = MHU_AP_NONSEC_IRQ,
            .in = SCP_MHU_AP_SCP_NS(0),
            .out = SCP_MHU_SCP_AP_NS(0),
        })
    },
    [SCP_SGI575_MHU_DEVICE_IDX_COUNT] = { 0 },
};


static const struct fwk_element *mhu_get_element_table(fwk_id_t module_id)
{
    return mhu_element_table;
}

const struct fwk_module_config config_mhu = {
    .get_element_table = mhu_get_element_table,
};
