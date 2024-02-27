/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <noc_s3.h>
#include <noc_s3_discovery.h>
#include <noc_s3_reg.h>

#include <mod_noc_s3.h>

#include <fwk_log.h>
#include <fwk_status.h>

#include <inttypes.h>

/* Check if the node_type is of node_type domain. */
bool noc_s3_is_node_type_domain(enum mod_noc_s3_node_type node_type)
{
    bool is_domain;

    switch (node_type) {
    case MOD_NOC_S3_NODE_TYPE_GLOBAL_CFGNI:
    case MOD_NOC_S3_NODE_TYPE_VD:
    case MOD_NOC_S3_NODE_TYPE_PD:
    case MOD_NOC_S3_NODE_TYPE_CD:
        is_domain = true;
        break;
    default:
        is_domain = false;
        break;
    };

    return is_domain;
}

/* Check if the node_type is of node_type component. */
bool noc_s3_is_node_type_component(enum mod_noc_s3_node_type node_type)
{
    bool is_component;

    switch (node_type) {
    case MOD_NOC_S3_NODE_TYPE_ASNI:
    case MOD_NOC_S3_NODE_TYPE_AMNI:
    case MOD_NOC_S3_NODE_TYPE_PMU:
    case MOD_NOC_S3_NODE_TYPE_HSNI:
    case MOD_NOC_S3_NODE_TYPE_HMNI:
    case MOD_NOC_S3_NODE_TYPE_PMNI:
        is_component = true;
        break;
    default:
        is_component = false;
        break;
    };

    return is_component;
}

/* Check if the subfeature type is correct.  */
bool noc_s3_is_type_subfeature(enum noc_s3_subfeature_type type)
{
    bool is_subfeature;

    switch (type) {
    case NOC_S3_NODE_TYPE_APU:
    case NOC_S3_NODE_TYPE_PSAM:
    case NOC_S3_NODE_TYPE_FCU:
    case NOC_S3_NODE_TYPE_IDM:
        is_subfeature = true;
        break;
    default:
        is_subfeature = false;
        break;
    };

    return is_subfeature;
}

/*
 * Check if the incoming address range is overlapping with the mapped address.
 */
bool noc_s3_check_overlap(
    uint64_t start_a,
    uint64_t end_a,
    uint64_t start_b,
    uint64_t end_b)
{
    return ((start_a <= end_b) && (start_b <= end_a));
}

/*
 * Compute the peripheral base address of a subfeature by taking the component
 * offset from the discovery table, parsing the component to obtain the
 * subfeature offset, and then adding the subfeature offset to the peripheral
 * base.
 */
int noc_s3_get_subfeature_address(
    struct mod_noc_s3_dev *dev,
    enum mod_noc_s3_node_type node_type,
    enum noc_s3_subfeature_type subfeature_type,
    uint32_t node_id,
    uintptr_t *subfeature_address)
{
    struct noc_s3_component_cfg_hdr *component_hdr;
    struct noc_s3_discovery_data *discovery_data;
    uint32_t component_offset;
    uint32_t subfeature_offset;
    int status;

    /*
     * Find the target xSNI node and get register base address for system
     * address map subfeature.
     */
    discovery_data = &dev->discovery_data;
    if (discovery_data == NULL) {
        FWK_LOG_ERR(MOD_NAME "Invalid discovery table.");
        return FWK_E_PARAM;
    }

    /* Make sure that the node type is either domain or component. */
    if (!noc_s3_is_node_type_domain(node_type) &&
        !noc_s3_is_node_type_component(node_type)) {
        return FWK_E_PARAM;
    }

    if (!noc_s3_is_type_subfeature(subfeature_type)) {
        return FWK_E_PARAM;
    }

    /*
     * Make sure that the id is not more than the maximum known id. Maximum
     * known id is 1 less than maximum row size.
     */
    if (node_id >= discovery_data->max_node_size[node_type]) {
        FWK_LOG_ERR(
            MOD_NAME "Node ID out of range. Input ID: %" PRId32 ", Max ID %d.",
            node_id,
            discovery_data->max_node_size[node_type]);
        return FWK_E_PARAM;
    }

    if (subfeature_address == NULL) {
        return FWK_E_PARAM;
    }

    /* Get the component offset from the discovery table. */
    component_offset = discovery_data->table[node_type][node_id];
    if (component_offset == 0) {
        FWK_LOG_ERR(
            MOD_NAME "Node(%d) ID:%" PRId32 " Invalid offset.",
            node_type,
            node_id);
        return FWK_E_PARAM;
    }
    component_hdr =
        (struct noc_s3_component_cfg_hdr *)(dev->periphbase + component_offset);

    /* Parse the component header and get offset of the subfeature. */
    status = noc_s3_get_subfeature_offset(
        component_hdr, subfeature_type, &subfeature_offset);
    if (status != FWK_SUCCESS) {
        return status;
    }
    *subfeature_address = dev->periphbase + subfeature_offset;

    return FWK_SUCCESS;
}
