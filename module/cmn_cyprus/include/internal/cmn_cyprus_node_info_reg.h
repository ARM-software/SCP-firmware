/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for the accessing node info register.
 */

#ifndef CMN_CYPRUS_NODE_INFO_REG_INTERNAL_H
#define CMN_CYPRUS_NODE_INFO_REG_INTERNAL_H

#include <fwk_macros.h>

#include <stdint.h>

/*
 * CMN-Cyprus node type identifiers.
 */
enum cmn_cyprus_node_type {
    NODE_TYPE_INVALID = 0x0,
    NODE_TYPE_DVM = 0x1,
    NODE_TYPE_CFG = 0x2,
    NODE_TYPE_DTC = 0x3,
    NODE_TYPE_HN_I = 0x4,
    NODE_TYPE_HN_F = 0x5,
    NODE_TYPE_XP = 0x6,
    NODE_TYPE_SBSX = 0x7,
    NODE_TYPE_HN_F_MPAM_S = 0x8,
    NODE_TYPE_HN_F_MPAM_NS = 0x9,
    NODE_TYPE_RN_I = 0xA,
    NODE_TYPE_RN_D = 0xD,
    NODE_TYPE_RN_SAM = 0xF,
    NODE_TYPE_HN_P = 0x11,
    NODE_TYPE_CCRA = 0x103,
    NODE_TYPE_CCHA = 0x104,
    NODE_TYPE_CCLA = 0x105,
    NODE_TYPE_HN_S = 0x200,
    NODE_TYPE_HN_S_MPAM_S = 0x201,
    NODE_TYPE_HN_S_MPAM_NS = 0x202,
    NODE_TYPE_COUNT,
};

/*
 * Retrieve node type identifier.
 *
 * \param node_info Node Info register value.
 *
 * \return Node's type identifier.
 */
enum cmn_cyprus_node_type node_info_get_type(FWK_R uint64_t node_info);

/*
 * Retrieve the node id.
 *
 * \param node_info Node Info register value.
 *
 * \return Node's physical identifier.
 */
unsigned int node_info_get_id(FWK_R uint64_t node_info);

/*
 * Retrieve the node logical id.
 * This is an unique identifier (index) among nodes of the same type in the
 * system.
 *
 * \param node_info Node Info register value.
 *
 * \return An integer representing the node's logical identifier.
 */
unsigned int node_info_get_ldid(FWK_R uint64_t node_info);

#endif /* CMN_CYPRUS_NODE_INFO_REG_INTERNAL_H */
