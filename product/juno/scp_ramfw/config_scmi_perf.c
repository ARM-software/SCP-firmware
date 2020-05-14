/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_dvfs.h"
#include "juno_alarm_idx.h"
#include "juno_scmi.h"
#include "software_mmap.h"

#include <internal/scmi_perf.h>
#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

static const struct mod_scmi_perf_domain_config domains[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
        .permissions = &(const uint32_t[]) {
            [JUNO_SCMI_AGENT_IDX_OSPM] = MOD_SCMI_PERF_PERMS_SET_LEVEL  |
                                   MOD_SCMI_PERF_PERMS_SET_LIMITS,
            [JUNO_SCMI_AGENT_IDX_PSCI] = 0 /* No Access */,
        },
#ifdef BUILD_HAS_FAST_CHANNELS
        .fast_channels_addr_scp = SCMI_FAST_CHANNEL_BASE,
        .fast_channels_addr_ap = SCMI_FAST_CHANNEL_BASE - EXTERNAL_DEV_BASE,
#endif
    },
    [DVFS_ELEMENT_IDX_BIG] = {
        .permissions = &(const uint32_t[]) {
            [JUNO_SCMI_AGENT_IDX_OSPM] = MOD_SCMI_PERF_PERMS_SET_LEVEL  |
                                   MOD_SCMI_PERF_PERMS_SET_LIMITS,
            [JUNO_SCMI_AGENT_IDX_PSCI] = 0 /* No Access */,
        },
#ifdef BUILD_HAS_FAST_CHANNELS
        .fast_channels_addr_scp = SCMI_FAST_CHANNEL_BASE +
            sizeof(struct mod_scmi_perf_fast_channel),
        .fast_channels_addr_ap = SCMI_FAST_CHANNEL_BASE +
            sizeof(struct mod_scmi_perf_fast_channel) -
            EXTERNAL_DEV_BASE,
#endif
    },
    [DVFS_ELEMENT_IDX_GPU] = {
        .permissions = &(const uint32_t[]) {
            [JUNO_SCMI_AGENT_IDX_OSPM] = MOD_SCMI_PERF_PERMS_SET_LEVEL  |
                                   MOD_SCMI_PERF_PERMS_SET_LIMITS,
            [JUNO_SCMI_AGENT_IDX_PSCI] = 0 /* No Access */,
        },
#ifdef BUILD_HAS_FAST_CHANNELS
        .fast_channels_addr_scp = SCMI_FAST_CHANNEL_BASE +
            (sizeof(struct mod_scmi_perf_fast_channel) * 2),
        .fast_channels_addr_ap = SCMI_FAST_CHANNEL_BASE +
            (sizeof(struct mod_scmi_perf_fast_channel) * 2) -
            EXTERNAL_DEV_BASE,
#endif
    },
};


struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config) {
        .domains = &domains,
#ifdef BUILD_HAS_FAST_CHANNELS
        .fast_channels_alarm_id =
            FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
            JUNO_SCMI_FAST_CHANNEL_IDX),
        .fast_channels_rate_limit = SCMI_PERF_FC_MIN_RATE_LIMIT,
#else
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
#endif
    }),
};
