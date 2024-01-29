/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'mhu3'.
 */
#include <lcp_css_mmap.h>
#include <lcp_mhu3.h>

#include <mod_mhu3.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <fmw_cmsis.h>

#include <stddef.h>

enum mhu3_device2lcp_irq {
    MHU3_AP2LCP_IRQ_PBX_COMBINED = 48,
};

struct mod_mhu3_channel_config lcp_ap_channel_config[] = {
    /* FCH_IDX 0, FCH_GROUP_NUM 0, FCH_DIRECTION in */
    MOD_MHU3_INIT_FCH(0, 0, MOD_MHU3_FCH_DIR_IN),
};

static const struct fwk_element element_table[] = {
    [MHU3_DEVICE_IDX_LCP_AP_FCH_DVFS_SET_LVL] = {
        .name = "AP-LCP FCH DVFS SET LEVEL",
        .sub_element_count = 1,
        .data = &(struct mod_mhu3_device_config) {
            .irq = (unsigned int) MHU3_AP2LCP_IRQ_PBX_COMBINED,
            .in = LCP_AP_MHU_MBX_BASE,
            .channels = &lcp_ap_channel_config[0],
        },
    },
    [MHU3_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_mhu3 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};
