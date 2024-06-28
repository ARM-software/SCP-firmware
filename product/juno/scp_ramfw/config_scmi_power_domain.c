/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_power_domain.h>
#include <juno_scmi.h>

#include <mod_scmi_power_domain.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
struct mod_scmi_pd_agent_config agent_table[3] = {
    [JUNO_SCMI_AGENT_IDX_OSPM] = {
        .domain_count = 10,
        .domains = (uint32_t [10]) {
            [0] = POWER_DOMAIN_IDX_BIG_CPU0,
            [1] = POWER_DOMAIN_IDX_BIG_CPU1,
            [2] = POWER_DOMAIN_IDX_LITTLE_CPU0,
            [3] = POWER_DOMAIN_IDX_LITTLE_CPU1,
            [4] = POWER_DOMAIN_IDX_LITTLE_CPU2,
            [5] = POWER_DOMAIN_IDX_LITTLE_CPU3,
            [6] = POWER_DOMAIN_IDX_BIG_SSTOP,
            [7] = POWER_DOMAIN_IDX_LITTLE_SSTOP,
            [8] = POWER_DOMAIN_IDX_DBGSYS,
            [9] = POWER_DOMAIN_IDX_GPUTOP,
        },
    },

    [JUNO_SCMI_AGENT_IDX_PSCI] = {
        .domain_count = 10,
        .domains = (uint32_t [10]) {
            [0] = POWER_DOMAIN_IDX_BIG_CPU0,
            [1] = POWER_DOMAIN_IDX_BIG_CPU1,
            [2] = POWER_DOMAIN_IDX_LITTLE_CPU0,
            [3] = POWER_DOMAIN_IDX_LITTLE_CPU1,
            [4] = POWER_DOMAIN_IDX_LITTLE_CPU2,
            [5] = POWER_DOMAIN_IDX_LITTLE_CPU3,
            [6] = POWER_DOMAIN_IDX_BIG_SSTOP,
            [7] = POWER_DOMAIN_IDX_LITTLE_SSTOP,
            [8] = POWER_DOMAIN_IDX_DBGSYS,
            [9] = POWER_DOMAIN_IDX_GPUTOP,
        },
    },
};
#endif

struct fwk_module_config config_scmi_power_domain = {
    .data =
        &(struct mod_scmi_pd_config){
#ifdef BUILD_HAS_MOD_DEBUG
            .debug_pd_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_POWER_DOMAIN,
                POWER_DOMAIN_IDX_DBGSYS),
            .debug_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DEBUG, 0),
#endif
#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
            .agent_config_table = agent_table,
#endif
        },
};
