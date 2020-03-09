/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "n1sdp_pik_cpu.h"
#include "n1sdp_pik_debug.h"
#include "n1sdp_pik_scp.h"
#include "n1sdp_pik_system.h"
#include "n1sdp_scc_reg.h"
#include "n1sdp_scp_pik.h"
#include "n1sdp_system_clock.h"

#include <mod_pik_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>

/*
 * Rate lookup tables
 */

static const struct mod_pik_clock_rate rate_table_iofpga_tmif2xclk[] = {
    {
        .rate = SCC_CLK_RATE_IOFPGA_TMIF2XCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_IOFPGA_TMIF2XCLK,
    },
};

static const struct mod_pik_clock_rate rate_table_iofpga_tsif2xclk[] = {
    {
        .rate = SCC_CLK_RATE_IOFPGA_TSIF2XCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_IOFPGA_TSIF2XCLK,
    },
};

static const struct mod_pik_clock_rate rate_table_sysapbclk[] = {
    {
        .rate = SCC_CLK_RATE_SYSAPBCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_SYSAPBCLK,
    },
};

static const struct mod_pik_clock_rate rate_table_scpnicclk[] = {
    {
        .rate = SCC_CLK_RATE_SCPNICCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_SCPNICCLK,
    },
};

static const struct mod_pik_clock_rate rate_table_scpi2cclk[] = {
    {
        .rate = SCC_CLK_RATE_SCPI2CCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_SCPI2CCLK,
    },
};

static const struct mod_pik_clock_rate rate_table_scpqspiclk[] = {
    {
        .rate = SCC_CLK_RATE_SCPQSPICLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSREFCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = 1,
    },
};

static const struct mod_pik_clock_rate rate_table_sensorclk[] = {
    {
        .rate = SCC_CLK_RATE_SENSORCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_SENSORCLK,
    },
};

static const struct mod_pik_clock_rate rate_table_mcpnicclk[] = {
    {
        .rate = SCC_CLK_RATE_MCPNICCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_MCPNICCLK,
    },
};

static const struct mod_pik_clock_rate rate_table_mcpi2cclk[] = {
    {
        .rate = SCC_CLK_RATE_MCPI2CCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_MCPI2CCLK,
    },
};

static const struct mod_pik_clock_rate rate_table_mcpqspiclk[] = {
    {
        .rate = SCC_CLK_RATE_MCPQSPICLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSREFCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = 1,
    },
};

static const struct mod_pik_clock_rate rate_table_pcieaxiclk[] = {
    {
        .rate = SCC_CLK_RATE_PCIEAXICLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_PCIEAXICLK,
    },
};

static const struct mod_pik_clock_rate rate_table_ccixaxiclk[] = {
    {
        .rate = SCC_CLK_RATE_CCIXAXICLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_CCIXAXICLK,
    },
};

static const struct mod_pik_clock_rate rate_table_pcieapbclk[] = {
    {
        .rate = SCC_CLK_RATE_PCIEAPBCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_PCIEAPBCLK,
    },
};

static const struct mod_pik_clock_rate rate_table_ccixapbclk[] = {
    {
        .rate = SCC_CLK_RATE_CCIXAPBCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / SCC_CLK_RATE_CCIXAPBCLK,
    },
};

static struct mod_pik_clock_rate rate_table_cpu_group_0[] = {
    {
        .rate = PIK_CLK_RATE_CLUS0_CPU,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1,
    },
};

static struct mod_pik_clock_rate rate_table_cpu_group_1[] = {
    {
        .rate = PIK_CLK_RATE_CLUS1_CPU,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1,
    },
};

static struct mod_pik_clock_rate rate_table_clus_0[] = {
    {
        .rate = PIK_CLK_RATE_CLUS0,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL1,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1,
    },
};

static struct mod_pik_clock_rate rate_table_clus_1[] = {
    {
        .rate = PIK_CLK_RATE_CLUS1,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL1,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1,
    },
};

static struct mod_pik_clock_rate rate_table_clus_0_ppu[] = {
    {
        .rate = PIK_CLK_RATE_CLUS0_PPU,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_CLUS0_PPU,
    },
};

static struct mod_pik_clock_rate rate_table_clus_1_ppu[] = {
    {
        .rate = PIK_CLK_RATE_CLUS1_PPU,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_CLUS1_PPU,
    },
};

static struct mod_pik_clock_rate rate_table_clus0_pclk[] = {
    {
        .rate = PIK_CLK_RATE_CLUS0_PCLK,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = PIK_CLK_RATE_CLUS0 / PIK_CLK_RATE_CLUS0_PCLK,
    },
};

static struct mod_pik_clock_rate rate_table_clus0_atclk[] = {
    {
        .rate = PIK_CLK_RATE_CLUS0_ATCLK,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = PIK_CLK_RATE_CLUS0 / PIK_CLK_RATE_CLUS0_ATCLK,
    },
};

static struct mod_pik_clock_rate rate_table_clus0_gic[] = {
    {
        .rate = PIK_CLK_RATE_CLUS0_GIC,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = PIK_CLK_RATE_CLUS0 / PIK_CLK_RATE_CLUS0_GIC,
    },
};

static struct mod_pik_clock_rate rate_table_clus0_ambaclk[] = {
    {
        .rate = PIK_CLK_RATE_CLUS0_AMBACLK,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = PIK_CLK_RATE_CLUS0 / PIK_CLK_RATE_CLUS0_AMBACLK,
    },
};

static struct mod_pik_clock_rate rate_table_clus1_pclk[] = {
    {
        .rate = PIK_CLK_RATE_CLUS1_PCLK,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = PIK_CLK_RATE_CLUS1 / PIK_CLK_RATE_CLUS1_PCLK,
    },
};

static struct mod_pik_clock_rate rate_table_clus1_atclk[] = {
    {
        .rate = PIK_CLK_RATE_CLUS1_ATCLK,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = PIK_CLK_RATE_CLUS1 / PIK_CLK_RATE_CLUS1_ATCLK,
    },
};

static struct mod_pik_clock_rate rate_table_clus1_gic[] = {
    {
        .rate = PIK_CLK_RATE_CLUS1_GIC,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = PIK_CLK_RATE_CLUS1 / PIK_CLK_RATE_CLUS1_GIC,
    },
};

static struct mod_pik_clock_rate rate_table_clus1_ambaclk[] = {
    {
        .rate = PIK_CLK_RATE_CLUS1_AMBACLK,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = PIK_CLK_RATE_CLUS1 / PIK_CLK_RATE_CLUS1_AMBACLK,
    },
};

static const struct mod_pik_clock_rate rate_table_scp_coreclk[] = {
    {
        .rate = PIK_CLK_RATE_SCP_CORECLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_SCP_CORECLK,
    },
};

static const struct mod_pik_clock_rate rate_table_scp_aclk[] = {
    {
        .rate = PIK_CLK_RATE_SCP_AXICLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_SCP_AXICLK,
    },
};

static const struct mod_pik_clock_rate rate_table_scp_syncclk[] = {
    {
        .rate = PIK_CLK_RATE_SCP_SYNCCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_SCP_SYNCCLK,
    },
};

static const struct mod_pik_clock_rate rate_table_sys_ppu[] = {
    {
        .rate = PIK_CLK_RATE_SYS_PPU,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_SYS_PPU,
    },
};

static const struct mod_pik_clock_rate rate_table_sys_intclk[] = {
    {
        .rate = PIK_CLK_RATE_INTERCONNECT,
        .source = MOD_PIK_CLOCK_INTCLK_SOURCE_INTPLL,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1,
    },
};

static const struct mod_pik_clock_rate rate_table_pclkscp[] = {
    {
        .rate = PIK_CLK_RATE_PCLKSCP,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_PCLKSCP,
    },
};

static const struct mod_pik_clock_rate rate_table_gicclk[] = {
    {
        .rate = PIK_CLK_RATE_SYS_GIC,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_SYS_GIC,
    },
};

static const struct mod_pik_clock_rate rate_table_syspclkdbg[] = {
    {
        .rate = PIK_CLK_RATE_SYSPCLKDBG,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_SYSPCLKDBG,
    },
};

static const struct mod_pik_clock_rate rate_table_sysperclk[] = {
    {
        .rate = PIK_CLK_RATE_SYSPERCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_SYSPERCLK,
    },
};

static const struct mod_pik_clock_rate rate_table_uart[] = {
    {
        .rate = PIK_CLK_RATE_UART,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSREFCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = 1,
    },
};

static const struct mod_pik_clock_rate rate_table_tcu0[] = {
    {
        .rate = PIK_CLK_RATE_TCU0,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_TCU0,
    },
};

static const struct mod_pik_clock_rate rate_table_tcu1[] = {
    {
        .rate = PIK_CLK_RATE_TCU1,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_TCU1,
    },
};

static const struct mod_pik_clock_rate rate_table_tcu2[] = {
    {
        .rate = PIK_CLK_RATE_TCU2,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_TCU2,
    },
};

static const struct mod_pik_clock_rate rate_table_tcu3[] = {
    {
        .rate = PIK_CLK_RATE_TCU3,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_TCU3,
    },
};

static const struct mod_pik_clock_rate rate_table_atclkdbg[] = {
    {
        .rate = PIK_CLK_RATE_ATCLKDBG,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_ATCLKDBG,
    },
};

static const struct mod_pik_clock_rate rate_table_pclkdbg[] = {
    {
        .rate = PIK_CLK_RATE_PCLKDBG,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = PIK_CLK_RATE_ATCLKDBG / PIK_CLK_RATE_PCLKDBG,
    },
};

static const struct mod_pik_clock_rate rate_table_traceclk[] = {
    {
        .rate = PIK_CLK_RATE_TRACECLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_TRACECLK,
    },
};

static const struct mod_pik_clock_rate rate_table_sys_dmcclk[] = {
    {
        .rate = PIK_CLK_RATE_DMC,
        .source = MOD_PIK_CLOCK_DMCCLK_SOURCE_DDRPLL,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1,
    },
};

static const struct fwk_element pik_clock_element_table[] = {
    [CLOCK_SCC_IDX_IOFPGA_TMIF2XCLK] = {
        .name = "IOFPGA TMIF2XCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->IOFPGA_TMIF2XCLK_CTRL,
            .divsys_reg = &SCC->IOFPGA_TMIF2XCLK_DIV,
            .rate_table = rate_table_iofpga_tmif2xclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_iofpga_tmif2xclk),
            .initial_rate = SCC_CLK_RATE_IOFPGA_TMIF2XCLK,
        }),
    },
    [CLOCK_SCC_IDX_IOFPGA_TSIF2XCLK] = {
        .name = "IOFPGA TSIF2XCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->IOFPGA_TSIF2XCLK_CTRL,
            .divsys_reg = &SCC->IOFPGA_TSIF2XCLK_DIV,
            .rate_table = rate_table_iofpga_tsif2xclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_iofpga_tsif2xclk),
            .initial_rate = SCC_CLK_RATE_IOFPGA_TSIF2XCLK,
        }),
    },
    [CLOCK_SCC_IDX_SYSAPBCLK] = {
        .name = "SYSAPBCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->SYSAPBCLK_CTRL,
            .divsys_reg = &SCC->SYSAPBCLK_DIV,
            .rate_table = rate_table_sysapbclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sysapbclk),
            .initial_rate = SCC_CLK_RATE_SYSAPBCLK,
        }),
    },
    [CLOCK_SCC_IDX_SCPNICCLK] = {
        .name = "SCPNICCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->SCPNICCLK_CTRL,
            .divsys_reg = &SCC->SCPNICCLK_DIV,
            .rate_table = rate_table_scpnicclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_scpnicclk),
            .initial_rate = SCC_CLK_RATE_SCPNICCLK,
        }),
    },
    [CLOCK_SCC_IDX_SCPI2CCLK] = {
        .name = "SCPI2CCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->SCPI2CCLK_CTRL,
            .divsys_reg = &SCC->SCPI2CCLK_DIV,
            .rate_table = rate_table_scpi2cclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_scpi2cclk),
            .initial_rate = SCC_CLK_RATE_SCPI2CCLK,
        }),
    },
    [CLOCK_SCC_IDX_SCPQSPICLK] = {
        .name = "SCPQSPICLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->SCPQSPICLK_CTRL,
            .divsys_reg = &SCC->SCPQSPICLK_DIV,
            .rate_table = rate_table_scpqspiclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_scpqspiclk),
            .initial_rate = SCC_CLK_RATE_SCPQSPICLK,
        }),
    },
    [CLOCK_SCC_IDX_SENSORCLK] = {
        .name = "SENSORCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->SENSORCLK_CTRL,
            .divsys_reg = &SCC->SENSORCLK_DIV,
            .rate_table = rate_table_sensorclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sensorclk),
            .initial_rate = SCC_CLK_RATE_SENSORCLK,
        }),
    },
    [CLOCK_SCC_IDX_MCPNICCLK] = {
        .name = "MCPNICCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->MCPNICCLK_CTRL,
            .divsys_reg = &SCC->MCPNICCLK_DIV,
            .rate_table = rate_table_mcpnicclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_mcpnicclk),
            .initial_rate = SCC_CLK_RATE_MCPNICCLK,
        }),
    },
    [CLOCK_SCC_IDX_MCPI2CCLK] = {
        .name = "MCPI2CCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->MCPI2CCLK_CTRL,
            .divsys_reg = &SCC->MCPI2CCLK_DIV,
            .rate_table = rate_table_mcpi2cclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_mcpi2cclk),
            .initial_rate = SCC_CLK_RATE_MCPI2CCLK,
        }),
    },
    [CLOCK_SCC_IDX_MCPQSPICLK] = {
        .name = "MCPQSPICLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->MCPQSPICLK_CTRL,
            .divsys_reg = &SCC->MCPQSPICLK_DIV,
            .rate_table = rate_table_mcpqspiclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_mcpqspiclk),
            .initial_rate = SCC_CLK_RATE_MCPQSPICLK,
        }),
    },
    [CLOCK_SCC_IDX_PCIEAXICLK] = {
        .name = "PCIEAXICLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->PCIEAXICLK_CTRL,
            .divsys_reg = &SCC->PCIEAXICLK_DIV,
            .rate_table = rate_table_pcieaxiclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_pcieaxiclk),
            .initial_rate = SCC_CLK_RATE_PCIEAXICLK,
        }),
    },
    [CLOCK_SCC_IDX_CCIXAXICLK] = {
        .name = "CCIXAXICLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->CCIXAXICLK_CTRL,
            .divsys_reg = &SCC->CCIXAXICLK_DIV,
            .rate_table = rate_table_ccixaxiclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_ccixaxiclk),
            .initial_rate = SCC_CLK_RATE_CCIXAXICLK,
        }),
    },
    [CLOCK_SCC_IDX_PCIEAPBCLK] = {
        .name = "PCIEAPBCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->PCIEAPBCLK_CTRL,
            .divsys_reg = &SCC->PCIEAPBCLK_DIV,
            .rate_table = rate_table_pcieapbclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_pcieapbclk),
            .initial_rate = SCC_CLK_RATE_PCIEAPBCLK,
        }),
    },
    [CLOCK_SCC_IDX_CCIXAPBCLK] = {
        .name = "CCIXAPBCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCC->CCIXAPBCLK_CTRL,
            .divsys_reg = &SCC->CCIXAPBCLK_DIV,
            .rate_table = rate_table_ccixapbclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_ccixapbclk),
            .initial_rate = SCC_CLK_RATE_CCIXAPBCLK,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU0] = {
        .name = "CLUS0_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(0)->CORECLK[0].CTRL,
            .divext_reg = &PIK_CLUSTER(0)->CORECLK[0].DIV,
            .modulator_reg = &PIK_CLUSTER(0)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group_0,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_0),
            .initial_rate = PIK_CLK_RATE_CLUS0_CPU,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU1] = {
        .name = "CLUS0_CPU1",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(0)->CORECLK[1].CTRL,
            .divext_reg = &PIK_CLUSTER(0)->CORECLK[1].DIV,
            .modulator_reg = &PIK_CLUSTER(0)->CORECLK[1].MOD,
            .rate_table = rate_table_cpu_group_0,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_0),
            .initial_rate = PIK_CLK_RATE_CLUS0_CPU,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_CPU0] = {
        .name = "CLUS1_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(1)->CORECLK[0].CTRL,
            .divext_reg = &PIK_CLUSTER(1)->CORECLK[0].DIV,
            .modulator_reg = &PIK_CLUSTER(1)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group_1,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_1),
            .initial_rate = PIK_CLK_RATE_CLUS1_CPU,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_CPU1] = {
        .name = "CLUS1_CPU1",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(1)->CORECLK[1].CTRL,
            .divext_reg = &PIK_CLUSTER(1)->CORECLK[1].DIV,
            .modulator_reg = &PIK_CLUSTER(1)->CORECLK[1].MOD,
            .rate_table = rate_table_cpu_group_1,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_1),
            .initial_rate = PIK_CLK_RATE_CLUS1_CPU,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0] = {
        .name = "CLUS0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->CLUSCLK_CTRL,
            .divext_reg = &PIK_CLUSTER(0)->CLUSCLK_DIV1,
            .rate_table = rate_table_clus_0,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_0),
            .initial_rate = PIK_CLK_RATE_CLUS0,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1] = {
        .name = "CLUS1",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->CLUSCLK_CTRL,
            .divext_reg = &PIK_CLUSTER(1)->CLUSCLK_DIV1,
            .rate_table = rate_table_clus_1,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_1),
            .initial_rate = PIK_CLK_RATE_CLUS1,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_PPU] = {
        .name = "CLUS0 PPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(0)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_0_ppu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_0_ppu),
            .initial_rate = PIK_CLK_RATE_CLUS0_PPU,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_PPU] = {
        .name = "CLUS1 PPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(1)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_1_ppu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_1_ppu),
            .initial_rate = PIK_CLK_RATE_CLUS1_PPU,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_PCLK] = {
        .name = "CLUS0 PCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->PCLK_CTRL,
            .rate_table = rate_table_clus0_pclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus0_pclk),
            .initial_rate = PIK_CLK_RATE_CLUS0_PCLK,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_ATCLK] = {
        .name = "CLUS0 ATCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->ATCLK_CTRL,
            .rate_table = rate_table_clus0_atclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus0_atclk),
            .initial_rate = PIK_CLK_RATE_CLUS0_ATCLK,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_GIC] = {
        .name = "CLUS0 GIC",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->GICCLK_CTRL,
            .rate_table = rate_table_clus0_gic,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus0_gic),
            .initial_rate = PIK_CLK_RATE_CLUS0_GIC,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_AMBACLK] = {
        .name = "CLUS0 AMBACLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->AMBACLK_CTRL,
            .rate_table = rate_table_clus0_ambaclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus0_ambaclk),
            .initial_rate = PIK_CLK_RATE_CLUS0_AMBACLK,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_PCLK] = {
        .name = "CLUS1 PCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->PCLK_CTRL,
            .rate_table = rate_table_clus1_pclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus1_pclk),
            .initial_rate = PIK_CLK_RATE_CLUS1_PCLK,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_ATCLK] = {
        .name = "CLUS1 ATCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->ATCLK_CTRL,
            .rate_table = rate_table_clus1_atclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus1_atclk),
            .initial_rate = PIK_CLK_RATE_CLUS1_ATCLK,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_GIC] = {
        .name = "CLUS1 GIC",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->GICCLK_CTRL,
            .rate_table = rate_table_clus1_gic,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus1_gic),
            .initial_rate = PIK_CLK_RATE_CLUS1_GIC,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_AMBACLK] = {
        .name = "CLUS1 AMBACLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->AMBACLK_CTRL,
            .rate_table = rate_table_clus1_ambaclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus1_ambaclk),
            .initial_rate = PIK_CLK_RATE_CLUS1_AMBACLK,
        }),
    },
    [CLOCK_PIK_IDX_SCP_CORECLK] = {
        .name = "SCP CORECLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SCP->CORECLK_CTRL,
            .divsys_reg = &PIK_SCP->CORECLK_DIV1,
            .rate_table = rate_table_scp_coreclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_scp_coreclk),
            .initial_rate = PIK_CLK_RATE_SCP_CORECLK,
        }),
    },
    [CLOCK_PIK_IDX_SCP_AXICLK] = {
        .name = "SCP AXICLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SCP->ACLK_CTRL,
            .divsys_reg = &PIK_SCP->ACLK_DIV1,
            .rate_table = rate_table_scp_aclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_scp_aclk),
            .initial_rate = PIK_CLK_RATE_SCP_AXICLK,
        }),
    },
    [CLOCK_PIK_IDX_SCP_SYNCCLK] = {
        .name = "SCP SYNCCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SCP->SYNCCLK_CTRL,
            .divsys_reg = &PIK_SCP->SYNCCLK_DIV1,
            .rate_table = rate_table_scp_syncclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_scp_syncclk),
            .initial_rate = PIK_CLK_RATE_SCP_SYNCCLK,
        }),
    },
    [CLOCK_PIK_IDX_SYS_PPU] = {
        .name = "SYS PPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->PPUCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->PPUCLK_DIV1,
            .rate_table = rate_table_sys_ppu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_ppu),
            .initial_rate = PIK_CLK_RATE_SYS_PPU,
        }),
    },
    [CLOCK_PIK_IDX_INTERCONNECT] = {
        .name = "INTERCONNECT",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->INTCLK_CTRL,
            .divext_reg = &PIK_SYSTEM->INTCLK_DIV1,
            .rate_table = rate_table_sys_intclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_intclk),
            .initial_rate = PIK_CLK_RATE_INTERCONNECT,
        }),
    },
    [CLOCK_PIK_IDX_PCLKSCP] = {
        .name = "PCLKSCP",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->PCLKSCP_CTRL,
            .divsys_reg = &PIK_SYSTEM->PCLKSCP_DIV1,
            .rate_table = rate_table_pclkscp,
            .rate_count = FWK_ARRAY_SIZE(rate_table_pclkscp),
            .initial_rate = PIK_CLK_RATE_PCLKSCP,
        }),
    },
    [CLOCK_PIK_IDX_SYS_GIC] = {
        .name = "SYS GIC",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->GICCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->GICCLK_DIV1,
            .rate_table = rate_table_gicclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_gicclk),
            .initial_rate = PIK_CLK_RATE_SYS_GIC,
        }),
    },
    [CLOCK_PIK_IDX_SYSPCLKDBG] = {
        .name = "SYSPCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->SYSPCLKDBG_CTRL,
            .divsys_reg = &PIK_SYSTEM->SYSPCLKDBG_DIV1,
            .rate_table = rate_table_syspclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_syspclkdbg),
            .initial_rate = PIK_CLK_RATE_SYSPCLKDBG,
        }),
    },
    [CLOCK_PIK_IDX_SYSPERCLK] = {
        .name = "SYSPERCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->SYSPERCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->SYSPERCLK_DIV1,
            .rate_table = rate_table_sysperclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sysperclk),
            .initial_rate = PIK_CLK_RATE_SYSPERCLK,
        }),
    },
    [CLOCK_PIK_IDX_UART] = {
        .name = "UART",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->UARTCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->UARTCLK_DIV1,
            .rate_table = rate_table_uart,
            .rate_count = FWK_ARRAY_SIZE(rate_table_uart),
            .initial_rate = PIK_CLK_RATE_UART,
        }),
    },
    [CLOCK_PIK_IDX_TCU0] = {
        .name = "TCU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->TCUCLK[0].TCUCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->TCUCLK[0].TCUCLK_DIV1,
            .rate_table = rate_table_tcu0,
            .rate_count = FWK_ARRAY_SIZE(rate_table_tcu0),
            .initial_rate = PIK_CLK_RATE_TCU0,
        }),
    },
    [CLOCK_PIK_IDX_TCU1] = {
        .name = "TCU1",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->TCUCLK[1].TCUCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->TCUCLK[1].TCUCLK_DIV1,
            .rate_table = rate_table_tcu1,
            .rate_count = FWK_ARRAY_SIZE(rate_table_tcu1),
            .initial_rate = PIK_CLK_RATE_TCU1,
        }),
    },
    [CLOCK_PIK_IDX_TCU2] = {
        .name = "TCU2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->TCUCLK[2].TCUCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->TCUCLK[2].TCUCLK_DIV1,
            .rate_table = rate_table_tcu2,
            .rate_count = FWK_ARRAY_SIZE(rate_table_tcu2),
            .initial_rate = PIK_CLK_RATE_TCU2,
        }),
    },
    [CLOCK_PIK_IDX_TCU3] = {
        .name = "TCU3",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->TCUCLK[3].TCUCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->TCUCLK[3].TCUCLK_DIV1,
            .rate_table = rate_table_tcu3,
            .rate_count = FWK_ARRAY_SIZE(rate_table_tcu3),
            .initial_rate = PIK_CLK_RATE_TCU3,
        }),
    },
    [CLOCK_PIK_IDX_ATCLKDBG] = {
        .name = "ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_DEBUG->ATCLKDBG_CTRL,
            .divsys_reg = &PIK_DEBUG->ATCLKDBG_DIV1,
            .rate_table = rate_table_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_atclkdbg),
            .initial_rate = PIK_CLK_RATE_ATCLKDBG,
        }),
    },
    [CLOCK_PIK_IDX_PCLKDBG] = {
        .name = "PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_DEBUG->PCLKDBG_CTRL,
            .rate_table = rate_table_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_pclkdbg),
            .initial_rate = PIK_CLK_RATE_PCLKDBG,
        }),
    },
    [CLOCK_PIK_IDX_TRACECLK] = {
        .name = "TRACECLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_DEBUG->TRACECLK_CTRL,
            .divsys_reg = &PIK_DEBUG->TRACECLK_DIV1,
            .rate_table = rate_table_traceclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_traceclk),
            .initial_rate = PIK_CLK_RATE_TRACECLK,
        }),
    },
    [CLOCK_PIK_IDX_DMC] = {
        .name = "DMC",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->DMCCLK_CTRL,
            .divext_reg = &PIK_SYSTEM->DMCCLK_DIV1,
            .rate_table = rate_table_sys_dmcclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_dmcclk),
            .initial_rate = PIK_CLK_RATE_DMC,
        }),
    },
    [CLOCK_PIK_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *pik_clock_get_element_table
    (fwk_id_t module_id)
{
    return pik_clock_element_table;
}

const struct fwk_module_config config_pik_clock = {
    .get_element_table = pik_clock_get_element_table,
};
