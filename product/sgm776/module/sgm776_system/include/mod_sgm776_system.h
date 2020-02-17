/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SGM776 System Support
 */

#ifndef MOD_SGM776_SYSTEM_H
#define MOD_SGM776_SYSTEM_H

/*!
 * \addtogroup GroupSGM776Module SGM776 Product Modules
 * @{
 */

/*!
 * \defgroup GroupSGM776System SGM776 System Support
 *
 * @{
 */

/*!
 * \brief API indices.
 */
enum mod_sgm776_system_api_idx {
    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_SGM776_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Number of defined APIs */
    MOD_SGM776_SYSTEM_API_COUNT
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SGM776_SYSTEM_H */
