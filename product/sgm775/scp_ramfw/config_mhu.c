/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sgm775_irq.h"
#include "sgm775_mhu.h"
#include "sgm775_mmap.h"

#include <mod_mhu.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

static const struct fwk_element mhu_element_table[] = {
    [SGM775_MHU_DEVICE_IDX_S] = {
        .name = "MHU_S",
        .sub_element_count = 1,
        .data = &((struct mod_mhu_device_config) {
            .irq = MHU_SECURE_IRQ,
            .in = MHU_CPU_INTR_S_BASE,
            .out = MHU_SCP_INTR_S_BASE,
        })
    },
    [SGM775_MHU_DEVICE_IDX_NS_H] = {
        .name = "MHU_NS_H",
        .sub_element_count = 1,
        .data = &((struct mod_mhu_device_config) {
            .irq = MHU_HIGH_PRIO_IRQ,
            .in = MHU_CPU_INTR_H_BASE,
            .out = MHU_SCP_INTR_H_BASE,
        })
    },
    [SGM775_MHU_DEVICE_IDX_NS_L] = {
        .name = "MHU_NS_L",
        .sub_element_count = 1,
        .data = &((struct mod_mhu_device_config) {
            .irq = MHU_LOW_PRIO_IRQ,
            .in = MHU_CPU_INTR_L_BASE,
            .out = MHU_SCP_INTR_L_BASE,
        })
    },
    [SGM775_MHU_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mhu_get_element_table(fwk_id_t module_id)
{
    return mhu_element_table;
}

struct fwk_module_config config_mhu = {
    .get_element_table = mhu_get_element_table,
};
