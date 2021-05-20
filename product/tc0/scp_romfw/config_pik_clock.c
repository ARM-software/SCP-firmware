/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "cpu_pik.h"
#include "system_pik.h"

#include <mod_pik_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
/*
 * Rate lookup tables
 */
static struct mod_pik_clock_rate rate_table_cpu_group_klein[] = {
    {
        .rate = 1537 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static const struct mod_pik_clock_rate rate_table_gicclk[] = {
    {
        .rate = 2000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (2000 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_pclkscp[] = {
    {
        .rate = 2000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (2000 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_sysperclk[] = {
    {
        .rate = 2000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (2000 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_uartclk[] = {
    {
        .rate = 2000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (2000 * FWK_MHZ),
    },
};

static const struct fwk_element
    pik_clock_element_table[] =
        {
            [CLOCK_PIK_IDX_CLUS0_CPU0] =
                {
                    .name = "CLUS0_CPU0",
                    .data = &((struct mod_pik_clock_dev_config){
                        .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
                        .is_group_member = true,
                        .control_reg = &CLUSTER_PIK_PTR(0)->CORECLK[0].CTRL,
                        .divext_reg = &CLUSTER_PIK_PTR(0)->CORECLK[0].DIV,
                        .modulator_reg = &CLUSTER_PIK_PTR(0)->CORECLK[0].MOD,
                        .rate_table = rate_table_cpu_group_klein,
                        .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_klein),
                    }),
                },
            [CLOCK_PIK_IDX_CLUS0_CPU1] =
                {
                    .name = "CLUS0_CPU1",
                    .data = &((struct mod_pik_clock_dev_config){
                        .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
                        .is_group_member = true,
                        .control_reg = &CLUSTER_PIK_PTR(0)->CORECLK[1].CTRL,
                        .divext_reg = &CLUSTER_PIK_PTR(0)->CORECLK[1].DIV,
                        .modulator_reg = &CLUSTER_PIK_PTR(0)->CORECLK[1].MOD,
                        .rate_table = rate_table_cpu_group_klein,
                        .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_klein),
                    }),
                },
            [CLOCK_PIK_IDX_CLUS0_CPU2] =
                {
                    .name = "CLUS0_CPU2",
                    .data = &((struct mod_pik_clock_dev_config){
                        .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
                        .is_group_member = true,
                        .control_reg = &CLUSTER_PIK_PTR(0)->CORECLK[2].CTRL,
                        .divext_reg = &CLUSTER_PIK_PTR(0)->CORECLK[2].DIV,
                        .modulator_reg = &CLUSTER_PIK_PTR(0)->CORECLK[2].MOD,
                        .rate_table = rate_table_cpu_group_klein,
                        .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_klein),
                    }),
                },
            [CLOCK_PIK_IDX_CLUS0_CPU3] =
                {
                    .name = "CLUS0_CPU3",
                    .data = &((struct mod_pik_clock_dev_config){
                        .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
                        .is_group_member = true,
                        .control_reg = &CLUSTER_PIK_PTR(0)->CORECLK[3].CTRL,
                        .divext_reg = &CLUSTER_PIK_PTR(0)->CORECLK[3].DIV,
                        .modulator_reg = &CLUSTER_PIK_PTR(0)->CORECLK[3].MOD,
                        .rate_table = rate_table_cpu_group_klein,
                        .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_klein),
                    }),
                },
            [CLOCK_PIK_IDX_GIC] =
                {
                    .name = "GIC",
                    .data = &((struct mod_pik_clock_dev_config){
                        .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
                        .is_group_member = false,
                        .control_reg = &SYSTEM_PIK_PTR->GICCLK_CTRL,
                        .divsys_reg = &SYSTEM_PIK_PTR->GICCLK_DIV1,
                        .rate_table = rate_table_gicclk,
                        .rate_count = FWK_ARRAY_SIZE(rate_table_gicclk),
                        .initial_rate = 2000 * FWK_MHZ,
                    }),
                },
            [CLOCK_PIK_IDX_PCLKSCP] =
                {
                    .name = "PCLKSCP",
                    .data = &((struct mod_pik_clock_dev_config){
                        .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
                        .is_group_member = false,
                        .control_reg = &SYSTEM_PIK_PTR->PCLKSCP_CTRL,
                        .divsys_reg = &SYSTEM_PIK_PTR->PCLKSCP_DIV1,
                        .rate_table = rate_table_pclkscp,
                        .rate_count = FWK_ARRAY_SIZE(rate_table_pclkscp),
                        .initial_rate = 2000 * FWK_MHZ,
                    }),
                },
            [CLOCK_PIK_IDX_SYSPERCLK] =
                {
                    .name = "SYSPERCLK",
                    .data =
                        &(
                            (struct mod_pik_clock_dev_config){
                                .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
                                .is_group_member = false,
                                .control_reg = &SYSTEM_PIK_PTR->SYSPERCLK_CTRL,
                                .divsys_reg = &SYSTEM_PIK_PTR->SYSPERCLK_DIV1,
                                .rate_table = rate_table_sysperclk,
                                .rate_count =
                                    FWK_ARRAY_SIZE(rate_table_sysperclk),
                                .initial_rate = 2000 * FWK_MHZ,
                            }),
                },
            [CLOCK_PIK_IDX_UARTCLK] =
                {
                    .name = "UARTCLK",
                    .data = &(
                        (struct mod_pik_clock_dev_config){
                            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
                            .is_group_member = false,
                            .control_reg = &SYSTEM_PIK_PTR->UARTCLK_CTRL,
                            .divsys_reg = &SYSTEM_PIK_PTR->UARTCLK_DIV1,
                            .rate_table = rate_table_uartclk,
                            .rate_count = FWK_ARRAY_SIZE(rate_table_uartclk),
                            .initial_rate = 2000 * FWK_MHZ,
                        }),
                },
            [CLOCK_PIK_IDX_COUNT] = { 0 }, /* Termination description. */
        };

static const struct fwk_element *pik_clock_get_element_table(fwk_id_t module_id)
{
    return pik_clock_element_table;
}

const struct fwk_module_config config_pik_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(pik_clock_get_element_table),
};
