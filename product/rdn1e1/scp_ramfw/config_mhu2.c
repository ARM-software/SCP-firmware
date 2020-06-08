/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_rdn1e1_mhu.h"
#include "scp_rdn1e1_mmap.h"

#include <mod_mhu2.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <fmw_cmsis.h>

static const struct fwk_element mhu_element_table[] = {
    [SCP_RDN1E1_MHU_DEVICE_IDX_SCP_AP_S_CLUS0] = {
        .name = "MHU_SCP_AP_S",
        .sub_element_count = 1,
        .data = &((struct mod_mhu2_channel_config) {
            .irq = MHU_AP_SEC_IRQ,
            .recv = SCP_MHU_SCP_AP_RCV_S_CLUS0,
            .send = SCP_MHU_SCP_AP_SND_S_CLUS0,
            .channel = 0,
        })
    },
    [SCP_RDN1E1_MHU_DEVICE_IDX_SCP_AP_NS_CLUS0] = {
        .name = "MHU_SCP_AP_NS",
        .sub_element_count = 1,
        .data = &((struct mod_mhu2_channel_config) {
            .irq = MHU_AP_NONSEC_IRQ,
            .recv = SCP_MHU_SCP_AP_RCV_NS_CLUS0,
            .send = SCP_MHU_SCP_AP_SND_NS_CLUS0,
            .channel = 0,
        })
    },
    [SCP_RDN1E1_MHU_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mhu_get_element_table(fwk_id_t module_id)
{
    return mhu_element_table;
}

const struct fwk_module_config config_mhu2 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mhu_get_element_table),
};
