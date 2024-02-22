/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      NoC S3 Node Discovery Support.
 */

#include <noc_s3.h>
#include <noc_s3_discovery.h>
#include <noc_s3_reg.h>

#include <fwk_list.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * Helpers to extract Node ID and Node type from node_type register in the
 * header configuration.
 */
#define GET_NODE_TYPE(node_type) \
    ((node_type & NOC_S3_DOMAIN_NODE_TYPE_MSK) >> NOC_S3_DOMAIN_NODE_TYPE_POS)
#define GET_NODE_ID(node_type) \
    ((node_type & NOC_S3_DOMAIN_NODE_ID_MSK) >> NOC_S3_DOMAIN_NODE_ID_POS)

/*
 * The NoC S3 specification defines the following levels.
 * CFGNI->VD->PD->CD->Component
 *
 * This means while doing DFS, At MAX only 5 context needs to be stored because
 * the depth of the tree is limited to 5.
 */
#define NOC_S3_TREE_DEPTH 5U

/* List the stages in the discovery process. */
enum noc_s3_discovery_stage {
    /* NoC S3 Discovery stage to determine the row size. */
    NOC_S3_DISCOVERY_STAGE_DETERMINE_ROW_SIZE,
    /* NoC S3 Discovery stage to record offset for each node. */
    NOC_S3_DISCOVERY_STAGE_RECORD_OFFSET,
};

struct noc_s3_discovery_node_context {
    struct noc_s3_domain_cfg_hdr *domain_node;
    uint32_t children_processed;
};

/*
 * Helper to extract part ID from global configuration node's peripheral_id0 and
 * peripheral_id1 registers
 */
static uint16_t get_part_id(uint32_t peripheral_id0, uint32_t peripheral_id1)
{
    uint8_t part_num_l;
    uint8_t part_num_h;

    part_num_l = peripheral_id0 & NOC_S3_GLOBAL_CFG_PERIPHERAL_ID0_MASK;
    part_num_h = peripheral_id1 & NOC_S3_GLOBAL_CFG_PERIPHERAL_ID1_MASK;

    return ((uint16_t)part_num_h
            << NOC_S3_GLOBAL_CFG_PERIPHERAL_ID0_PART_NUM_WIDTH) |
        part_num_l;
}

/*
 * Check if the domain node is of type global cfgni. The discovery can only
 * start from the global cfgni node.
 */
static bool is_global_cfgni_node(uintptr_t periphbase)
{
    struct noc_s3_global_reg *reg;

    reg = (struct noc_s3_global_reg *)periphbase;
    return (GET_NODE_TYPE(reg->node_type) == MOD_NOC_S3_NODE_TYPE_GLOBAL_CFGNI);
}

/*
 * This driver only supports NoC S3. Check the part number to ensure that the
 * correct device is being initialized.
 */
static bool is_part_number_supported(uintptr_t periphbase)
{
    struct noc_s3_global_reg *reg;
    uint16_t part_id;

    reg = (struct noc_s3_global_reg *)periphbase;
    part_id = get_part_id(reg->peripheral_id0, reg->peripheral_id1);
    return part_id == NOC_S3_PART_NUMBER;
}

/*
 * The FMU node on NoC S3 does not have a node_type field. This type of node can
 * only be identified by reading part number register in the node's
 * configuration space.
 */
static bool is_fmu_node(struct mod_noc_s3_dev *dev, uint32_t offset)
{
    struct noc_s3_fmu_reg *reg;

    /*
     * NoC S3 permits 4KB or 64KB configuration nodes. FMU check requires
     * reading at 0xFFE0 offset which can only be done for 64KB config node
     * granularity mode.
     */
    if (dev->node_granularity == NOC_S3_4KB_CONFIG_NODES) {
        return false;
    }

    /*
     * Read the FMU part number to check if the node is FMU.
     */
    reg = (struct noc_s3_fmu_reg *)(dev->periphbase + offset);
    if (reg->fmu_errpidr0 != 0) {
        return true;
    }

    return false;
}

/*
 * NoC S3 permits 4KB and 64KB configuration nodes. This can be identified
 * during the runtime by reading the offset of the first node after global
 * configuration node. In this case, it is Voltage domain 0.
 */
static void determine_node_granularity(struct mod_noc_s3_dev *dev)
{
    struct noc_s3_global_reg *reg;

    /*
     * Compare the offset of the first voltage domain to check the size of
     * configuration node.
     */
    reg = (struct noc_s3_global_reg *)dev->periphbase;
    if (reg->vd_pointers == (64 * FWK_KIB)) {
        dev->node_granularity = NOC_S3_64KB_CONFIG_NODES;
    } else {
        dev->node_granularity = NOC_S3_4KB_CONFIG_NODES;
    }
}

/* Perform a series of check to validate the device. */
static int validate_device(struct mod_noc_s3_dev *dev)
{
    if (dev->periphbase == 0) {
        return FWK_E_PARAM;
    }

    /* Discovery starts from the CFGNI node. */
    if (!is_global_cfgni_node(dev->periphbase)) {
        return FWK_E_PARAM;
    }

    if (!is_part_number_supported(dev->periphbase)) {
        return FWK_E_SUPPORT;
    }

    return FWK_SUCCESS;
}

/*
 * This API populate the discovery data base on the stage of the discovery.
 */
static int fill_discovery_data(
    struct mod_noc_s3_dev *dev,
    struct noc_s3_discovery_data *discovery_data,
    struct noc_s3_domain_cfg_hdr *domain_node,
    enum noc_s3_discovery_stage stage)
{
    uint16_t node_id;
    uint16_t node_type;

    node_type = GET_NODE_TYPE(domain_node->node_type);
    node_id = GET_NODE_ID(domain_node->node_type);
    FWK_TRACE(MOD_NAME "Found Node Type: %d, Node ID: %d", node_type, node_id);
    switch (stage) {
    /*
     * For each node type, the Node IDs are numbered sequentially. That
     * means the maximum Node ID + 1 is the size of the row for each
     * node type.
     */
    case NOC_S3_DISCOVERY_STAGE_DETERMINE_ROW_SIZE:
        discovery_data->max_node_size[node_type] =
            FWK_MAX(discovery_data->max_node_size[node_type], node_id + 1);
        break;
    /* Record the node offset in the table */
    case NOC_S3_DISCOVERY_STAGE_RECORD_OFFSET:
        discovery_data->table[node_type][node_id] =
            (uintptr_t)domain_node - dev->periphbase;
        break;
    default:
        return FWK_E_PARAM;
        break;
    };

    return FWK_SUCCESS;
}

/*
 * This function performs a depth first walk of the node tree and can be
 * invoked to either count the type of each node found during the walk or to
 * record the offsets of the node in the discovery data table. The 'stage'
 * parameter can be used to choose the desired operation during the tree walk.
 */
static int discover_nodes(
    struct mod_noc_s3_dev *dev,
    struct noc_s3_discovery_data *discovery_data,
    enum noc_s3_discovery_stage stage)
{
    struct noc_s3_discovery_node_context node_context[NOC_S3_TREE_DEPTH] = {
        0
    };
    struct noc_s3_domain_cfg_hdr *domain_node;
    uint32_t *children_processed;
    uint32_t child_offset;
    uint32_t node_context_size;
    int status;

    domain_node = (struct noc_s3_domain_cfg_hdr *)dev->periphbase;

    /* Allocate the first node entry and push to the stack. */
    node_context_size = 0;
    node_context[node_context_size].domain_node = domain_node;
    node_context[node_context_size].children_processed = 0;
    node_context_size++;

    while (node_context_size > 0) {
        domain_node = node_context[node_context_size - 1].domain_node;
        children_processed =
            &node_context[node_context_size - 1].children_processed;

        /*
         * Only domains have children. Check if node type is component to
         * process.
         */
        if (!noc_s3_is_node_type_domain(
                GET_NODE_TYPE(domain_node->node_type)) ||
            /* Stop if all the children are processed. */
            *children_processed >= domain_node->child_node_info) {
            status =
                fill_discovery_data(dev, discovery_data, domain_node, stage);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(MOD_NAME "Discovery Data fill failed.");
                return status;
            }

            /* This element is processed. Continue with the parent. */
            node_context_size--;
            continue;
        }

        /*
         * Check if the last node was FMU node and all the children are
         * processed.
         */
        child_offset = domain_node->x_pointers[*children_processed];
        while ((*children_processed < domain_node->child_node_info) &&
               is_fmu_node(dev, child_offset)) {
            *children_processed += 1;
            child_offset = domain_node->x_pointers[*children_processed];
        }

        if (*children_processed >= domain_node->child_node_info) {
            continue;
        }

        /* Push the next child to the context. */
        if (node_context_size < NOC_S3_TREE_DEPTH) {
            *children_processed += 1;
            node_context_size += 1;
            node_context[node_context_size - 1].domain_node =
                (struct noc_s3_domain_cfg_hdr
                     *)(dev->periphbase + child_offset);
            node_context[node_context_size - 1].children_processed = 0;
        } else {
            /* Depth of more than 5 is not supported. */
            fwk_unexpected();
            return FWK_E_SUPPORT;
        }
    }

    return FWK_SUCCESS;
}

/* Allocate rows of nodes for each node type in the discovery table. */
static int allocate_nodes(struct noc_s3_discovery_data *discovery_data)
{
    enum mod_noc_s3_node_type node_type;
    uintptr_t *table_node_ptr;
    uint8_t max_node_size;

    for (node_type = MOD_NOC_S3_NODE_TYPE_GLOBAL_CFGNI;
         node_type < MOD_NOC_S3_NODE_TYPE_COUNT;
         node_type++) {
        max_node_size = discovery_data->max_node_size[node_type];
        if (max_node_size == 0) {
            /* Node is not present. No need for allocation. */
            continue;
        }

        /* Row size is +1 to the max node id because ids starts from 0. */
        table_node_ptr = fwk_mm_calloc(max_node_size, sizeof(uintptr_t));
        if (table_node_ptr == NULL) {
            return FWK_E_NOMEM;
        }

        discovery_data->table[node_type] = table_node_ptr;
    }

    return FWK_SUCCESS;
}

/*
 * The component node contains a list of the subfeatures it supports. This API
 * parses the list and records the offset of the target subfeature configuration
 * registers.
 */
int noc_s3_get_subfeature_offset(
    struct noc_s3_component_cfg_hdr *component_hdr,
    enum noc_s3_subfeature_type subfeature_type,
    uint32_t *ret_off_addr)
{
    uint32_t s_idx;

    if (!noc_s3_is_node_type_component(
            GET_NODE_TYPE(component_hdr->node_type))) {
        FWK_LOG_ERR(MOD_NAME "Invalid component header");
        return FWK_E_PARAM;
    }

    for (s_idx = 0; s_idx < component_hdr->num_subfeatures; s_idx++) {
        if (component_hdr->subfeature[s_idx].type == subfeature_type) {
            *ret_off_addr = component_hdr->subfeature[s_idx].pointer;
            return FWK_SUCCESS;
        }
    }

    FWK_LOG_ERR(
        MOD_NAME "Subfeature(%d) not supported by the Node[Type: %" PRId32
                 "][ID: %" PRId32 "]",
        subfeature_type,
        GET_NODE_TYPE(component_hdr->node_type),
        GET_NODE_ID(component_hdr->node_type));
    return FWK_E_SUPPORT;
}

/*
 * This API is designed to traverse a tree and convert the node data into a
 * table for easy and optimized access in O(1) time. The table is used to store
 * the offset of each node. During the discovery process, the tree is parsed
 * twice. The first time it is parsed, the maximum row size for all node types
 * is found, and during the second pass, the offset for each discovered node
 * type is recorded. The node IDs within the same types are linear, and the
 * maximum node ID can be used to determine the total number of nodes.
 *
 * The discovery will convert the following tree
 *                         Global CFGNI
 *                         /         \
 *                        /           \
 *                      VD0           VD1
 *                     /              /
 *                   PD0             PD1
 *                  /               /
 *                CD0              CD1
 *               /  \             /
 *              C0  C1           C2
 *
 *  To the following table containing node offset.
 *  _______________________________________________
 * |Node Type      | Index  |      Node Id         |
 * |---------------|--------|----------------------|
 * |               |        |   0   |   1   |  2   |
 * |---------------|--------|-------|-------|------|
 * |Global CFGNI   |   0    |CFGNI0 |       |      |
 * |Voltage Domain |   1    |VD0    |VD1    |      |
 * |Power Domain   |   2    |PD0    |PD1    |      |
 * |CLock Domain   |   3    |CD0    |CD1    |      |
 * |ASNI           |   4    |C0     |C1     |C2    |
 * |AMNI           |   5    |       |       |      |
 * |PMU            |   6    |       |       |      |
 * |HSNI           |   7    |       |       |      |
 * |HMNI           |   8    |       |       |      |
 * |PMNI           |   9    |       |       |      |
 * |_______________|________|_______|_______|______|
 *
 *  As the node id and type are predefined, the offset of the target node is
 *  fetch in O(1) by using id and type.
 *  Example: To get offset of the C2, the user will access the table using node
 *  type 4 and Id 2. Table[4][2] will give offset of C2
 */
int noc_s3_discovery(struct mod_noc_s3_dev *dev)
{
    struct noc_s3_discovery_data *discovery_data;
    int err;

    if (dev == NULL) {
        return FWK_E_PARAM;
    }

    err = validate_device(dev);
    if (err != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME " Device validation failed.");
        return err;
    }

    determine_node_granularity(dev);

    if (dev->discovery_completed) {
        return FWK_SUCCESS;
    }

    /*
     * Find the maximum node IDs for all the node types. This information is
     * used to allocate rows in the table.
     */
    discovery_data = &dev->discovery_data;
    err = discover_nodes(
        dev, discovery_data, NOC_S3_DISCOVERY_STAGE_DETERMINE_ROW_SIZE);
    if (err != FWK_SUCCESS) {
        return err;
    }

    /*
     * Allocate the rows to store node information for node types discovered
     * during the process.
     */
    err = allocate_nodes(discovery_data);
    if (err != FWK_SUCCESS) {
        return err;
    }

    /* Record in the offset value for each node. */
    err = discover_nodes(
        dev, discovery_data, NOC_S3_DISCOVERY_STAGE_RECORD_OFFSET);
    if (err != FWK_SUCCESS) {
        return err;
    }

    /* Update the device handler. */
    dev->discovery_completed = true;

    return err;
}
