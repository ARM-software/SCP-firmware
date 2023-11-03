/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
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

static int discover_mxp_nodes(struct cmn_cyprus_mxp_reg *mxp)
{
    int status;
    unsigned int node_count;
    unsigned int node_id;
    unsigned int node_idx;
    unsigned int node_pos_x;
    unsigned int node_pos_y;
    uint8_t mesh_size_x;
    uint8_t mesh_size_y;
    struct cmn_cyprus_node_cfg_reg *node;

    /* Get number of children connected to the cross point */
    node_count = get_child_count(mxp->CHILD_INFO);

    /* Get node id */
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

    /* Traverse nodes connected to the cross point */
    for (node_idx = 0; node_idx < node_count; node_idx++) {
        if (mxp_is_child_external(mxp, node_idx)) {
            /* Skip external nodes */
            continue;
        }

        /* Pointer to the child node */
        node =
            mxp_get_child_node(mxp, node_idx, shared_ctx->config->periphbase);

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

    FWK_LOG_INFO(MOD_NAME "Total HN-S nodes: %d", shared_ctx->hns_count);

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

static int store_hns_info(
    struct cmn_cyprus_node_cfg_reg *node,
    struct cmn_cyprus_mxp_reg *mxp)
{
    unsigned int ldid;
    unsigned int node_id;
    unsigned int node_pos_x;
    unsigned int node_pos_y;
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

    return FWK_SUCCESS;
}

static int cmn_cyprus_init_node_info(struct cmn_cyprus_mxp_reg *mxp)
{
    int status;
    unsigned int node_count;
    unsigned int node_idx;
    unsigned int node_type;
    const struct mod_cmn_cyprus_config *config;
    struct cmn_cyprus_node_cfg_reg *node;

    config = shared_ctx->config;

    /* Get the number of children connected to the cross point */
    node_count = get_child_count(mxp->CHILD_INFO);

    /* Traverse nodes */
    for (node_idx = 0; node_idx < node_count; node_idx++) {
        if (mxp_is_child_external(mxp, node_idx)) {
            /* Skip external nodes */
            continue;
        }

        /* Pointer to the child node */
        node = mxp_get_child_node(mxp, node_idx, config->periphbase);

        /* Get the node type identifier */
        node_type = node_info_get_type(node->NODE_INFO);

        if ((node_type == NODE_TYPE_HN_S) || (node_type == NODE_TYPE_HN_F)) {
            status = store_hns_info(node, mxp);
            if (status != FWK_SUCCESS) {
                return status;
            }
        }
    }

    return FWK_SUCCESS;
}

/* Traverse the mesh and initialize the module context data */
static int cmn_cyprus_init_ctx(void)
{
    int status;
    unsigned int mxp_count;
    unsigned int mxp_idx;
    struct cmn_cyprus_node_cfg_reg *node;
    struct cmn_cyprus_mxp_reg *mxp;

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
        shared_ctx->hns_count, sizeof(*shared_ctx->hns_info_table));

    /* Traverse the mesh and initialize context data */
    status = cmn_cyprus_init_ctx();
    if (status != FWK_SUCCESS) {
        return status;
    }

    FWK_LOG_INFO(MOD_NAME "CMN Discovery complete");

    return status;
}
