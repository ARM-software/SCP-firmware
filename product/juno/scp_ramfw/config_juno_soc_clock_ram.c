/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "juno_clock.h"
#include "juno_id.h"
#include "juno_scc.h"
#include "scp_config.h"

#include <mod_juno_soc_clock_ram.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

/*
 * Big cluster rate tables respectively for Juno r0, r1 and r2.
 */

static const struct juno_soc_clock_ram_rate big_cluster_r0_rate_table[] = {
    [0] = {
        .rate = 450 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 54,
            .nr = 1,
            .od = 6,
        },
    },
    [1] = {
        .rate = 625 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 50,
            .nr = 1,
            .od = 4,
        },
    },
    [2] = {
        .rate = 800 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_SYSINCLK,
        .divider = 2,
        .pll_setting = {0},
    },
    [3] = {
        .rate = 950 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 38,
            .nr = 1,
            .od = 2,
        },
    },
    [4] = {
        .rate = 1100 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 44,
            .nr = 1,
            .od = 2,
        },
    },
};

static const struct juno_soc_clock_ram_rate big_cluster_r1_rate_table[] = {
    [0] = {
        .rate = 600 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 48,
            .nr = 1,
            .od = 4,
        },
    },
    [1] = {
        .rate = 900 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 36,
            .nr = 1,
            .od = 2,
        },
    },
    [2] = {
        .rate = 1150 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 46,
            .nr = 1,
            .od = 2,
        },
    },
};

static const struct juno_soc_clock_ram_rate big_cluster_r2_rate_table[] = {
    [0] = {
        .rate = 600 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 48,
            .nr = 1,
            .od = 4,
        },
    },
    [1] = {
        .rate = 1000 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 40,
            .nr = 1,
            .od = 2,
        },
    },
    [2] = {
        .rate = 1200 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 48,
            .nr = 1,
            .od = 2,
        },
    },
};

/*
 * Little cluster rate tables respectively for Juno r0, r1 and r2.
 */

static const struct juno_soc_clock_ram_rate
    little_cluster_r0_rate_table[] = {
    [0] = {
        .rate = 450 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 54,
            .nr = 1,
            .od = 6,
        },
    },
    [1] = {
        .rate = 575 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 69,
            .nr = 1,
            .od = 6,
        },
    },
    [2] = {
        .rate = 700 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 56,
            .nr = 1,
            .od = 4,
        },
    },
    [3] = {
        .rate = 775 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 62,
            .nr = 1,
            .od = 4,
        },
    },
    [4] = {
        .rate = 850 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 68,
            .nr = 1,
            .od = 4,
        },
    },
};

static const struct juno_soc_clock_ram_rate
    little_cluster_r1_rate_table[] = {
    [0] = {
        .rate = 650 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 52,
            .nr = 1,
            .od = 4,
        },
    },
};

static const struct juno_soc_clock_ram_rate
    little_cluster_r2_rate_table[] = {
    [0] = {
        .rate = 450 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 54,
            .nr = 1,
            .od = 6,
        },
    },
    [1] = {
        .rate = 800 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 64,
            .nr = 1,
            .od = 4,
        },
    },
    [2] = {
        .rate = 950 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 38,
            .nr = 1,
            .od = 2,
        },
    },
};

/*
 * GPU rate tables respectively for Juno r0, r1 and r2.
 */

static const struct juno_soc_clock_ram_rate gpu_r0_rate_table[] = {
    [0] = {
        .rate = 450 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 54,
            .nr = 1,
            .od = 6,
        },
    },
    [1] = {
        .rate = 487500 * FWK_KHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 117,
            .nr = 2,
            .od = 6,
        },
    },
    [2] = {
        .rate = 525 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 63,
            .nr = 1,
            .od = 6,
        },
    },
    [3] = {
        .rate = 562500 * FWK_KHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 135,
            .nr = 2,
            .od = 6,
        },
    },
    [4] = {
        .rate = 600 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 48,
            .nr = 1,
            .od = 4,
        },
    },
};

static const struct juno_soc_clock_ram_rate gpu_r1_rate_table[] = {
    [0] = {
        .rate = 450 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 54,
            .nr = 1,
            .od = 6,
        },
    },
    [1] = {
        .rate = 487500 * FWK_KHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 117,
            .nr = 2,
            .od = 6,
        },
    },
    [2] = {
        .rate = 525 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 63,
            .nr = 1,
            .od = 6,
        },
    },
    [3] = {
        .rate = 562500 * FWK_KHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 135,
            .nr = 2,
            .od = 6,
        },
    },
    [4] = {
        .rate = 600 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 48,
            .nr = 1,
            .od = 4,
        },
    },
};

static const struct juno_soc_clock_ram_rate gpu_r2_rate_table[] = {
    [0] = {
        .rate = 450 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 54,
            .nr = 1,
            .od = 6,
        },
    },
    [1] = {
        .rate = 600 * FWK_MHZ,
        .source = SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
        .divider = 1,
        .pll_setting = {
            .nf = 48,
            .nr = 1,
            .od = 4,
        },
    },
};

static const struct juno_soc_clock_ram_rate
    *big_cluster_rate_tables[JUNO_IDX_REVISION_COUNT] = {
    big_cluster_r0_rate_table,
    big_cluster_r1_rate_table,
    big_cluster_r2_rate_table,
};

static const size_t big_cluster_rate_counts[] = {
    FWK_ARRAY_SIZE(big_cluster_r0_rate_table),
    FWK_ARRAY_SIZE(big_cluster_r1_rate_table),
    FWK_ARRAY_SIZE(big_cluster_r2_rate_table),
};

static const struct juno_soc_clock_ram_rate
    *little_cluster_rate_tables[JUNO_IDX_REVISION_COUNT] = {
    little_cluster_r0_rate_table,
    little_cluster_r1_rate_table,
    little_cluster_r2_rate_table,
};

static const size_t little_cluster_rate_counts[] = {
    FWK_ARRAY_SIZE(little_cluster_r0_rate_table),
    FWK_ARRAY_SIZE(little_cluster_r1_rate_table),
    FWK_ARRAY_SIZE(little_cluster_r2_rate_table),
};

static const struct juno_soc_clock_ram_rate
    *gpu_rate_tables[JUNO_IDX_REVISION_COUNT] = {
    gpu_r0_rate_table,
    gpu_r1_rate_table,
    gpu_r2_rate_table,
};

static const size_t gpu_rate_counts[] = {
    FWK_ARRAY_SIZE(gpu_r0_rate_table),
    FWK_ARRAY_SIZE(gpu_r1_rate_table),
    FWK_ARRAY_SIZE(gpu_r2_rate_table),
};

static struct mod_juno_soc_clock_ram_dev_config big_cluster_clock_config = {
    .pll = &SCC->PLL[PLL_IDX_BIG],
    .clock_control = &SCP_CONFIG->BIGCLK_CONTROL,
};

static struct mod_juno_soc_clock_ram_dev_config little_cluster_clock_config = {
    .pll = &SCC->PLL[PLL_IDX_LITTLE],
    .clock_control = &SCP_CONFIG->LITTLECLK_CONTROL,
};

static struct mod_juno_soc_clock_ram_dev_config gpu_clock_config = {
    .pll = &SCC->PLL[PLL_IDX_GPU],
    .clock_control = &SCP_CONFIG->GPUCLK_CONTROL,
};

static struct fwk_element juno_soc_clock_element_table[] = {
    [JUNO_CLOCK_SOC_RAM_IDX_BIGCLK] = {
        .name = "",
        .data = &big_cluster_clock_config,
    },
    [JUNO_CLOCK_SOC_RAM_IDX_LITTLECLK] = {
        .name = "",
        .data = &little_cluster_clock_config,
    },
    [JUNO_CLOCK_SOC_RAM_IDX_GPUCLK] = {
        .name = "",
        .data = &gpu_clock_config,
    },
    [JUNO_CLOCK_SOC_RAM_IDX_COUNT] = {0}, /* Termination description. */
};

static const struct fwk_element *juno_soc_clock_ram_get_element_table
    (fwk_id_t module_id)
{
    int status;
    enum juno_idx_revision revision;

    status = juno_id_get_revision(&revision);
    if (status != FWK_SUCCESS)
        return NULL;

    big_cluster_clock_config.rate_table = big_cluster_rate_tables[revision];
    big_cluster_clock_config.rate_count = big_cluster_rate_counts[revision];

    little_cluster_clock_config.rate_table =
        little_cluster_rate_tables[revision];
    little_cluster_clock_config.rate_count =
        little_cluster_rate_counts[revision];

    gpu_clock_config.rate_table = gpu_rate_tables[revision];
    gpu_clock_config.rate_count = gpu_rate_counts[revision];

    return juno_soc_clock_element_table;
}

struct fwk_module_config config_juno_soc_clock_ram = {
    .get_element_table = juno_soc_clock_ram_get_element_table,
    .data = &((struct mod_juno_soc_clock_ram_config) {
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .debug_pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                POWER_DOMAIN_IDX_DBGSYS),
            .systop_pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                POWER_DOMAIN_IDX_SYSTOP),
        }),
};
