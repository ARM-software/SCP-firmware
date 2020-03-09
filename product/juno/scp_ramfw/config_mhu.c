/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_irq.h"
#include "juno_mhu.h"
#include "system_mmap.h"

#include <mod_mhu.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stddef.h>

static const struct fwk_element element_table[] = {
    [JUNO_MHU_DEVICE_IDX_S] = {
        .name = "",
        .sub_element_count = 1,
        .data = &(struct mod_mhu_device_config) {
            .irq = MHU_SECURE_IRQ,
            .in = MHU_CPU_INTR_S_BASE,
            .out = MHU_SCP_INTR_S_BASE,
        },
    },
    [JUNO_MHU_DEVICE_IDX_NS_H] = {
        .name = "",
        .sub_element_count = 1,
        .data = &(struct mod_mhu_device_config) {
            .irq = MHU_HIGH_PRIO_IRQ,
            .in = MHU_CPU_INTR_H_BASE,
            .out = MHU_SCP_INTR_H_BASE,
        },
    },
    [JUNO_MHU_DEVICE_IDX_NS_L] = {
        .name = "",
        .sub_element_count = 1,
        .data = &(struct mod_mhu_device_config) {
            .irq = MHU_LOW_PRIO_IRQ,
            .in = MHU_CPU_INTR_L_BASE,
            .out = MHU_SCP_INTR_L_BASE,
        },
    },
    [JUNO_MHU_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_mhu = {
    .get_element_table = get_element_table,
    .data = NULL,
};
