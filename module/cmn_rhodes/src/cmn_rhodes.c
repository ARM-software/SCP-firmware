/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cmn_rhodes.h>

#include <fwk_assert.h>
#include <fwk_math.h>

/*
 * Encoding bits size of the X and Y position in the Node info value.
 * If X and Y dimension are less than 4, encoding bits size will be 2.
 * If X and Y dimension are between 5 and 8, encoding bits size will be 3.
 */
static unsigned int encoding_bits;
static unsigned int mask_bits;

unsigned int get_node_child_count(void *node_base)
{
    struct node_header *node = node_base;
    return node->CHILD_INFO & CMN_RHODES_CHILD_INFO_COUNT;
}

enum node_type get_node_type(void *node_base)
{
    struct node_header *node = node_base;
    return (enum node_type)(node->NODE_INFO & CMN_RHODES_NODE_INFO_TYPE);
}

unsigned int get_node_id(void *node_base)
{
    struct node_header *node = node_base;
    return (node->NODE_INFO & CMN_RHODES_NODE_INFO_ID) >>
           CMN_RHODES_NODE_INFO_ID_POS;
}

unsigned int get_node_logical_id(void *node_base)
{
    struct node_header *node = node_base;
    return (node->NODE_INFO & CMN_RHODES_NODE_INFO_LOGICAL_ID) >>
           CMN_RHODES_NODE_INFO_LOGICAL_ID_POS;
}

void *get_child_node(uintptr_t base, void *node_base, unsigned int child_index)
{
    struct node_header *node = node_base;
    uint32_t child_pointer;
    unsigned int offset;
    void *child_node;

    child_pointer = node->CHILD_POINTER[child_index];
    offset = child_pointer & CMN_RHODES_CHILD_POINTER_OFFSET;

    child_node = (void *)(base + offset);
    return child_node;
}

unsigned int get_child_node_id(void *node_base,
    unsigned int child_index)
{
    struct node_header *node = node_base;
    uint32_t node_pointer;
    unsigned int node_id;

    node_pointer = (node->CHILD_POINTER[child_index] &
                    CMN_RHODES_CHILD_POINTER_EXT_NODE_POINTER) >>
                   CMN_RHODES_CHILD_POINTER_EXT_NODE_POINTER_POS;

    /*
     * For mesh widths using 2 bits each for X,Y encoding:
     * NodeID[1:0] = DeviceID[3:2]
     * NodeID[2]   = DeviceID[0]
     * NodeID[4:3] = NODE POINTER[7:6]
     * NodeID[6:5] = NODE POINTER[9:8]
     *
     * For mesh widths using 3 bits each for X,Y encoding:
     * NodeID[1:0] = DeviceID[3:2]
     * NodeID[2]   = DeviceID[0]
     * NodeID[5:3] = NODE POINTER[8:6]
     * NodeID[8:6] = NODE POINTER[11:9]
     */
    node_id = (((node_pointer >> 6) & 0xff) << 3) |
              ((node_pointer & 0x1) << 2) |
              ((node_pointer >> 2) & 0x3);

    return node_id;
}

bool is_child_external(void *node_base, unsigned int child_index)
{
    struct node_header *node = node_base;
    return !!(node->CHILD_POINTER[child_index] & (1U << 31));
}

bool get_port_number(unsigned int child_node_id)
{
    return (child_node_id >> CMN_RHODES_NODE_ID_PORT_POS) &
           CMN_RHODES_NODE_ID_PORT_MASK;
}

unsigned int get_device_type(void *mxp_base, bool port)
{
    struct cmn_rhodes_mxp_reg *mxp = mxp_base;
    return mxp->PORT_CONNECT_INFO[port] &
           CMN_RHODES_MXP_PORT_CONNECT_INFO_DEVICE_TYPE_MASK;
}

uint64_t sam_encode_region_size(uint64_t size)
{
    uint64_t blocks;
    uint64_t result;

    /* Size must be a multiple of SAM_GRANULARITY */
    fwk_assert((size % SAM_GRANULARITY) == 0);

    blocks = size / SAM_GRANULARITY;
    result = fwk_math_log2(blocks);

    return result;
}

void configure_region(volatile uint64_t *reg, uint64_t base, uint64_t size,
    enum sam_node_type node_type)
{
    uint64_t value;

    fwk_assert(reg);
    fwk_assert((base % size) == 0);

    value = CMN_RHODES_RNSAM_REGION_ENTRY_VALID;
    value |= node_type << CMN_RHODES_RNSAM_REGION_ENTRY_TYPE_POS;
    value |= sam_encode_region_size(size) <<
             CMN_RHODES_RNSAM_REGION_ENTRY_SIZE_POS;
    value |= (base / SAM_GRANULARITY) << CMN_RHODES_RNSAM_REGION_ENTRY_BASE_POS;

    *reg = value;
}

static const char * const type_to_name[] = {
    [NODE_TYPE_INVALID] = "<Invalid>",
    [NODE_TYPE_DVM]     = "DVM",
    [NODE_TYPE_CFG]     = "CFG",
    [NODE_TYPE_DTC]     = "DTC",
    [NODE_TYPE_HN_I]    = "HN-I",
    [NODE_TYPE_HN_F]    = "HN-F",
    [NODE_TYPE_XP]      = "XP",
    [NODE_TYPE_SBSX]    = "SBSX",
    [NODE_TYPE_RN_I]    = "RN-I",
    [NODE_TYPE_RN_D]    = "RN-D",
    [NODE_TYPE_RN_SAM]  = "RN-SAM",
};

static const char * const type_to_name_cml[] = {
    [NODE_TYPE_CXRA - NODE_TYPE_CML_BASE] = "CXRA",
    [NODE_TYPE_CXHA - NODE_TYPE_CML_BASE] = "CXHA",
    [NODE_TYPE_CXLA - NODE_TYPE_CML_BASE] = "CXLA",

};

const char *get_node_type_name(enum node_type node_type)
{
    /* Base node IDs */
    if (node_type <= NODE_TYPE_RN_SAM)
        return type_to_name[node_type];

    /* CML node IDs */
    if ((node_type >= NODE_TYPE_CML_BASE) &&
        (node_type <= NODE_TYPE_CXLA))
        return type_to_name_cml[node_type - NODE_TYPE_CML_BASE];

    /* Invalid node IDs */
    return type_to_name[NODE_TYPE_INVALID];
}

unsigned int get_node_pos_x(void *node_base)
{
    struct node_header *node = node_base;
    return (get_node_id(node) >> (CMN_RHODES_NODE_ID_Y_POS + encoding_bits)) &
           mask_bits;
}

unsigned int get_node_pos_y(void *node_base)
{
    struct node_header *node = node_base;
    return (get_node_id(node) >> CMN_RHODES_NODE_ID_Y_POS) & mask_bits;
}

struct cmn_rhodes_cfgm_reg *get_root_node(uintptr_t base,
    unsigned int hnd_node_id, unsigned int mesh_size_x,
    unsigned int mesh_size_y)
{
    unsigned int node_pos_x;
    unsigned int node_pos_y;
    unsigned int node_port;
    uintptr_t offset;

    /*
     * Determine the number of bits used to represent each node coordinate based
     * on the mesh size as per CMN_RHODES specification.
     */
    encoding_bits = ((mesh_size_x > 4) || (mesh_size_y > 4)) ? 3 : 2;

    /* Extract node coordinates from the node identifier */
    mask_bits = (1 << encoding_bits) - 1;
    node_pos_y = (hnd_node_id >> CMN_RHODES_NODE_ID_Y_POS) & mask_bits;
    node_pos_x = (hnd_node_id >> (CMN_RHODES_NODE_ID_Y_POS + encoding_bits)) &
                 mask_bits;
    node_port = (hnd_node_id >> CMN_RHODES_NODE_ID_PORT_POS) &
                CMN_RHODES_NODE_ID_PORT_MASK;

    /* Calculate node address offset */
    offset = (node_pos_y << CMN_RHODES_ROOT_NODE_OFFSET_Y_POS) |
             (node_pos_x << (CMN_RHODES_ROOT_NODE_OFFSET_Y_POS +
                             encoding_bits)) |
             (node_port << CMN_RHODES_ROOT_NODE_OFFSET_PORT_POS);

    return (struct cmn_rhodes_cfgm_reg *)(base + offset);
}
