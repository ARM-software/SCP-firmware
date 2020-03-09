/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sgm776_pik.h"
#include "sgm776_pik_cpu.h"
#include "sgm776_pik_dpu.h"
#include "sgm776_pik_gpu.h"
#include "sgm776_pik_system.h"
#include "sgm776_pik_vpu.h"
#include "system_clock.h"

#include <mod_pik_clock.h>
#include <mod_sid.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

/*
 * Rate lookup tables
 */

static struct mod_pik_clock_rate rate_table_cpu_a55[] = {
    {
        .rate = 2200 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static struct mod_pik_clock_rate rate_table_cpu_a75[] = {
    {
        .rate = 2700 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL1,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static struct mod_pik_clock_rate rate_table_gpu[] = {
    {
        .rate = 800 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via GPU PLL */
    },
};

static struct mod_pik_clock_rate rate_table_vpu[] = {
    {
        .rate = 650 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_VPUCLK_SOURCE_VIDEOPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via VPU PLL */
    },
};

static struct mod_pik_clock_rate rate_table_dpu[] = {
    {
        .rate = 600 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_ACLKDPU_SOURCE_DISPLAYPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via display PLL */
    },
};

static const struct mod_pik_clock_rate rate_table_sys_nocmemclk[] = {
    {
        .rate = 720 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (720 * FWK_MHZ),
    },
};

static const struct fwk_element pik_clock_element_table[] = {
    /*
     * Cluster 0 CPUS
     */
    {
        .name = "CLUS0_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->CORECLK[0].CTRL,
            .divext_reg = &PIK_CLUS0->CORECLK[0].DIV,
            .modulator_reg = &PIK_CLUS0->CORECLK[0].MOD,
            .initial_rate = 2200 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "CLUS0_CPU1",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->CORECLK[1].CTRL,
            .divext_reg = &PIK_CLUS0->CORECLK[1].DIV,
            .modulator_reg = &PIK_CLUS0->CORECLK[1].MOD,
            .initial_rate = 2200 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "CLUS0_CPU2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->CORECLK[2].CTRL,
            .divext_reg = &PIK_CLUS0->CORECLK[2].DIV,
            .modulator_reg = &PIK_CLUS0->CORECLK[2].MOD,
            .initial_rate = 2200 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "CLUS0_CPU3",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->CORECLK[3].CTRL,
            .divext_reg = &PIK_CLUS0->CORECLK[3].DIV,
            .modulator_reg = &PIK_CLUS0->CORECLK[3].MOD,
            .initial_rate = 2200 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "CLUS0_CPU4",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->CORECLK[4].CTRL,
            .divext_reg = &PIK_CLUS0->CORECLK[4].DIV,
            .modulator_reg = &PIK_CLUS0->CORECLK[4].MOD,
            .initial_rate = 2700 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "CLUS0_CPU5",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->CORECLK[5].CTRL,
            .divext_reg = &PIK_CLUS0->CORECLK[5].DIV,
            .modulator_reg = &PIK_CLUS0->CORECLK[5].MOD,
            .initial_rate = 2700 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "CLUS0_CPU6",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->CORECLK[6].CTRL,
            .divext_reg = &PIK_CLUS0->CORECLK[6].DIV,
            .modulator_reg = &PIK_CLUS0->CORECLK[6].MOD,
            .initial_rate = 2700 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "CLUS0_CPU7",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->CORECLK[7].CTRL,
            .divext_reg = &PIK_CLUS0->CORECLK[7].DIV,
            .modulator_reg = &PIK_CLUS0->CORECLK[7].MOD,
            .initial_rate = 2700 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    /*
     * GPU
     */
    {
        .name = "GPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = true,
            .control_reg = &PIK_GPU->GPUCLK_CTRL,
            .divsys_reg = &PIK_GPU->GPUCLK_DIV1,
            .divext_reg = &PIK_GPU->GPUCLK_DIV2,
            .rate_table = rate_table_gpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_gpu),
            .initial_rate = 800 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    /*
     * VPU
     */
    {
        .name = "VPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = true,
            .control_reg = &PIK_VPU->VIDEOCLK_CTRL,
            .divsys_reg = &PIK_VPU->VIDEOCLK_DIV1,
            .divext_reg = &PIK_VPU->VIDEOCLK_DIV2,
            .rate_table = rate_table_vpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_vpu),
            .initial_rate = 650 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    /*
     * DPU
     */
    {
        .name = "DPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = true,
            .control_reg = &PIK_DPU->ACLKDP_CTRL,
            .divsys_reg = &PIK_DPU->ACLKDP_DIV1,
            .divext_reg = &PIK_DPU->ACLKDP_DIV2,
            .rate_table = rate_table_dpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_dpu),
            .initial_rate = 600 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    /*
     * Arachne NOC
     */
    {
        .name = "INTERCONNECT",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->NOCMEMCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->NOCMEMCLK_DIV1,
            .rate_table = rate_table_sys_nocmemclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_nocmemclk),
            .initial_rate = 720 * FWK_MHZ,
            .defer_initialization = false,
        }),
    },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *pik_clock_get_element_table
    (fwk_id_t module_id)
{
    int status;
    int element_idx;
    struct mod_pik_clock_dev_config *config;
    const struct mod_sid_info *system_info;

    status = mod_sid_get_system_info(&system_info);
    fwk_assert(status == FWK_SUCCESS);

    switch (system_info->config_number) {
    case 1:
    case 5:
    case 7:
    case 8:
        /* CPUs 0-5: Little */
        for (element_idx = 0; element_idx < 6; element_idx++) {
            config = (struct mod_pik_clock_dev_config *)
                pik_clock_element_table[element_idx].data;
            config->rate_table = rate_table_cpu_a55;
            config->rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a55);
        }
        /* CPUs 6-7: Big */
        for (element_idx = 6; element_idx < 8; element_idx++) {
            config = (struct mod_pik_clock_dev_config *)
                pik_clock_element_table[element_idx].data;
            config->rate_table = rate_table_cpu_a75;
            config->rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a75);
        }
        break;
    case 2:
    case 3:
    case 4:
    case 6:
        /* CPUs 0-3: Little */
        for (element_idx = 0; element_idx < 4; element_idx++) {
            config = (struct mod_pik_clock_dev_config *)
                pik_clock_element_table[element_idx].data;
            config->rate_table = rate_table_cpu_a55;
            config->rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a55);
        }
        /* CPUs 4-7: Big */
        for (element_idx = 4; element_idx < 8; element_idx++) {
            config = (struct mod_pik_clock_dev_config *)
                pik_clock_element_table[element_idx].data;
            config->rate_table = rate_table_cpu_a75;
            config->rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a75);
        }
        break;
    default:
        return NULL;
    }

    return pik_clock_element_table;
}

const struct fwk_module_config config_pik_clock = {
    .get_element_table = pik_clock_get_element_table,
};
