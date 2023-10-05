/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_dvfs.h"
#include "juno_alarm_idx.h"
#include "juno_scmi.h"
#include "scp_mmap.h"
#include "software_mmap.h"

#include <mod_fch_polled.h>
#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

#define FC_LEVEL_SET_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LIMIT_SET_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LEVEL_GET_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LIMIT_GET_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LEVEL_SET_AP_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - EXTERNAL_DEV_BASE)

#define FC_LIMIT_SET_AP_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - EXTERNAL_DEV_BASE)

#define FC_LEVEL_GET_AP_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - EXTERNAL_DEV_BASE)

#define FC_LIMIT_GET_AP_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - EXTERNAL_DEV_BASE)

static struct mod_fch_polled_config module_config = {
    .fch_alarm_id = FWK_ID_SUB_ELEMENT_INIT(
        FWK_MODULE_IDX_TIMER,
        0,
        JUNO_SCMI_FAST_CHANNEL_IDX),
    .fch_poll_rate = FCH_MIN_POLL_RATE_US,
    .rate_limit = (4 * 1000),
    .attributes = 0,
};

enum fch_polled_length {
    FCH_POLLED_LEVEL_SET_LENGTH = sizeof(uint32_t),
    FCH_POLLED_LIMIT_SET_LENGTH =
        sizeof(struct mod_scmi_perf_fast_channel_limit),
    FCH_POLLED_LEVEL_GET_LENGTH = sizeof(uint32_t),
    FCH_POLLED_LIMIT_GET_LENGTH =
        sizeof(struct mod_scmi_perf_fast_channel_limit)
};

#define FCH_ADDR_INIT(scp_addr, ap_addr, len) \
    &((struct mod_fch_polled_channel_config){ \
        .fch_addr = { \
            .local_view_address = scp_addr, \
            .target_view_address = ap_addr, \
            .length = len, \
        } })

static const struct fwk_element fch_polled_element_table[] = {
    [JUNO_PLAT_FCH_BIG_LEVEL_SET] = {
        .name = "FCH_BIG_LEVEL_SET",
        .data = FCH_ADDR_INIT(
            FC_LEVEL_SET_ADDR(DVFS_ELEMENT_IDX_BIG),
            FC_LEVEL_SET_AP_ADDR(DVFS_ELEMENT_IDX_BIG),
            (uint32_t)FCH_POLLED_LEVEL_SET_LENGTH
            )
    },
    [JUNO_PLAT_FCH_BIG_LIMIT_SET] = {
        .name = "FCH_BIG_LIMIT_SET",
        .data =FCH_ADDR_INIT(
            FC_LIMIT_SET_ADDR(DVFS_ELEMENT_IDX_BIG),
            FC_LIMIT_SET_AP_ADDR(DVFS_ELEMENT_IDX_BIG),
            (uint32_t)FCH_POLLED_LIMIT_SET_LENGTH
            ),
    },
    [JUNO_PLAT_FCH_BIG_LEVEL_GET] = {
        .name = "FCH_BIG_LEVEL_GET",
        .data = FCH_ADDR_INIT(
            FC_LEVEL_GET_ADDR(DVFS_ELEMENT_IDX_BIG),
            FC_LEVEL_GET_AP_ADDR(DVFS_ELEMENT_IDX_BIG),
            (uint32_t)FCH_POLLED_LEVEL_GET_LENGTH
            )
    },
    [JUNO_PLAT_FCH_BIG_LIMIT_GET] = {
        .name = "FCH_BIG_LIMIT_GET",
        .data = FCH_ADDR_INIT(
            FC_LIMIT_GET_ADDR(DVFS_ELEMENT_IDX_BIG),
            FC_LIMIT_GET_AP_ADDR(DVFS_ELEMENT_IDX_BIG),
            (uint32_t)FCH_POLLED_LIMIT_GET_LENGTH
            )
    },
    [JUNO_PLAT_FCH_LITTLE_LEVEL_SET] = {
        .name = "FCH_LITTLE_LEVEL_SET",
        .data = FCH_ADDR_INIT(
            FC_LEVEL_SET_ADDR(DVFS_ELEMENT_IDX_LITTLE),
            FC_LEVEL_SET_AP_ADDR(DVFS_ELEMENT_IDX_LITTLE),
            (uint32_t)FCH_POLLED_LEVEL_SET_LENGTH
            )
    },
    [JUNO_PLAT_FCH_LITTLE_LIMIT_SET] = {
        .name = "FCH_LITTLE_LIMIT_SET",
        .data =FCH_ADDR_INIT(
            FC_LIMIT_SET_ADDR(DVFS_ELEMENT_IDX_LITTLE),
            FC_LIMIT_SET_AP_ADDR(DVFS_ELEMENT_IDX_LITTLE),
            (uint32_t)FCH_POLLED_LIMIT_SET_LENGTH
            ),
    },
    [JUNO_PLAT_FCH_LITTLE_LEVEL_GET] = {
        .name = "FCH_LITTLE_LEVEL_GET",
        .data = FCH_ADDR_INIT(
            FC_LEVEL_GET_ADDR(DVFS_ELEMENT_IDX_LITTLE),
            FC_LEVEL_GET_AP_ADDR(DVFS_ELEMENT_IDX_LITTLE),
            (uint32_t)FCH_POLLED_LEVEL_GET_LENGTH
            )
    },
    [JUNO_PLAT_FCH_LITTLE_LIMIT_GET] = {
        .name = "FCH_LITTLE_LIMIT_GET",
        .data = FCH_ADDR_INIT(
            FC_LIMIT_GET_ADDR(DVFS_ELEMENT_IDX_LITTLE),
            FC_LIMIT_GET_AP_ADDR(DVFS_ELEMENT_IDX_LITTLE),
            (uint32_t)FCH_POLLED_LIMIT_GET_LENGTH
            )
    },
    [JUNO_PLAT_FCH_GPU_LEVEL_SET] = {
        .name = "FCH_GPU_LEVEL_SET",
        .data = FCH_ADDR_INIT(
            FC_LEVEL_SET_ADDR(DVFS_ELEMENT_IDX_GPU),
            FC_LEVEL_SET_AP_ADDR(DVFS_ELEMENT_IDX_GPU),
            (uint32_t)FCH_POLLED_LEVEL_SET_LENGTH
            )
    },
    [JUNO_PLAT_FCH_GPU_LIMIT_SET] = {
        .name = "FCH_GPU_LIMIT_SET",
        .data =FCH_ADDR_INIT(
            FC_LIMIT_SET_ADDR(DVFS_ELEMENT_IDX_GPU),
            FC_LIMIT_SET_AP_ADDR(DVFS_ELEMENT_IDX_GPU),
            (uint32_t)FCH_POLLED_LIMIT_SET_LENGTH
            ),
    },
    [JUNO_PLAT_FCH_GPU_LEVEL_GET] = {
        .name = "FCH_GPU_LEVEL_GET",
        .data = FCH_ADDR_INIT(
            FC_LEVEL_GET_ADDR(DVFS_ELEMENT_IDX_GPU),
            FC_LEVEL_GET_AP_ADDR(DVFS_ELEMENT_IDX_GPU),
            (uint32_t)FCH_POLLED_LEVEL_GET_LENGTH
            )
    },
    [JUNO_PLAT_FCH_GPU_LIMIT_GET] = {
        .name = "FCH_GPU_LIMIT_GET",
        .data = FCH_ADDR_INIT(
            FC_LIMIT_GET_ADDR(DVFS_ELEMENT_IDX_GPU),
            FC_LIMIT_GET_AP_ADDR(DVFS_ELEMENT_IDX_GPU),
            (uint32_t)FCH_POLLED_LIMIT_GET_LENGTH
            )
    },

    [JUNO_PLAT_FCH_COUNT] = {0},
};

static const struct fwk_element *fch_polled_get_element_table(
    fwk_id_t module_id)
{
    return fch_polled_element_table;
}

const struct fwk_module_config config_fch_polled = {
    .data = &module_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(fch_polled_get_element_table),
};
