/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SGM775 System Support
 */

#ifndef MOD_SGM775_SYSTEM_H
#define MOD_SGM775_SYSTEM_H

/*!
 * \addtogroup GroupSGM775Module SGM775 Product Modules
 * \{
 */

/*!
 * \defgroup GroupSGM775System SGM775 System Support
 *
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_sgm775_system_api_idx {
    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_SGM775_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Number of defined APIs */
    MOD_SGM775_SYSTEM_API_COUNT
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SGM775_SYSTEM_H */
