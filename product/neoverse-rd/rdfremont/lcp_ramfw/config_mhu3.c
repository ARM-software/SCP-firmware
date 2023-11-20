/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
    MHU3_SCP2LCP_IRQ_COMBINED = 40,
    MHU3_AP2LCP_IRQ_PBX_COMBINED = 48,
    MHU3_AP2LCP_IRQ_STD_CH_TRANSFER = 52,
    MHU3_AP2LCP_IRQ_FAST_CH_TRANSFER_0 = 56,
    MHU3_AP2LCP_IRQ_FAST_CH_TRANSFER_1 = 57,
    MHU3_AP2LCP_IRQ_FAST_CH_TRANSFER_2 = 58,
};

#define LCP_AP_NUM_CHANNELS  1
#define LCP_SCP_NUM_CHANNELS 1

struct mod_mhu3_channel_config ap_lcp_channel_config[LCP_AP_NUM_CHANNELS] = {
    /* PBX CH 0, FLAG 0, MBX CH 0, FLAG 0 */
    MOD_MHU3_INIT_DBCH(0, 0, 0, 0),
};

struct mod_mhu3_channel_config scp_lcp_channel_config[LCP_SCP_NUM_CHANNELS] = {
    /* PBX CH 0, FLAG 0, MBX CH 0, FLAG 0 */
    MOD_MHU3_INIT_DBCH(0, 0, 0, 0),
};

static const struct fwk_element element_table[MHU3_DEVICE_IDX_COUNT+1] = {
    [MHU3_DEVICE_IDX_SCP_LCP] = {
        .name = "",
        .sub_element_count = LCP_SCP_NUM_CHANNELS,
        .data = &(struct mod_mhu3_device_config) {
            .irq = (unsigned int) MHU3_SCP2LCP_IRQ_COMBINED,
            .in = LCP0_SCP_MHU_MBX_BASE,
            .out = LCP0_SCP_MHU_PBX_BASE,
            .channels = &scp_lcp_channel_config[0],
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
