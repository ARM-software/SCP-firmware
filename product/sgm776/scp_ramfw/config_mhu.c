/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sgm776_irq.h"
#include "sgm776_mhu.h"
#include "sgm776_mmap.h"

#include <mod_mhu2.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

static const struct fwk_element mhu_element_table[] = {
    [SGM776_MHU_DEVICE_IDX_S] = {
        .name = "MHU_S",
        .sub_element_count = 1,
        .data = &((struct mod_mhu2_channel_config) {
            .irq = MHU_SECURE_IRQ,
            .recv = MHU_RECV_S_BASE,
            .send = MHU_SEND_S_BASE,
            .channel = 0,
        })
    },
    [SGM776_MHU_DEVICE_IDX_NS_H] = {
        .name = "MHU_NS_HIGH",
        .sub_element_count = 1,
        .data = &((struct mod_mhu2_channel_config) {
            .irq = MHU_HIGH_PRIO_IRQ,
            .recv = MHU_RECV_NS_BASE,
            .send = MHU_SEND_NS_BASE,
            .channel = 0,
        })
    },
    [SGM776_MHU_DEVICE_IDX_NS_L] = {
        .name = "MHU_NS_LOW",
        .sub_element_count = 1,
        .data = &((struct mod_mhu2_channel_config) {
            .irq = MHU_LOW_PRIO_IRQ,
            .recv = MHU_RECV_NS_BASE,
            .send = MHU_SEND_NS_BASE,
            .channel = 1,
        })
    },
    [SGM776_MHU_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mhu_get_element_table(fwk_id_t module_id)
{
    return mhu_element_table;
}

struct fwk_module_config config_mhu2 = {
    .get_element_table = mhu_get_element_table,
};
