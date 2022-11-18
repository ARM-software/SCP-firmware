/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <Mockfwk_module.h>

#include <internal/mhu3.h>

#include <mod_mhu3.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>

enum mhu3_fake_irqs {
    /* Some fake random device irq numbers */
    MHU3_FAKE_IRQ_COMBINED = 40,
    MHU3_FAKE_IRQ_PBX = 45,
    MHU3_FAKE_DEVICE_1_PBX = 48,
    MHU3_FAKE_DEVICE_1_MBX = 49,
    MHU3_FAKE_DEVICE_1_FAST_CH_TRANSFER_0 = 56,
    MHU3_FAKE_DEVICE_1_FAST_CH_TRANSFER_1 = 57,
};

/*
 * This represents index of the channel descriptor within
 * element table
 */
enum mhu3_fake_device_1_channel_idx {
    FAKE_DEVICE_1_CHANNEL_DBCH_0_IDX,
    FAKE_DEVICE_1_CHANNEL_FCH_0_IN_IDX,
    FAKE_DEVICE_1_CHANNEL_FCH_0_OUT_IDX,
    FAKE_DEVICE_1_CHANNEL_COUNT,
};

/* Doorbell channel number */
enum dbch_channels {
    FAKE_DEVICE_1_CHANNEL_DBCH_0,
};

/*
 * Fast channel number
 * separate fast channels with different directions
 * can have same sequence number
 */
enum fch_channels {
    FAKE_DEVICE_CHANNEL_FCH_0,
};

#define FAKE_DEVICE_1_NUM_CH FAKE_DEVICE_1_CHANNEL_COUNT

enum mhu3_fake_device_idx {
    MHU3_DEVICE_IDX_DEVICE_1,
    MHU3_DEVICE_IDX_COUNT,
};

struct mod_mhu3_channel_config device_1_channel_config[FAKE_DEVICE_1_NUM_CH] = {
    /* PBX CH 0, FLAG 0, MBX CH 0, FLAG 0 */
    MOD_MHU3_INIT_DBCH(0, 0, 0, 0),
    /* FCH 0, group 0, direction in (can changed for few test cases) */
    MOD_MHU3_INIT_FCH(0, 0, MOD_MHU3_FCH_DIR_IN),
    MOD_MHU3_INIT_FCH(0, 0, MOD_MHU3_FCH_DIR_OUT),
};

/* Provide a fake device info */
static const struct fwk_element element_table[MHU3_DEVICE_IDX_COUNT+1] = {
    [MHU3_DEVICE_IDX_DEVICE_1] = {
        .name = "",
        .sub_element_count = FAKE_DEVICE_1_NUM_CH,
        .data = &(struct mod_mhu3_device_config) {
            .irq = (unsigned int) MHU3_FAKE_IRQ_COMBINED,
            .in = (uintptr_t)NULL,
            .out = (uintptr_t)NULL,
            .channels = &device_1_channel_config[0],
        },
    },
    [MHU3_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_fake_mhu3 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};
