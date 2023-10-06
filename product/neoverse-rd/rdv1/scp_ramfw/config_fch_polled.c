/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_dvfs.h"
#include "platform_scmi.h"
#include "rd_alarm_idx.h"
#include "scp_software_mmap.h"

#include <mod_fch_polled.h>
#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

#define FC_LEVEL_SET_ADDR(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LIMIT_SET_ADDR(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LEVEL_GET_ADDR(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LIMIT_GET_ADDR(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LEVEL_SET_AP_ADDR(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - \
     SCP_SYSTEM_ACCESS_PORT1_BASE)

#define FC_LIMIT_SET_AP_ADDR(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - \
     SCP_SYSTEM_ACCESS_PORT1_BASE)

#define FC_LEVEL_GET_AP_ADDR(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - \
     SCP_SYSTEM_ACCESS_PORT1_BASE)

#define FC_LIMIT_GET_AP_ADDR(PERF_IDX) \
    (SCP_SCMI_FAST_CHANNEL_BASE + \
     MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - \
     SCP_SYSTEM_ACCESS_PORT1_BASE)

static struct mod_fch_polled_config module_config = {
    .fch_alarm_id = FWK_ID_SUB_ELEMENT_INIT(
        FWK_MODULE_IDX_TIMER,
        0,
        RD_SCMI_FAST_CHANNEL_IDX),
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
    [RDV1_PLAT_FCH_CPU0_LEVEL_SET] = { .name = "FCH_RDV1_CPU0_LEVEL_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU0),
                                           FC_LEVEL_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU0),
                                           FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU0_LIMIT_SET] = { .name = "FCH_RDV1_CPU0_LIMIT_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU0),
                                           FC_LIMIT_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU0),
                                           FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU0_LEVEL_GET] = { .name = "FCH_RDV1_CPU0_LEVEL_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU0),
                                           FC_LEVEL_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU0),
                                           FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU0_LIMIT_GET] = { .name = "FCH_RDV1_CPU0_LIMIT_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU0),
                                           FC_LIMIT_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU0),
                                           FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU1_LEVEL_SET] = { .name = "FCH_RDV1_CPU1_LEVEL_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU1),
                                           FC_LEVEL_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU1),
                                           FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU1_LIMIT_SET] = { .name = "FCH_RDV1_CPU1_LIMIT_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU1),
                                           FC_LIMIT_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU1),
                                           FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU1_LEVEL_GET] = { .name = "FCH_RDV1_CPU1_LEVEL_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU1),
                                           FC_LEVEL_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU1),
                                           FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU1_LIMIT_GET] = { .name = "FCH_RDV1_CPU1_LIMIT_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU1),
                                           FC_LIMIT_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU1),
                                           FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU2_LEVEL_SET] = { .name = "FCH_RDV1_CPU2_LEVEL_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU2),
                                           FC_LEVEL_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU2),
                                           FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU2_LIMIT_SET] = { .name = "FCH_RDV1_CPU2_LIMIT_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU2),
                                           FC_LIMIT_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU2),
                                           FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU2_LEVEL_GET] = { .name = "FCH_RDV1_CPU2_LEVEL_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU2),
                                           FC_LEVEL_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU2),
                                           FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU2_LIMIT_GET] = { .name = "FCH_RDV1_CPU2_LIMIT_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU2),
                                           FC_LIMIT_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU2),
                                           FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU3_LEVEL_SET] = { .name = "FCH_RDV1_CPU3_LEVEL_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU3),
                                           FC_LEVEL_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU3),
                                           FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU3_LIMIT_SET] = { .name = "FCH_RDV1_CPU3_LIMIT_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU3),
                                           FC_LIMIT_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU3),
                                           FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU3_LEVEL_GET] = { .name = "FCH_RDV1_CPU3_LEVEL_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU3),
                                           FC_LEVEL_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU3),
                                           FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU3_LIMIT_GET] = { .name = "FCH_RDV1_CPU3_LIMIT_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU3),
                                           FC_LIMIT_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU3),
                                           FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU4_LEVEL_SET] = { .name = "FCH_RDV1_CPU4_LEVEL_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU4),
                                           FC_LEVEL_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU4),
                                           FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU4_LIMIT_SET] = { .name = "FCH_RDV1_CPU4_LIMIT_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU4),
                                           FC_LIMIT_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU4),
                                           FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU4_LEVEL_GET] = { .name = "FCH_RDV1_CPU4_LEVEL_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU4),
                                           FC_LEVEL_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU4),
                                           FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU4_LIMIT_GET] = { .name = "FCH_RDV1_CPU4_LIMIT_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU4),
                                           FC_LIMIT_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU4),
                                           FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU5_LEVEL_SET] = { .name = "FCH_RDV1_CPU5_LEVEL_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU5),
                                           FC_LEVEL_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU5),
                                           FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU5_LIMIT_SET] = { .name = "FCH_RDV1_CPU5_LIMIT_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU5),
                                           FC_LIMIT_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU5),
                                           FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU5_LEVEL_GET] = { .name = "FCH_RDV1_CPU5_LEVEL_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU5),
                                           FC_LEVEL_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU5),
                                           FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU5_LIMIT_GET] = { .name = "FCH_RDV1_CPU5_LIMIT_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU5),
                                           FC_LIMIT_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU5),
                                           FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU6_LEVEL_SET] = { .name = "FCH_RDV1_CPU6_LEVEL_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU6),
                                           FC_LEVEL_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU6),
                                           FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU6_LIMIT_SET] = { .name = "FCH_RDV1_CPU6_LIMIT_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU6),
                                           FC_LIMIT_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU6),
                                           FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU6_LEVEL_GET] = { .name = "FCH_RDV1_CPU6_LEVEL_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU6),
                                           FC_LEVEL_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU6),
                                           FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU6_LIMIT_GET] = { .name = "FCH_RDV1_CPU6_LIMIT_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU6),
                                           FC_LIMIT_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU6),
                                           FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU7_LEVEL_SET] = { .name = "FCH_RDV1_CPU7_LEVEL_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU7),
                                           FC_LEVEL_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU7),
                                           FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU7_LIMIT_SET] = { .name = "FCH_RDV1_CPU7_LIMIT_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU7),
                                           FC_LIMIT_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU7),
                                           FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU7_LEVEL_GET] = { .name = "FCH_RDV1_CPU7_LEVEL_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU7),
                                           FC_LEVEL_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU7),
                                           FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU7_LIMIT_GET] = { .name = "FCH_RDV1_CPU7_LIMIT_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU7),
                                           FC_LIMIT_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU7),
                                           FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU8_LEVEL_SET] = { .name = "FCH_RDV1_CPU8_LEVEL_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU8),
                                           FC_LEVEL_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU8),
                                           FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU8_LIMIT_SET] = { .name = "FCH_RDV1_CPU8_LIMIT_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU8),
                                           FC_LIMIT_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU8),
                                           FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU8_LEVEL_GET] = { .name = "FCH_RDV1_CPU8_LEVEL_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU8),
                                           FC_LEVEL_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU8),
                                           FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU8_LIMIT_GET] = { .name = "FCH_RDV1_CPU8_LIMIT_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU8),
                                           FC_LIMIT_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU8),
                                           FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU9_LEVEL_SET] = { .name = "FCH_RDV1_CPU9_LEVEL_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU9),
                                           FC_LEVEL_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU9),
                                           FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU9_LIMIT_SET] = { .name = "FCH_RDV1_CPU9_LIMIT_SET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_SET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU9),
                                           FC_LIMIT_SET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU9),
                                           FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU9_LEVEL_GET] = { .name = "FCH_RDV1_CPU9_LEVEL_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LEVEL_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU9),
                                           FC_LEVEL_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU9),
                                           FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU9_LIMIT_GET] = { .name = "FCH_RDV1_CPU9_LIMIT_GET",
                                       .data = FCH_ADDR_INIT(
                                           FC_LIMIT_GET_ADDR(
                                               DVFS_ELEMENT_IDX_CPU9),
                                           FC_LIMIT_GET_AP_ADDR(
                                               DVFS_ELEMENT_IDX_CPU9),
                                           FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU10_LEVEL_SET] = { .name = "FCH_RDV1_CPU10_LEVEL_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU10),
                                            FC_LEVEL_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU10),
                                            FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU10_LIMIT_SET] = { .name = "FCH_RDV1_CPU10_LIMIT_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU10),
                                            FC_LIMIT_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU10),
                                            FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU10_LEVEL_GET] = { .name = "FCH_RDV1_CPU10_LEVEL_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU10),
                                            FC_LEVEL_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU10),
                                            FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU10_LIMIT_GET] = { .name = "FCH_RDV1_CPU10_LIMIT_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU10),
                                            FC_LIMIT_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU10),
                                            FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU11_LEVEL_SET] = { .name = "FCH_RDV1_CPU11_LEVEL_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU11),
                                            FC_LEVEL_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU11),
                                            FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU11_LIMIT_SET] = { .name = "FCH_RDV1_CPU11_LIMIT_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU11),
                                            FC_LIMIT_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU11),
                                            FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU11_LEVEL_GET] = { .name = "FCH_RDV1_CPU11_LEVEL_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU11),
                                            FC_LEVEL_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU11),
                                            FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU11_LIMIT_GET] = { .name = "FCH_RDV1_CPU11_LIMIT_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU11),
                                            FC_LIMIT_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU11),
                                            FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU12_LEVEL_SET] = { .name = "FCH_RDV1_CPU12_LEVEL_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU12),
                                            FC_LEVEL_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU12),
                                            FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU12_LIMIT_SET] = { .name = "FCH_RDV1_CPU12_LIMIT_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU12),
                                            FC_LIMIT_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU12),
                                            FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU12_LEVEL_GET] = { .name = "FCH_RDV1_CPU12_LEVEL_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU12),
                                            FC_LEVEL_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU12),
                                            FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU12_LIMIT_GET] = { .name = "FCH_RDV1_CPU12_LIMIT_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU12),
                                            FC_LIMIT_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU12),
                                            FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU13_LEVEL_SET] = { .name = "FCH_RDV1_CPU13_LEVEL_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU13),
                                            FC_LEVEL_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU13),
                                            FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU13_LIMIT_SET] = { .name = "FCH_RDV1_CPU13_LIMIT_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU13),
                                            FC_LIMIT_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU13),
                                            FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU13_LEVEL_GET] = { .name = "FCH_RDV1_CPU13_LEVEL_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU13),
                                            FC_LEVEL_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU13),
                                            FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU13_LIMIT_GET] = { .name = "FCH_RDV1_CPU13_LIMIT_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU13),
                                            FC_LIMIT_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU13),
                                            FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU14_LEVEL_SET] = { .name = "FCH_RDV1_CPU14_LEVEL_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU14),
                                            FC_LEVEL_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU14),
                                            FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU14_LIMIT_SET] = { .name = "FCH_RDV1_CPU14_LIMIT_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU14),
                                            FC_LIMIT_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU14),
                                            FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU14_LEVEL_GET] = { .name = "FCH_RDV1_CPU14_LEVEL_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU14),
                                            FC_LEVEL_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU14),
                                            FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU14_LIMIT_GET] = { .name = "FCH_RDV1_CPU14_LIMIT_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU14),
                                            FC_LIMIT_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU14),
                                            FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU15_LEVEL_SET] = { .name = "FCH_RDV1_CPU15_LEVEL_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU15),
                                            FC_LEVEL_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU15),
                                            FCH_POLLED_LEVEL_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU15_LIMIT_SET] = { .name = "FCH_RDV1_CPU15_LIMIT_SET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_SET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU15),
                                            FC_LIMIT_SET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU15),
                                            FCH_POLLED_LIMIT_SET_LENGTH) },
    [RDV1_PLAT_FCH_CPU15_LEVEL_GET] = { .name = "FCH_RDV1_CPU15_LEVEL_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LEVEL_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU15),
                                            FC_LEVEL_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU15),
                                            FCH_POLLED_LEVEL_GET_LENGTH) },
    [RDV1_PLAT_FCH_CPU15_LIMIT_GET] = { .name = "FCH_RDV1_CPU15_LIMIT_GET",
                                        .data = FCH_ADDR_INIT(
                                            FC_LIMIT_GET_ADDR(
                                                DVFS_ELEMENT_IDX_CPU15),
                                            FC_LIMIT_GET_AP_ADDR(
                                                DVFS_ELEMENT_IDX_CPU15),
                                            FCH_POLLED_LIMIT_GET_LENGTH) },
    [RDV1_PLAT_FCH_COUNT] = { 0 },
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
