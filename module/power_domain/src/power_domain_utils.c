/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power domain utilities.
 */

#include <mod_power_domain.h>
#include <power_domain_utils.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include <stdio.h>

/* Maximum power domain name size including the null terminator */
#define PD_NAME_SIZE 25
/* Maximum number of cores */
#define MAX_CORES_COUNT 64
/* Maximum number of static table elements */
#define MAX_STATIC_ELEMENTS_COUNT 32

static int create_core_cluster_pd_element_table(
    struct fwk_element *element_table,
    unsigned int core_count,
    unsigned int cluster_count,
    unsigned int driver_idx,
    unsigned int api_idx,
    const uint32_t *core_state_table,
    size_t core_state_table_size,
    const uint32_t *cluster_state_table,
    size_t cluster_state_table_size)
{
    struct fwk_element *element;
    struct mod_power_domain_element_config *pd_config, *pd_config_table = NULL;
    unsigned int core_element_counter = 0;
    unsigned int core_idx;
    unsigned int cluster_idx;
    unsigned int cores_per_clusters = core_count / cluster_count;
    int status;

    pd_config_table = fwk_mm_calloc(
        core_count + cluster_count,
        sizeof(struct mod_power_domain_element_config));
    if (pd_config_table == NULL) {
        return FWK_E_NOMEM;
    }

    for (cluster_idx = 0; cluster_idx < cluster_count; cluster_idx++) {
        for (core_idx = 0; core_idx < cores_per_clusters; core_idx++) {
            element = &element_table[core_element_counter];
            pd_config = &pd_config_table[core_element_counter];
            element->name = fwk_mm_alloc(PD_NAME_SIZE, 1);
            status = snprintf(
                (char *)element->name,
                PD_NAME_SIZE,
                "CLUS%uCORE%u",
                cluster_idx,
                core_idx);
            if (status < 0) {
                return FWK_E_PANIC;
            }
            element->data = pd_config;
            pd_config->attributes.pd_type = MOD_PD_TYPE_CORE;
            pd_config->parent_idx = cluster_idx + core_count;
            pd_config->driver_id =
                FWK_ID_ELEMENT(driver_idx, core_element_counter);
            pd_config->api_id = FWK_ID_API(driver_idx, api_idx);
            pd_config->allowed_state_mask_table = core_state_table;
            pd_config->allowed_state_mask_table_size = core_state_table_size;
            core_element_counter++;
        }
        /* Define the cluster configuration */
        element = &element_table[cluster_idx + core_count];
        pd_config = &pd_config_table[cluster_idx + core_count];
        element->name = fwk_mm_alloc(PD_NAME_SIZE, 1);
        status = snprintf(
            (char *)element->name, PD_NAME_SIZE, "CLUS%u", cluster_idx);
        if (status < 0) {
            return FWK_E_PANIC;
        }
        element->data = pd_config;
        pd_config->attributes.pd_type = MOD_PD_TYPE_CLUSTER;
        pd_config->driver_id =
            FWK_ID_ELEMENT(driver_idx, cluster_idx + core_count);
        pd_config->api_id = FWK_ID_API(driver_idx, api_idx);
        pd_config->allowed_state_mask_table = cluster_state_table;
        pd_config->allowed_state_mask_table_size = cluster_state_table_size;
    }

    return FWK_SUCCESS;
}

const struct fwk_element *create_power_domain_element_table(
    unsigned int core_count,
    unsigned int cluster_count,
    unsigned int driver_idx,
    unsigned int api_idx,
    const uint32_t *core_state_table,
    size_t core_state_table_size,
    const uint32_t *cluster_state_table,
    size_t cluster_state_table_size,
    struct fwk_element *static_table,
    size_t static_table_size)
{
    struct fwk_element *element_table = NULL;
    struct mod_power_domain_element_config *pd_config;
    unsigned int systop_idx, element_idx;
    int status;

    if ((core_count % cluster_count != 0) || (core_count > MAX_CORES_COUNT) ||
        (static_table_size > MAX_STATIC_ELEMENTS_COUNT)) {
        return NULL;
    }

    element_table = fwk_mm_calloc(
        core_count + cluster_count + static_table_size + 1, /* Terminator */
        sizeof(struct fwk_element));

    if (element_table == NULL) {
        return element_table;
    }

    status = create_core_cluster_pd_element_table(
        element_table,
        core_count,
        cluster_count,
        driver_idx,
        api_idx,
        core_state_table,
        core_state_table_size,
        cluster_state_table,
        cluster_state_table_size);

    if (status != FWK_SUCCESS) {
        return NULL;
    }

    fwk_str_memcpy(
        element_table + (core_count + cluster_count),
        static_table,
        static_table_size * sizeof(struct fwk_element));

    /* Calculate SYSTOP index */
    systop_idx =
        (unsigned int)(core_count + cluster_count + static_table_size - 1);

    /* Set Power Domain Parent for all SYSTOP children */
    for (element_idx = core_count; element_idx < systop_idx; element_idx++) {
        pd_config =
            (struct mod_power_domain_element_config *)element_table[element_idx]
                .data;
        pd_config->parent_idx = systop_idx;
    }

    return element_table;
}
