/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for accessing MXP node.
 */

#include <internal/cmn_cyprus_mxp_reg.h>
#include <internal/cmn_cyprus_reg.h>

#include <stdbool.h>
#include <stdint.h>

/* MXP Port Connect Info */
#define MXP_PORT_CONNECT_INFO_DEVICE_TYPE_MASK   UINT64_C(0x3F)
#define MXP_PORT_CONNECT_INFO_CAL_CONNECTED_MASK UINT64_C(0x80)
#define MXP_PORT_CONNECT_INFO_CAL_CONNECTED_POS  7

#define CHILD_POINTER_OFFSET      UINT64_C(0x000000003FFFFFFF)
#define CHILD_POINTER_EXT_BIT_POS 31

/* MXP device port count */
#define MXP_NODE_INFO_NUM_DEVICE_PORT_MASK UINT64_C(0xF000000000000)
#define MXP_NODE_INFO_NUM_DEVICE_PORT_POS  48

struct cmn_cyprus_node_cfg_reg *mxp_get_child_node(
    struct cmn_cyprus_mxp_reg *mxp,
    unsigned int child_index,
    uintptr_t periphbase)
{
    unsigned int offset;
    struct cmn_cyprus_node_cfg_reg *child_node;

    offset = (mxp->CHILD_POINTER[child_index] & CHILD_POINTER_OFFSET);

    child_node = (struct cmn_cyprus_node_cfg_reg *)(periphbase + offset);

    return child_node;
}

bool mxp_is_child_external(
    struct cmn_cyprus_mxp_reg *mxp,
    unsigned int child_index)
{
    /* Read External Child Node indicator, bit[31] */
    return (
        (mxp->CHILD_POINTER[child_index] & (1U << CHILD_POINTER_EXT_BIT_POS)) >
        0);
}

uint8_t mxp_get_device_port_count(struct cmn_cyprus_mxp_reg *mxp)
{
    return (mxp->NODE_INFO & MXP_NODE_INFO_NUM_DEVICE_PORT_MASK) >>
        MXP_NODE_INFO_NUM_DEVICE_PORT_POS;
}

enum cmn_cyprus_device_type mxp_get_device_type(
    struct cmn_cyprus_mxp_reg *mxp,
    uint8_t port)
{
    return mxp->PORT_CONNECT_INFO[port] &
        MXP_PORT_CONNECT_INFO_DEVICE_TYPE_MASK;
}

bool mxp_is_cal_connected(struct cmn_cyprus_mxp_reg *mxp, uint8_t port)
{
    return (mxp->PORT_CONNECT_INFO[port] &
            MXP_PORT_CONNECT_INFO_CAL_CONNECTED_MASK) >>
        MXP_PORT_CONNECT_INFO_CAL_CONNECTED_POS;
}
