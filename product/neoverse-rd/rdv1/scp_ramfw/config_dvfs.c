/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "config_dvfs.h"
#include "rd_alarm_idx.h"

#include <mod_dvfs.h>
#include <mod_scmi_perf.h>
#include <mod_sid.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#define DVFS_ELEMENT_IDX(n) \
    [DVFS_ELEMENT_IDX_CPU##n] = { .name = "GROUP" #n, .data = &cpu##n }

#define DVFS_DOMAIN_CPU_GROUP_IDX(n) \
    { \
        .psu_id = \
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, DVFS_ELEMENT_IDX_CPU##n), \
        .clock_id = \
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP##n), \
        .retry_ms = 0, .latency = 1200, .sustained_idx = 2, .opps = opps \
    }

/* The frequencies should match with the frequencies configured for css clock */
static struct mod_dvfs_opp opps[] = {
    { .level = 65UL, .frequency = 1313 * FWK_KHZ, .voltage = 800 },
    { .level = 75UL, .frequency = 1531 * FWK_KHZ, .voltage = 850 },
    { .level = 85UL, .frequency = 1750 * FWK_KHZ, .voltage = 900 },
    { .level = 105UL, .frequency = 2100 * FWK_KHZ, .voltage = 950 },
    { .level = 130UL, .frequency = 2600 * FWK_KHZ, .voltage = 1000 },
    { 0 }
};

static const struct mod_dvfs_domain_config cpu0 = DVFS_DOMAIN_CPU_GROUP_IDX(0);
static const struct mod_dvfs_domain_config cpu1 = DVFS_DOMAIN_CPU_GROUP_IDX(1);
static const struct mod_dvfs_domain_config cpu2 = DVFS_DOMAIN_CPU_GROUP_IDX(2);
static const struct mod_dvfs_domain_config cpu3 = DVFS_DOMAIN_CPU_GROUP_IDX(3);
static const struct mod_dvfs_domain_config cpu4 = DVFS_DOMAIN_CPU_GROUP_IDX(4);
static const struct mod_dvfs_domain_config cpu5 = DVFS_DOMAIN_CPU_GROUP_IDX(5);
static const struct mod_dvfs_domain_config cpu6 = DVFS_DOMAIN_CPU_GROUP_IDX(6);
static const struct mod_dvfs_domain_config cpu7 = DVFS_DOMAIN_CPU_GROUP_IDX(7);
static const struct mod_dvfs_domain_config cpu8 = DVFS_DOMAIN_CPU_GROUP_IDX(8);
static const struct mod_dvfs_domain_config cpu9 = DVFS_DOMAIN_CPU_GROUP_IDX(9);
static const struct mod_dvfs_domain_config cpu10 =
    DVFS_DOMAIN_CPU_GROUP_IDX(10);
static const struct mod_dvfs_domain_config cpu11 =
    DVFS_DOMAIN_CPU_GROUP_IDX(11);
static const struct mod_dvfs_domain_config cpu12 =
    DVFS_DOMAIN_CPU_GROUP_IDX(12);
static const struct mod_dvfs_domain_config cpu13 =
    DVFS_DOMAIN_CPU_GROUP_IDX(13);
static const struct mod_dvfs_domain_config cpu14 =
    DVFS_DOMAIN_CPU_GROUP_IDX(14);
static const struct mod_dvfs_domain_config cpu15 =
    DVFS_DOMAIN_CPU_GROUP_IDX(15);

static const struct fwk_element element_table[] = {
    DVFS_ELEMENT_IDX(0),
    DVFS_ELEMENT_IDX(1),
    DVFS_ELEMENT_IDX(2),
    DVFS_ELEMENT_IDX(3),
    DVFS_ELEMENT_IDX(4),
    DVFS_ELEMENT_IDX(5),
    DVFS_ELEMENT_IDX(6),
    DVFS_ELEMENT_IDX(7),
    DVFS_ELEMENT_IDX(8),
    DVFS_ELEMENT_IDX(9),
    DVFS_ELEMENT_IDX(10),
    DVFS_ELEMENT_IDX(11),
    DVFS_ELEMENT_IDX(12),
    DVFS_ELEMENT_IDX(13),
    DVFS_ELEMENT_IDX(14),
    DVFS_ELEMENT_IDX(15),
    { 0 },
};

static const struct fwk_element *dvfs_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

const struct fwk_module_config config_dvfs = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_get_element_table)
};
