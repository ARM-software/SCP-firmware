/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_rcar_pd_core.h>
#include <config_power_domain.h>
#include <rcar_core.h>
#include <rcar_mmap.h>

#include <mod_rcar_pd_core.h>
#include <mod_power_domain.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdio.h>
#include <string.h>

/* Maximum core name size including the null terminator */
#define CORE_NAME_SIZE 12

/* Maximum cluster name size including the null terminator */
#define CLUS_NAME_SIZE 6

static const char *core_pd_name_table[RCAR_CORE_PER_CLUSTER_MAX] = {
    "CLUS0CORE0", "CLUS0CORE1", "CLUS0CORE2", "CLUS0CORE3",
    "CLUS1CORE0", "CLUS1CORE1", "CLUS1CORE2", "CLUS1CORE3",
};

static const char *cluster_pd_name_table[2] = {
    "CLUS0",
    "CLUS1",
};

/* Module configuration data */
static struct mod_rcar_pd_core_config rcar_pd_core_config_data = {
    .pd_notification_id = FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_POWER_DOMAIN,
        MOD_PD_NOTIFICATION_IDX_POWER_STATE_TRANSITION),
};

static struct fwk_element rcar_pd_core_system_element_table[] = {
    [0] =
        {
            .name = "SYS0",
            .data = &((struct mod_rcar_pd_core_pd_config){
                .pd_type = RCAR_PD_TYPE_SYSTEM,
                .observer_id = FWK_ID_NONE_INIT,
            }),
        },
    [1] =
        {
            .name = "SYS1",
            .data = &((struct mod_rcar_pd_core_pd_config){
                .pd_type = RCAR_PD_TYPE_SYSTEM,
                .observer_id = FWK_ID_NONE_INIT,
            }),
        },
};

static const struct fwk_element *rcar_pd_core_get_element_table(fwk_id_t mod)
{
    struct fwk_element *element_table, *element;
    struct mod_rcar_pd_core_pd_config *pd_config_table, *pd_config;
    unsigned int core_idx;
    unsigned int cluster_idx = 0;
    unsigned int core_count;
    unsigned int cluster_count;
    unsigned int core_element_count = 0;

    core_count = CPU_CORE_MAX;
    cluster_count = CPU_CLUSTER_MAX;

    /*
     * Allocate element descriptors based on:
     *   Number of cores
     *   + Number of cluster descriptors
     *   + Number of system power domain descriptors
     *   + 1 terminator descriptor
     */
    element_table = fwk_mm_calloc(
        core_count + cluster_count +
            FWK_ARRAY_SIZE(rcar_pd_core_system_element_table) + 1,
        sizeof(struct fwk_element));
    if (element_table == NULL)
        return NULL;

    pd_config_table = fwk_mm_calloc(
        core_count + cluster_count, sizeof(struct mod_rcar_pd_core_pd_config));
    if (pd_config_table == NULL)
        return NULL;

    for (core_idx = 0; core_idx < core_count; core_idx++) {
        element = &element_table[core_element_count];
        pd_config = &pd_config_table[core_element_count];

        element->name = fwk_mm_alloc(CORE_NAME_SIZE, 1);
        if (element->name == NULL)
            return NULL;

        element->name = core_pd_name_table[core_idx];
        element->data = pd_config;

        pd_config->pd_type = RCAR_PD_TYPE_CORE;
        pd_config->cluster_id =
            FWK_ID_ELEMENT(FWK_MODULE_IDX_RCAR_PD_CORE, cluster_idx);
        pd_config->observer_id = FWK_ID_NONE;
        if (core_idx == RCAR_PD_CORE_ELEMENT_IDX_CPU0)
            pd_config->always_on = true;
        else
            pd_config->always_on = false;

        core_element_count++;
    }

    for (cluster_idx = 0; cluster_idx < cluster_count; cluster_idx++) {
        element = &element_table[core_element_count];
        pd_config = &pd_config_table[core_element_count];

        element->name = fwk_mm_alloc(CLUS_NAME_SIZE, 1);
        if (element->name == NULL)
            return NULL;

        element->name = cluster_pd_name_table[cluster_idx];
        element->data = pd_config;
        pd_config->pd_type = RCAR_PD_TYPE_CLUSTER;
        pd_config->observer_id = FWK_ID_NONE;
        pd_config->observer_api = FWK_ID_NONE;
        if (cluster_idx == RCAR_PD_CORE_ELEMENT_IDX_CLU0)
            pd_config->always_on = true;
        else
            pd_config->always_on = false;

        core_element_count++;
    }

    memcpy(
        &element_table[core_count + cluster_count],
        rcar_pd_core_system_element_table,
        sizeof(rcar_pd_core_system_element_table));

    /*
     * Configure pd_source_id with the SYSTOP identifier from the power domain
     * module which is dynamically defined based on the number of cores.
     */
    rcar_pd_core_config_data.pd_source_id = fwk_id_build_element_id(
        fwk_module_id_power_domain,
        core_count + CONFIG_POWER_DOMAIN_CHILD_CLUSTER0);

    return element_table;
}

/*
 * Power module configuration data
 */
const struct fwk_module_config config_rcar_pd_core = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(rcar_pd_core_get_element_table),
    .data = &rcar_pd_core_config_data,
};
