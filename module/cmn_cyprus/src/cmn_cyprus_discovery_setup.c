/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for CMN-Cyprus Discovery.
 */

#include <internal/cmn_cyprus_cfg_reg.h>
#include <internal/cmn_cyprus_ctx.h>
#include <internal/cmn_cyprus_discovery_setup.h>
#include <internal/cmn_cyprus_mxp_reg.h>
#include <internal/cmn_cyprus_node_info_reg.h>
#include <internal/cmn_cyprus_reg.h>

#include <mod_cmn_cyprus.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Max Node Counts */
#define MAX_HNS_COUNT 128
#define MAX_RND_COUNT 40
#define MAX_RNI_COUNT 90
#define MAX_RNF_COUNT 256
#define MAX_CCG_COUNT 32

/* Max MXP port/device count */
#define MAX_MXP_PORT_COUNT   6
#define MAX_MXP_DEVICE_COUNT 4

/* Peripheral ID Revision Numbers */
#define PERIPH_ID_2_MASK    UINT64_C(0xFF)
#define PERIPH_ID_2_REV_POS 4

/* Node ID */
#define NODE_ID_Y_POS       3
#define NODE_ID_DEVICE_MASK (0x3)
#define NODE_ID_PORT_POS    2
#define NODE_ID_PORT_MASK   (0x1)

/* Child Info */
#define CHILD_INFO_COUNT UINT64_C(0x000000000000FFFF)

/* For MXP with 3 or 4 ports */
#define MULTI_PORTS_NODE_ID_PORT_POS  1
#define MULTI_PORTS_NODE_ID_PORT_MASK (0x3)

/*
 * Macro to get the Mask bits.
 * Mask bits are used to get the x and y coordinates from node position.
 */
#define GET_MASK_BITS(x) ((1 << x) - 1)

/* Shared module context pointer */
static struct cmn_cyprus_ctx *shared_ctx;

/* RNSAM table index */
static unsigned int rnsam_entry;

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
/* CMN Revision Numbers */
enum cmn_cyprus_revision {
    REV_R0_P0,
    REV_R1_P0,
    REV_R2_P0,
    REV_R3_P0,
    REV_COUNT,
};

/* CMN Cyprus revision names */
static const char *const cmn_cyprus_rev_to_name[REV_COUNT + 1] = {
    [REV_R0_P0] = "r0p0", [REV_R1_P0] = "r1p0",      [REV_R2_P0] = "r2p0",
    [REV_R3_P0] = "r3p0", [REV_COUNT] = "<Unknown>",
};

/* CMN Cyprus node type names */
static const char *const node_type_to_name[NODE_TYPE_COUNT] = {
    [NODE_TYPE_INVALID] = "<Invalid>",
    [NODE_TYPE_DVM] = "DVM",
    [NODE_TYPE_CFG] = "CFG",
    [NODE_TYPE_DTC] = "DTC",
    [NODE_TYPE_HN_I] = "HN-I",
    [NODE_TYPE_HN_F] = "HN-F",
    [NODE_TYPE_XP] = "XP",
    [NODE_TYPE_SBSX] = "SBSX",
    [NODE_TYPE_HN_F_MPAM_S] = "HN-F MPAM-S",
    [NODE_TYPE_HN_F_MPAM_NS] = "HN-F MPAM-NS",
    [NODE_TYPE_RN_I] = "RN-I",
    [NODE_TYPE_RN_D] = "RN-D",
    [NODE_TYPE_RN_SAM] = "RN-SAM",
    [NODE_TYPE_HN_P] = "HN-P",
    [NODE_TYPE_CCRA] = "CCRA",
    [NODE_TYPE_CCHA] = "CCHA",
    [NODE_TYPE_CCLA] = "CCLA",
    [NODE_TYPE_HN_S] = "HN-S",
    [NODE_TYPE_HN_S_MPAM_S] = "HN-S MPAM-S",
    [NODE_TYPE_HN_S_MPAM_NS] = "HN-S MPAM-NS",
};
#endif

/*
 * Determine the number of bits used to represent each node coordinate based
 * on the mesh size.
 */
static inline unsigned int get_encoding_bits(
    unsigned int mesh_size_x,
    unsigned int mesh_size_y)
{
    /*
     * If X and Y dimension are less than 4, encoding bits size will be 2.
     * If X and Y dimension are between 5 and 8, encoding bits size will be 3.
     */
    if ((mesh_size_x > 8) || (mesh_size_y > 8)) {
        return 4;
    } else if ((mesh_size_x > 4) || (mesh_size_y > 4)) {
        return 3;
    } else {
        return 2;
    }
}

/* Calculate 'x' coordinate of the node */
static unsigned int get_node_pos_x(
    unsigned int node_id,
    unsigned int mesh_size_x,
    unsigned int mesh_size_y)
{
    unsigned int encoding_bits;
    unsigned int mask_bits;

    encoding_bits = get_encoding_bits(mesh_size_x, mesh_size_y);
    mask_bits = GET_MASK_BITS(encoding_bits);

    return (node_id >> (NODE_ID_Y_POS + encoding_bits)) & mask_bits;
}

/* Calculate 'y' coordinate of the node */
static unsigned int get_node_pos_y(
    unsigned int node_id,
    unsigned int mesh_size_x,
    unsigned int mesh_size_y)
{
    unsigned int encoding_bits;
    unsigned int mask_bits;

    encoding_bits = get_encoding_bits(mesh_size_x, mesh_size_y);
    mask_bits = GET_MASK_BITS(encoding_bits);

    return (node_id >> NODE_ID_Y_POS) & mask_bits;
}

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
/* Get node type name */
static const char *get_node_type_name(enum cmn_cyprus_node_type node_type)
{
    if (node_type >= NODE_TYPE_COUNT) {
        node_type = NODE_TYPE_INVALID;
    }

    /* Check for undefined node types */
    if (strlen(node_type_to_name[node_type]) == 0) {
        node_type = NODE_TYPE_INVALID;
    }

    /* Invalid node type */
    return node_type_to_name[node_type];
}

/* Get CMN Cyprus product revision name */
static const char *get_cmn_cyprus_revision_name(
    struct cmn_cyprus_cfgm_reg *cfgm)
{
    unsigned int revision;

    /* Read the revision from the configuration register */
    revision = cfgm_get_cmn_revision(cfgm);

    if (revision > REV_COUNT) {
        revision = REV_COUNT;
    }

    return cmn_cyprus_rev_to_name[revision];
}
#endif

static inline unsigned int get_child_count(FWK_R uint64_t child_info)
{
    return (child_info & CHILD_INFO_COUNT);
}

/* Get port number to which the given node is connected */
static uint8_t get_port_number(unsigned int node_id, unsigned int mxp_port_cnt)
{
    if (mxp_port_cnt <= 2) {
        return (node_id >> NODE_ID_PORT_POS) & NODE_ID_PORT_MASK;
    } else {
        /* For port counts 3 and 4 */
        return (node_id >> MULTI_PORTS_NODE_ID_PORT_POS) &
            MULTI_PORTS_NODE_ID_PORT_MASK;
    }
}

/* Get the node device number */
static unsigned int get_device_number(unsigned int node_id)
{
    return (node_id & NODE_ID_DEVICE_MASK);
}

/*
 * Check if the given device type matches with the device connected to MXP port.
 */
static bool is_device_type(
    struct cmn_cyprus_mxp_reg *mxp,
    uint8_t port,
    enum cmn_cyprus_device_type device_type)
{
    enum cmn_cyprus_device_type port_device_type;

    /* Get device type of the node connected to the given port */
    port_device_type = mxp_get_device_type(mxp, port);

    return (port_device_type == device_type);
}

/* Check if an RN-F device is connected to given MXP port */
static bool is_device_type_rnf(struct cmn_cyprus_mxp_reg *mxp, uint8_t port)
{
    return (
        is_device_type(mxp, port, DEVICE_TYPE_RN_F_CHIF_ESAM) ||
        is_device_type(mxp, port, DEVICE_TYPE_RN_F_CHIE_ESAM) ||
        is_device_type(mxp, port, DEVICE_TYPE_RN_F_CHID_ESAM) ||
        is_device_type(mxp, port, DEVICE_TYPE_RN_F_CHIC_ESAM) ||
        is_device_type(mxp, port, DEVICE_TYPE_RN_F_CHIB_ESAM) ||
        is_device_type(mxp, port, DEVICE_TYPE_RN_F_CHIA_ESAM));
}

static int validate_node_count(
    unsigned int node_count,
    unsigned int max_node_count)
{
    if (node_count < max_node_count) {
        return FWK_SUCCESS;
    }

    FWK_LOG_ERR(MOD_NAME "  Error! Invalid node count");
    FWK_LOG_ERR(
        MOD_NAME "  count %d >= max limit (%d)", node_count, max_node_count);

    return FWK_E_DATA;
}

static int discover_node(
    struct cmn_cyprus_node_cfg_reg *node,
    struct cmn_cyprus_mxp_reg *mxp)
{
    int status;
    unsigned int node_id;
    enum cmn_cyprus_node_type node_type;
    uint8_t mxp_port;
    uint8_t mxp_port_count;

    /* Get node id */
    node_id = node_info_get_id(node->NODE_INFO);

    /* Get the number of device ports connected to the XP */
    mxp_port_count = mxp_get_device_port_count(mxp);

    /*
     * Get the port number in the cross point to which the node is
     * connected to.
     */
    mxp_port = get_port_number(node_id, mxp_port_count);

    /* Get node type */
    node_type = node_info_get_type(node->NODE_INFO);

    FWK_LOG_INFO(
        MOD_NAME "  P%u, %s ID:%d, LDID:%d",
        mxp_port,
        get_node_type_name(node_type),
        node_id,
        node_info_get_ldid(node->NODE_INFO));

    switch (node_type) {
    /*
     * HN-S node is HN-F node with LCN feature.  Any descriptions
     * related to the Fully coherent Home Node (HN-F) also apply to the
     * HN-S nodes. A CMN mesh will either have HN-F or HN-S nodes, hence
     * use the same variable to track the node count.
     */
    case NODE_TYPE_HN_F:
    case NODE_TYPE_HN_S:
        shared_ctx->hns_count++;
        status = validate_node_count(shared_ctx->hns_count, MAX_HNS_COUNT);
        break;

    case NODE_TYPE_RN_SAM:
        shared_ctx->rnsam_count++;
        /*
         * RN-F nodes don't have node type identifier and hence
         * the count cannot be determined using the node type
         * id. Alternatively, check if the device type connected
         * to the Crosspoint (XP) is one of the RNF types and
         * determine the RN-F count (if CAL connected RN-F,
         * double the count).
         */
        if (is_device_type_rnf(mxp, mxp_port)) {
            if (mxp_is_cal_connected(mxp, mxp_port)) {
                shared_ctx->rnf_count += 2;
                FWK_LOG_INFO(
                    MOD_NAME "  RN-F (CAL connected) found at P:%d", mxp_port);
            } else {
                shared_ctx->rnf_count++;
                FWK_LOG_INFO(MOD_NAME "  RN-F found at P:%d", mxp_port);
            }
        }
        status = validate_node_count(shared_ctx->rnf_count, MAX_RNF_COUNT);
        break;

    case NODE_TYPE_RN_D:
        shared_ctx->rnd_count++;
        status = validate_node_count(shared_ctx->rnd_count, MAX_RND_COUNT);
        break;

    case NODE_TYPE_RN_I:
        shared_ctx->rni_count++;
        status = validate_node_count(shared_ctx->rni_count, MAX_RNI_COUNT);
        break;

    case NODE_TYPE_CCRA:
        shared_ctx->ccg_ra_reg_count++;
        status =
            validate_node_count(shared_ctx->ccg_ra_reg_count, MAX_CCG_COUNT);
        break;

    case NODE_TYPE_CCHA:
        shared_ctx->ccg_ha_reg_count++;
        status =
            validate_node_count(shared_ctx->ccg_ha_reg_count, MAX_CCG_COUNT);
        break;

    case NODE_TYPE_CCLA:
        shared_ctx->ccla_reg_count++;
        status = validate_node_count(shared_ctx->ccla_reg_count, MAX_CCG_COUNT);
        break;

    default:
        status = FWK_SUCCESS;
        break;
    }

    return status;
}

/*
 * Check if the given HN-S or HN-S MPAM node must be isolated by comparing the
 * node base with the entries in the isolated HN-S table from config data.
 */
static bool is_node_base_isolated(uintptr_t node_base)
{
    unsigned int idx;
    struct isolated_hns_node_info *isolated_hns_table;

    isolated_hns_table = shared_ctx->config->isolated_hns_table;

    /* Iterate through the list of HN-S nodes to be isolated */
    for (idx = 0; idx < shared_ctx->config->isolated_hns_count; idx++) {
        if ((node_base == isolated_hns_table[idx].hns_base) ||
            (node_base == isolated_hns_table[idx].hns_mpam_s_base) ||
            (node_base == isolated_hns_table[idx].hns_mpam_ns_base)) {
            return true;
        }
    }

    return false;
}

/*
 * Check if the given node base address belongs to an isolated HN-S node from
 * the configuration data.
 */
static bool node_base_is_hns(uintptr_t node_base)
{
    unsigned int idx;
    struct isolated_hns_node_info *isolated_hns_table;

    isolated_hns_table = shared_ctx->config->isolated_hns_table;

    /* Iterate through the list of HN-S nodes to be isolated */
    for (idx = 0; idx < shared_ctx->config->isolated_hns_count; idx++) {
        if (node_base == isolated_hns_table[idx].hns_base) {
            return true;
        }
    }

    return false;
}

/* Check if the given node positions match */
static bool is_node_pos_equal(
    struct cmn_cyprus_node_pos *node1,
    struct cmn_cyprus_node_pos *node2)
{
    /* Check if X and Y positions match */
    if ((node1->pos_x == node2->pos_x) && (node1->pos_y == node2->pos_y)) {
        /* Check if port number and device number match */
        if ((node1->port_num == node2->port_num) &&
            (node1->device_num == node2->device_num)) {
            return true;
        }
    }

    return false;
}

/*
 * Check if the given HN-S node must be isolated by comparing the node position.
 */
static bool is_hns_pos_isolated(struct cmn_cyprus_node_pos *hns_pos)
{
    unsigned int idx;
    struct isolated_hns_node_info *isolated_hns_table;

    isolated_hns_table = shared_ctx->config->isolated_hns_table;

    for (idx = 0; idx < shared_ctx->config->isolated_hns_count; idx++) {
        if (is_node_pos_equal(hns_pos, &isolated_hns_table[idx].hns_pos)) {
            FWK_LOG_ERR(
                MOD_NAME "  P%u, D%u, Isolated HN-S",
                hns_pos->port_num,
                hns_pos->device_num);
            return true;
        }
    }
    return false;
}

/*
 * Disable MXP device isolation for HN-F/S device type.
 */
static void disable_hns_isolation(struct cmn_cyprus_mxp_reg *mxp)
{
    enum cmn_cyprus_device_type port_device_type;
    unsigned int hns_node_id;
    uint8_t device_num;
    uint8_t mesh_size_x;
    uint8_t mesh_size_y;
    uint8_t port_num;
    struct cmn_cyprus_node_pos hns_pos;

    mesh_size_x = shared_ctx->config->mesh_size_x;
    mesh_size_y = shared_ctx->config->mesh_size_y;

    /* Iterate through each port in the MXP */
    for (port_num = 0; port_num < MAX_MXP_PORT_COUNT; port_num++) {
        port_device_type = mxp_get_device_type(mxp, port_num);
        if ((port_device_type != DEVICE_TYPE_HN_F) &&
            (port_device_type != DEVICE_TYPE_HN_S)) {
            continue;
        }

        hns_node_id = node_info_get_id(mxp->NODE_INFO);

        /* Initialize node position of the HN-S node */
        hns_pos.pos_x = get_node_pos_x(hns_node_id, mesh_size_x, mesh_size_y);
        hns_pos.pos_y = get_node_pos_y(hns_node_id, mesh_size_x, mesh_size_y);
        hns_pos.port_num = port_num;

        /* Iterate through each device in the XP port */
        for (device_num = 0; device_num < MAX_MXP_DEVICE_COUNT; device_num++) {
            hns_pos.device_num = device_num;

            if (is_hns_pos_isolated(&hns_pos)) {
                /* Skip HN-S nodes that ought to be isolated */
                continue;
            }

            /* Disable HN-S isolation for the hns node */
            mxp_enable_device(mxp, port_num, device_num);
        }
    }
}

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
static void print_node_info(struct cmn_cyprus_mxp_reg *mxp)
{
    uint8_t mesh_size_x;
    uint8_t mesh_size_y;
    unsigned int node_id;
    unsigned int node_pos_x;
    unsigned int node_pos_y;

    node_id = node_info_get_id(mxp->NODE_INFO);

    mesh_size_x = shared_ctx->config->mesh_size_x;
    mesh_size_y = shared_ctx->config->mesh_size_y;

    node_pos_x = get_node_pos_x(node_id, mesh_size_x, mesh_size_y);
    node_pos_y = get_node_pos_y(node_id, mesh_size_x, mesh_size_y);

    FWK_LOG_INFO(
        MOD_NAME "XP (%d, %d) ID:%d, LDID:%d",
        node_pos_x,
        node_pos_y,
        node_id,
        node_info_get_ldid(mxp->NODE_INFO));
}
#endif

static int discover_mxp_nodes(struct cmn_cyprus_mxp_reg *mxp)
{
    int status;
    unsigned int node_count;
    unsigned int node_idx;
    struct cmn_cyprus_node_cfg_reg *node;

    /* Get number of children connected to the cross point */
    node_count = get_child_count(mxp->CHILD_INFO);

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
    print_node_info(mxp);
#endif

    /* Traverse nodes connected to the cross point */
    for (node_idx = 0; node_idx < node_count; node_idx++) {
        if (mxp_is_child_external(mxp, node_idx)) {
            /* Skip external nodes */
            continue;
        }

        /* Pointer to the child node */
        node =
            mxp_get_child_node(mxp, node_idx, shared_ctx->config->periphbase);

        /* Check if the node must be skipped due to HN-S isolation */
        if (is_node_base_isolated((uintptr_t)node)) {
            if (node_base_is_hns((uintptr_t)node)) {
                shared_ctx->isolated_hns_count++;
            }
            continue;
        }

        status = discover_node(node, mxp);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

/*
 * Discover the topology of the interconnect.
 */
static int discover_mesh_topology(void)
{
    int status;
    enum cmn_cyprus_node_type node_type;
    uint8_t mxp_count;
    uint8_t mxp_idx;
    struct cmn_cyprus_mxp_reg *mxp;
    struct cmn_cyprus_node_cfg_reg *node;

    FWK_LOG_INFO(
        MOD_NAME "CMN-CYPRUS revision: %s",
        get_cmn_cyprus_revision_name(shared_ctx->cfgm));
    FWK_LOG_DEBUG(
        MOD_NAME "Rootnode Base address: %p", (void *)shared_ctx->cfgm);

    /* Get number of cross points in the mesh */
    mxp_count = get_child_count(shared_ctx->cfgm->CHILD_INFO);

    /* Traverse cross points */
    for (mxp_idx = 0; mxp_idx < mxp_count; mxp_idx++) {
        node = cfgm_get_child_node(shared_ctx->cfgm, mxp_idx);
        node_type = node_info_get_type(node->NODE_INFO);

        if (node_type != NODE_TYPE_XP) {
            return FWK_E_DATA;
        }

        mxp = (struct cmn_cyprus_mxp_reg *)node;

        disable_hns_isolation(mxp);

        /* Discover the nodes connected to the crosspoint */
        status = discover_mxp_nodes(mxp);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

static void print_mesh_node_count(void)
{
    FWK_LOG_INFO(MOD_NAME "Total RN-SAM nodes: %d", shared_ctx->rnsam_count);

    FWK_LOG_INFO(
        MOD_NAME "Total HN-S nodes: %d",
        (shared_ctx->hns_count + shared_ctx->isolated_hns_count));
    FWK_LOG_INFO(
        MOD_NAME "Isolated HN-S nodes: %d",
        shared_ctx->config->isolated_hns_count);

    FWK_LOG_INFO(MOD_NAME "Total RN-D nodes: %d", shared_ctx->rnd_count);

    FWK_LOG_INFO(MOD_NAME "Total RN-F nodes: %d", shared_ctx->rnf_count);

    FWK_LOG_INFO(MOD_NAME "Total RN-I nodes: %d", shared_ctx->rni_count);

    FWK_LOG_INFO(
        MOD_NAME "Total CCG Request Agent nodes: %d",
        shared_ctx->ccg_ra_reg_count);

    FWK_LOG_INFO(
        MOD_NAME "Total CCG Home Agent nodes: %d",
        shared_ctx->ccg_ha_reg_count);

    FWK_LOG_INFO(
        MOD_NAME "Total CCG Link Agent nodes: %d", shared_ctx->ccla_reg_count);
}

static bool is_node_inside_rect(
    struct cmn_cyprus_node_pos *node,
    const struct cmn_cyprus_node_pos *rect_start,
    const struct cmn_cyprus_node_pos *rect_end)
{
    return (
        (node->pos_x >= rect_start->pos_x) &&
        (node->pos_y >= rect_start->pos_y) &&
        (node->pos_x <= rect_end->pos_x) && (node->pos_y <= rect_end->pos_y) &&
        (node->port_num <= rect_end->port_num));
}

/* Helper function to check if hns is inside the SCG/HTG square/rectangle */
static bool is_hns_inside_rect(
    struct cmn_cyprus_node_pos *hns_node_pos,
    const struct mod_cmn_cyprus_mem_region_map *region)
{
    const struct cmn_cyprus_node_pos *rect_start;
    const struct cmn_cyprus_node_pos *rect_end;
    bool is_inside_rect;

    is_inside_rect = false;
    rect_start = &region->hns_pos_start;
    rect_end = &region->hns_pos_end;

    if (is_node_inside_rect(hns_node_pos, rect_start, rect_end)) {
        if (hns_node_pos->pos_y == rect_start->pos_y) {
            is_inside_rect = (hns_node_pos->port_num >= rect_start->port_num);
            return is_inside_rect;
        } else if (hns_node_pos->pos_y == rect_end->pos_y) {
            is_inside_rect = (hns_node_pos->port_num <= rect_end->port_num);
            return is_inside_rect;
        }

        is_inside_rect = true;
    }

    return is_inside_rect;
}

/* Helper function to return the SCG index of the given HN-S node */
static int get_hns_node_scg_idx(
    struct cmn_cyprus_node_pos *hns_node_pos,
    uint8_t *scg_idx)
{
    uint8_t scg_region_idx;
    unsigned int region_idx;
    const struct mod_cmn_cyprus_mem_region_map *region;

    scg_region_idx = 0;

    /* Get the SCG index for all the HN-S nodes in the mesh */
    for (region_idx = 0; region_idx < shared_ctx->config->mmap_count;
         region_idx++) {
        region = &shared_ctx->config->mmap_table[region_idx];

        if (region->type != MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE) {
            continue;
        }

        /* Check if the given HN-S node is a part of the SCG */
        if (is_hns_inside_rect(hns_node_pos, region)) {
            *scg_idx = scg_region_idx;
            return FWK_SUCCESS;
        }
        scg_region_idx++;
    }

    /* Return error if the given HN-F node is not a part of any SCG region */
    return FWK_E_DATA;
}

static int store_hns_info(
    struct cmn_cyprus_node_cfg_reg *node,
    struct cmn_cyprus_mxp_reg *mxp)
{
    int status;
    unsigned int ldid;
    unsigned int node_id;
    unsigned int node_pos_x;
    unsigned int node_pos_y;
    uint8_t scg_idx;
    uint8_t mesh_size_x;
    uint8_t mesh_size_y;
    uint8_t mxp_port;
    uint8_t mxp_port_count;
    struct cmn_cyprus_hns_info *hns_info;

    ldid = node_info_get_ldid(node->NODE_INFO);

    if (ldid >= shared_ctx->hns_count) {
        return FWK_E_RANGE;
    }

    hns_info = &shared_ctx->hns_info_table[ldid];

    /*
     * Save the node info in the HN-S info table.
     */
    hns_info->hns = (struct cmn_cyprus_hns_reg *)node;

    node_id = node_info_get_id(node->NODE_INFO);
    hns_info->node_pos.device_num = get_device_number(node_id);

    hns_info->node_id = node_id;

    mesh_size_x = shared_ctx->config->mesh_size_x;
    mesh_size_y = shared_ctx->config->mesh_size_y;

    node_pos_x = get_node_pos_x(node_id, mesh_size_x, mesh_size_y);
    hns_info->node_pos.pos_x = node_pos_x;

    node_pos_y = get_node_pos_y(node_id, mesh_size_x, mesh_size_y);
    hns_info->node_pos.pos_y = node_pos_y;

    hns_info->mxp = mxp;

    mxp_port_count = mxp_get_device_port_count(mxp);

    /*
     * Get the port number in the cross point to which the node is
     * connected to.
     */
    mxp_port = get_port_number(node_id, mxp_port_count);
    hns_info->node_pos.port_num = mxp_port;

    status = get_hns_node_scg_idx(&hns_info->node_pos, &scg_idx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Save SCG idx that this HN-F node belongs to */
    hns_info->scg_idx = scg_idx;

    /* Increment the HN-S count in the SCG */
    shared_ctx->scg_hns_count[scg_idx]++;

    return FWK_SUCCESS;
}

static int cmn_cyprus_init_node_info(struct cmn_cyprus_mxp_reg *mxp)
{
    int status;
    unsigned int ldid;
    unsigned int node_count;
    unsigned int node_idx;
    unsigned int node_type;
    const struct mod_cmn_cyprus_config *config;
    struct cmn_cyprus_node_cfg_reg *node;

    config = shared_ctx->config;

    /* Get the number of children connected to the cross point */
    node_count = get_child_count(mxp->CHILD_INFO);

    status = FWK_SUCCESS;

    /* Traverse nodes */
    for (node_idx = 0; node_idx < node_count; node_idx++) {
        if (mxp_is_child_external(mxp, node_idx)) {
            /* Skip external nodes */
            continue;
        }

        /* Pointer to the child node */
        node = mxp_get_child_node(mxp, node_idx, config->periphbase);

        if (is_node_base_isolated((uintptr_t)node)) {
            /* Skip isolated HN-S nodes */
            continue;
        }

        ldid = node_info_get_ldid(node->NODE_INFO);

        /* Get the node type identifier */
        node_type = node_info_get_type(node->NODE_INFO);

        switch (node_type) {
        case NODE_TYPE_HN_F:
        case NODE_TYPE_HN_S:
            status = store_hns_info(node, mxp);
            break;

        case NODE_TYPE_RN_SAM:
            /* Save the node pointer in the RNSAM table */
            shared_ctx->rnsam_table[rnsam_entry] =
                (struct cmn_cyprus_rnsam_reg *)node;

            rnsam_entry++;
            break;

        case NODE_TYPE_CCRA:
            shared_ctx->ccg_ra_info_table[ldid].node_id =
                node_info_get_id(node->NODE_INFO);
            shared_ctx->ccg_ra_info_table[ldid].ccg_ra =
                (struct cmn_cyprus_ccg_ra_reg *)node;
            break;

        case NODE_TYPE_CCHA:
            shared_ctx->ccg_ha_info_table[ldid].node_id =
                node_info_get_id(node->NODE_INFO);
            shared_ctx->ccg_ha_info_table[ldid].ccg_ha =
                (struct cmn_cyprus_ccg_ha_reg *)node;
            break;

        case NODE_TYPE_CCLA:
            shared_ctx->ccla_info_table[ldid].node_id =
                node_info_get_id(node->NODE_INFO);
            shared_ctx->ccla_info_table[ldid].ccla =
                (struct cmn_cyprus_ccla_reg *)node;
            break;

        default:
            /* Do Nothing */
            break;
        };

        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return status;
}

/* Traverse the mesh and initialize the module context data */
static int cmn_cyprus_init_ctx(void)
{
    int status;
    unsigned int mxp_count;
    unsigned int mxp_idx;
    struct cmn_cyprus_node_cfg_reg *node;
    struct cmn_cyprus_mxp_reg *mxp;

    rnsam_entry = 0;

    /* Get number of cross points in the mesh */
    mxp_count = get_child_count(shared_ctx->cfgm->CHILD_INFO);

    /* Traverse cross points */
    for (mxp_idx = 0; mxp_idx < mxp_count; mxp_idx++) {
        node = cfgm_get_child_node(shared_ctx->cfgm, mxp_idx);
        mxp = (struct cmn_cyprus_mxp_reg *)node;

        /* Traverse the nodes connected to the MXP and init node info */
        status = cmn_cyprus_init_node_info(mxp);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

int cmn_cyprus_discovery(struct cmn_cyprus_ctx *ctx)
{
    int status;

    fwk_assert(ctx != NULL);

    shared_ctx = ctx;

    FWK_LOG_INFO(MOD_NAME "CMN Discovery start...");

    /* Traverse the mesh and discover the topology */
    status = discover_mesh_topology();
    if (status != FWK_SUCCESS) {
        return status;
    }

    print_mesh_node_count();

    /*
     * Allocate resources based on the discovery.
     */

    /* HN-S Info table */
    shared_ctx->hns_info_table = fwk_mm_calloc(
        (shared_ctx->hns_count + shared_ctx->isolated_hns_count),
        sizeof(*shared_ctx->hns_info_table));

    /* RN-SAM table */
    shared_ctx->rnsam_table = fwk_mm_calloc(
        shared_ctx->rnsam_count, sizeof(*shared_ctx->rnsam_table));

    /* Allocate resources for the CCG node descriptors in the context */
    if (shared_ctx->ccg_ra_reg_count != 0) {
        shared_ctx->ccg_ra_info_table = fwk_mm_calloc(
            shared_ctx->ccg_ra_reg_count,
            sizeof(*shared_ctx->ccg_ra_info_table));
    }

    if (shared_ctx->ccg_ha_reg_count != 0) {
        shared_ctx->ccg_ha_info_table = fwk_mm_calloc(
            shared_ctx->ccg_ha_reg_count,
            sizeof(*shared_ctx->ccg_ha_info_table));
    }

    if (shared_ctx->ccla_reg_count != 0) {
        shared_ctx->ccla_info_table = fwk_mm_calloc(
            shared_ctx->ccla_reg_count, sizeof(*shared_ctx->ccla_info_table));
    }

    /* Traverse the mesh and initialize context data */
    status = cmn_cyprus_init_ctx();
    if (status != FWK_SUCCESS) {
        return status;
    }

    FWK_LOG_INFO(MOD_NAME "CMN Discovery complete");

    return status;
}
