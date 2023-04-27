/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_mmap.h"
#include "scp_tc_mhu.h"

#include <mod_mhu2.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <fmw_cmsis.h>

static const struct fwk_element mhu_element_table[] = {
    [0] = {
        .name = "MHU_SCP_RSS_S",
        .sub_element_count = 1,
        .data = &((
            struct
            mod_mhu2_channel_config){
            .irq = 121,
            .recv = SCP_MHU_SCP_RSS_RCV_S_CLUS0,
            .send = SCP_MHU_SCP_RSS_SND_S_CLUS0,
            .channel = 0,
        }),
    },
    [1] = { 0 },
};

static const struct fwk_element *mhu_get_element_table(fwk_id_t module_id)
{
    return mhu_element_table;
}

const struct fwk_module_config config_mhu2 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mhu_get_element_table),
};
