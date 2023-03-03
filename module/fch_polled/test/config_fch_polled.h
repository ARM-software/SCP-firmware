/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <Mockfwk_module.h>

#include <mod_fch_polled.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>

#define FCH_MIN_POLL_RATE_US 4000

#define SCP_SCMI_FAST_CHANNEL_BASE   0xA0000000
#define SCP_SYSTEM_ACCESS_PORT1_BASE 0xB0000000

#define FAKE_RATE_LIMIT 0x0FFF

enum fake_fch_polled {
    FAKE_FCH_POLLED_0,
    FAKE_FCH_POLLED_1,
    FAKE_FCH_POLLED_COUNT,
};

#define FCH_ADDR_INIT(scp_addr, ap_addr) \
    &((struct mod_fch_polled_channel_config){ \
        .fch_addr = { \
            .local_view_address = scp_addr, \
            .target_view_address = ap_addr, \
        } })

static struct mod_fch_polled_config fake_fch_config = {
    .fch_alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0, 0),
    .fch_poll_rate = FCH_MIN_POLL_RATE_US,
    .rate_limit = FAKE_RATE_LIMIT,
};

static struct mod_fch_polled_config fake_fch_config_2 = {
    .fch_alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0, 0),
    .fch_poll_rate = (FCH_MIN_POLL_RATE_US - 1),
    .rate_limit = FAKE_RATE_LIMIT,
};

/*!
 *\brief Fast Channels in shared memory. Similar to mod_scmi_perf
 */
struct mod_scmi_perf_fast_channel_limit {
    /*! Performance limit max. */
    uint32_t range_max;
    /*! Performance limit min. */
    uint32_t range_min;
};

/*!
 *\brief Fast channels memory offset
 */
enum mod_scmi_perf_fast_channel_memory_offset {
    MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET = 0,
    MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET = sizeof(uint32_t),
    MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET =
        sizeof(uint32_t) + sizeof(struct mod_scmi_perf_fast_channel_limit),
    MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET =
        sizeof(uint32_t) * 2 + sizeof(struct mod_scmi_perf_fast_channel_limit),
    MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL = sizeof(uint32_t) * 2 +
        sizeof(struct mod_scmi_perf_fast_channel_limit) * 2
};

#define FCH_ADDRESS_LEVEL_SET(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FCH_ADDRESS_LIMIT_SET(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FCH_ADDRESS_LEVEL_SET_AP(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - \
     SCP_SYSTEM_ACCESS_PORT1_BASE)

#define FCH_ADDRESS_LIMIT_SET_AP(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - \
     SCP_SYSTEM_ACCESS_PORT1_BASE)

static const struct fwk_element fake_fch_polled_element_table[] = {
    [0] = {
        .name = "FCH_0_LEVEL_SET",
        .data = FCH_ADDR_INIT(
            FCH_ADDRESS_LEVEL_SET(0),
            FCH_ADDRESS_LEVEL_SET_AP(0)
            )
    },
    [1] = {
        .name = "FCH_0_LIMIT_SET",
        .data =FCH_ADDR_INIT(
            FCH_ADDRESS_LIMIT_SET(0),
            FCH_ADDRESS_LIMIT_SET_AP(0)
            ),
    },

};

static const struct fwk_element *fch_polled_get_element_table(
    fwk_id_t module_id)
{
    return fake_fch_polled_element_table;
}

struct fwk_module_config config_fake_fch_polled = {
    .data = &fake_fch_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(fch_polled_get_element_table),
};

static fwk_id_t fake_fch_0 =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_FCH_POLLED, FAKE_FCH_POLLED_0);

static const fwk_id_t fake_fch_1 =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_FCH_POLLED, FAKE_FCH_POLLED_1);
