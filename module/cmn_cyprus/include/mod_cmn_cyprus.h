/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_CMN_CYPRUS_H
#define MOD_CMN_CYPRUS_H

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
 * \brief CMN Cyprus configuration data.
 */
struct mod_cmn_cyprus_config {
    /*! Base address of the configuration address space */
    uintptr_t periphbase;

    /*! Size along x-axis of the interconnect mesh */
    unsigned int mesh_size_x;

    /*! Size along y-axis of the interconnect mesh */
    unsigned int mesh_size_y;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_CMN_CYPRUS_H */
