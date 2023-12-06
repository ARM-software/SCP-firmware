/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for accessing node info register.
 */

#include <internal/cmn_cyprus_node_info_reg.h>

#include <fwk_macros.h>

#include <stdint.h>

/* Node Info */
#define NODE_INFO_ID             UINT64_C(0x00000000FFFF0000)
#define NODE_INFO_ID_POS         16
#define NODE_INFO_LOGICAL_ID     UINT64_C(0x0000FFFF00000000)
#define NODE_INFO_LOGICAL_ID_POS 32
#define NODE_INFO_TYPE           UINT64_C(0x000000000000FFFF)

enum cmn_cyprus_node_type node_info_get_type(FWK_R uint64_t node_info)
{
    return (enum cmn_cyprus_node_type)(node_info & NODE_INFO_TYPE);
}

unsigned int node_info_get_id(FWK_R uint64_t node_info)
{
    return (node_info & NODE_INFO_ID) >> NODE_INFO_ID_POS;
}

unsigned int node_info_get_ldid(FWK_R uint64_t node_info)
{
    return (node_info & NODE_INFO_LOGICAL_ID) >> NODE_INFO_LOGICAL_ID_POS;
}
