/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_CMN_CYPRUS_H
#define MOD_CMN_CYPRUS_H

#include <fwk_id.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModuleCMN_CYPRUS CMN_CYPRUS
 *
 * \brief Arm Coherent Mesh Network (CMN) Cyprus driver module.
 *
 * \details This module adds support for the CMN Cyprus interconnect.
 * @{
 */

/*!
 * \brief Coordinate (x, y, port number, device number) of a node in the mesh.
 */
struct cmn_cyprus_node_pos {
    /*! X position of the node in the mesh */
    unsigned int pos_x;

    /*! Y position of the node in the mesh */
    unsigned int pos_y;

    /*! Port number of the node in the MXP */
    uint8_t port_num;

    /*! Device number of the node in the MXP port */
    unsigned int device_num;
};

/*!
 * \brief CMN Cyprus configuration data.
 */
struct mod_cmn_cyprus_config {
    /*! Base address of the configuration address space */
    uintptr_t periphbase;

    /*! Size along x-axis of the interconnect mesh */
    unsigned int mesh_size_x;

    /*! Size along y-axis of the interconnect mesh */
    unsigned int mesh_size_y;

    /*! Identifier of the clock that this device depends on */
    fwk_optional_id_t clock_id;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_CMN_CYPRUS_H */
