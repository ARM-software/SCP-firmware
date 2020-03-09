/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_dvfs.h"
#include "config_psu.h"
#include "juno_alarm_idx.h"
#include "juno_clock.h"
#include "juno_id.h"

#include <mod_dvfs.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

/*
 * The power cost figures from this file are built using the dynamic power
 * consumption formula (P = CfV^2), where C represents the capacitance of one
 * processing element in the domain (a core or shader core). This power figure
 * is scaled linearly with the number of processing elements in the performance
 * domain to give a rough representation of the overall power draw. The
 * capacitance constants are given in mW/MHz/V^2 and were taken from the Linux
 * device trees, which provide a dynamic-power-coefficient field in uW/MHz/V^2.
 */

static const struct mod_dvfs_domain_config cpu_group_little_r0 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU,
        MOD_PSU_ELEMENT_IDX_VLITTLE),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
        JUNO_CLOCK_IDX_LITTLECLK),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
        JUNO_DVFS_ALARM_VLITTLE_IDX),
    .retry_ms = 1,
    .latency = 1450,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .frequency = 450 * FWK_MHZ,
            .voltage = 820,
            .power = (0.14 * 450 * 0.820 * 0.820),
        },
        {
            .frequency = 575 * FWK_MHZ,
            .voltage = 850,
            .power = (0.14 * 575 * 0.850 * 0.850),
        },
        {
            .frequency = 700 * FWK_MHZ,
            .voltage = 900,
            .power = (0.14 * 700 * 0.900 * 0.900),
        },
        {
            .frequency = 775 * FWK_MHZ,
            .voltage = 950,
            .power = (0.14 * 775 * 0.950 * 0.950),
        },
        {
            .frequency = 850 * FWK_MHZ,
            .voltage = 1000,
            .power = (0.14 * 850 * 1.000 * 1.000),
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config cpu_group_little_r1 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU,
        MOD_PSU_ELEMENT_IDX_VLITTLE),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
        JUNO_CLOCK_IDX_LITTLECLK),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
        JUNO_DVFS_ALARM_VLITTLE_IDX),
    .retry_ms = 1,
    .latency = 1450,
    .sustained_idx = 0,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .frequency = 650 * FWK_MHZ,
            .voltage = 800,
            .power = (0.14 * 650 * 0.800 * 0.800),
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config cpu_group_little_r2 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU,
        MOD_PSU_ELEMENT_IDX_VLITTLE),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
        JUNO_CLOCK_IDX_LITTLECLK),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
        JUNO_DVFS_ALARM_VLITTLE_IDX),
    .retry_ms = 1,
    .latency = 1450,
    .sustained_idx = 1,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .frequency = 450 * FWK_MHZ,
            .voltage = 820,
            .power = (0.14 * 450 * 0.820 * 0.820),
        },
        {
            .frequency = 800 * FWK_MHZ,
            .voltage = 900,
            .power = (0.14 * 800 * 0.900 * 0.900),
        },
        {
            .frequency = 950 * FWK_MHZ,
            .voltage = 1000,
            .power = (0.14 * 950 * 1.000 * 1.000),
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config cpu_group_big_r0 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU,
        MOD_PSU_ELEMENT_IDX_VBIG),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
        JUNO_CLOCK_IDX_BIGCLK),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
        JUNO_DVFS_ALARM_BIG_IDX),
    .retry_ms = 1,
    .latency = 1450,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .frequency = 450 * FWK_MHZ,
            .voltage = 820,
            .power = (0.53 * 450 * 0.820 * 0.820),
        },
        {
            .frequency = 625 * FWK_MHZ,
            .voltage = 850,
            .power = (0.53 * 625 * 0.850 * 0.850),
        },
        {
            .frequency = 800 * FWK_MHZ,
            .voltage = 900,
            .power = (0.53 * 800 * 0.900 * 0.900),
        },
        {
            .frequency = 950 * FWK_MHZ,
            .voltage = 950,
            .power = (0.53 * 950 * 0.950 * 0.950),
        },
        {
            .frequency = 1100 * FWK_MHZ,
            .voltage = 1000,
            .power = (0.53 * 1100 * 1.000 * 1.000),
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config cpu_group_big_r1 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU,
        MOD_PSU_ELEMENT_IDX_VBIG),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
        JUNO_CLOCK_IDX_BIGCLK),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
        JUNO_DVFS_ALARM_BIG_IDX),
    .retry_ms = 1,
    .latency = 1450,
    .sustained_idx = 1,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .frequency = 600 * FWK_MHZ,
            .voltage = 800,
            .power = (0.53 * 600 * 0.800 * 0.800),
        },
        {
            .frequency = 900 * FWK_MHZ,
            .voltage = 900,
            .power = (0.53 * 900 * 0.900 * 0.900),
        },
        {
            .frequency = 1150 * FWK_MHZ,
            .voltage = 1000,
            .power = (0.53 * 1150 * 1.000 * 1.000),
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config cpu_group_big_r2 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU,
        MOD_PSU_ELEMENT_IDX_VBIG),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
        JUNO_CLOCK_IDX_BIGCLK),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
        JUNO_DVFS_ALARM_BIG_IDX),
    .retry_ms = 1,
    .latency = 1450,
    .sustained_idx = 1,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .frequency = 600 * FWK_MHZ,
            .voltage = 820,
            .power = (0.45 * 600 * 0.820 * 0.820),
        },
        {
            .frequency = 1000 * FWK_MHZ,
            .voltage = 900,
            .power = (0.45 * 1000 * 0.900 * 0.900),
        },
        {
            .frequency = 1200 * FWK_MHZ,
            .voltage = 1000,
            .power = (0.45 * 1200 * 1.000 * 1.000),
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config gpu_r0 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU,
        MOD_PSU_ELEMENT_IDX_VGPU),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
        JUNO_CLOCK_IDX_GPUCLK),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
        JUNO_DVFS_ALARM_GPU_IDX),
    .retry_ms = 1,
    .latency = 1450,
    .sustained_idx = 4,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .frequency = 450 * FWK_MHZ,
            .voltage = 820,
            .power = (4.6875 * 450 * 0.820 * 0.820),
        },
        {
            .frequency = 487500 * FWK_KHZ,
            .voltage = 825,
            .power = (4.6875 * 487.5 * 0.825 * 0.825),
        },
        {
            .frequency = 525 * FWK_MHZ,
            .voltage = 850,
            .power = (4.6875 * 525 * 0.850 * 0.850),
        },
        {
            .frequency = 562500 * FWK_KHZ,
            .voltage = 875,
            .power = (4.6875 * 562.5 * 0.875 * 0.875),
        },
        {
            .frequency = 600 * FWK_MHZ,
            .voltage = 900,
            .power = (4.6875 * 600 * 0.900 * 0.900),
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config gpu_r1 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU,
        MOD_PSU_ELEMENT_IDX_VGPU),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
        JUNO_CLOCK_IDX_GPUCLK),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
        JUNO_DVFS_ALARM_GPU_IDX),
    .retry_ms = 1,
    .latency = 1450,
    .sustained_idx = 4,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .frequency = 450 * FWK_MHZ,
            .voltage = 820,
            .power = (4.6875 * 450 * 0.820 * 0.820),
        },
        {
            .frequency = 487500 * FWK_KHZ,
            .voltage = 825,
            .power = (4.6875 * 487.5 * 0.825 * 0.825),
        },
        {
            .frequency = 525 * FWK_MHZ,
            .voltage = 850,
            .power = (4.6875 * 525 * 0.850 * 0.850),
        },
        {
            .frequency = 562500 * FWK_KHZ,
            .voltage = 875,
            .power = (4.6875 * 562.5 * 0.875 * 0.875),
        },
        {
            .frequency = 600 * FWK_MHZ,
            .voltage = 900,
            .power = (4.6875 * 600 * 0.900 * 0.900),
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config gpu_r2 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU,
        MOD_PSU_ELEMENT_IDX_VGPU),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
        JUNO_CLOCK_IDX_GPUCLK),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
        JUNO_DVFS_ALARM_GPU_IDX),
    .retry_ms = 1,
    .latency = 1450,
    .sustained_idx = 1,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .frequency = 450 * FWK_MHZ,
            .voltage = 820,
            .power = (4.6875 * 450 * 0.820 * 0.820),
        },
        {
            .frequency = 600 * FWK_MHZ,
            .voltage = 900,
            .power = (4.6875 * 600 * 0.900 * 0.900),
        },
        { 0 }
    }
};

static const struct fwk_element element_table_r0[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
        .name = "LITTLE_CPU",
        .data = &cpu_group_little_r0,
    },
    [DVFS_ELEMENT_IDX_BIG] = {
        .name = "BIG_CPU",
        .data = &cpu_group_big_r0,
    },
    [DVFS_ELEMENT_IDX_GPU] = {
        .name = "GPU",
        .data = &gpu_r0,
    },
    { 0 }
};

static const struct fwk_element element_table_r1[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
        .name = "LITTLE_CPU",
        .data = &cpu_group_little_r1,
    },
    [DVFS_ELEMENT_IDX_BIG] = {
        .name = "BIG_CPU",
        .data = &cpu_group_big_r1,
    },
    [DVFS_ELEMENT_IDX_GPU] = {
        .name = "GPU",
        .data = &gpu_r1,
    },
    { 0 }
};

static const struct fwk_element element_table_r2[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
        .name = "LITTLE_CPU",
        .data = &cpu_group_little_r2,
    },
    [DVFS_ELEMENT_IDX_BIG] = {
        .name = "BIG_CPU",
        .data = &cpu_group_big_r2,
    },
    [DVFS_ELEMENT_IDX_GPU] = {
        .name = "GPU",
        .data = &gpu_r2,
    },
    { 0 }
};

static const struct fwk_element *dvfs_get_element_table(fwk_id_t module_id)
{
    int status;
    enum juno_idx_revision revision;

    status = juno_id_get_revision(&revision);
    fwk_assert(status == FWK_SUCCESS);

    if (revision == JUNO_IDX_REVISION_R0)
        return element_table_r0;
    if (revision == JUNO_IDX_REVISION_R1)
        return element_table_r1;
    return element_table_r2;
}

struct fwk_module_config config_dvfs = {
    .get_element_table = dvfs_get_element_table,
    .data = NULL,
};
