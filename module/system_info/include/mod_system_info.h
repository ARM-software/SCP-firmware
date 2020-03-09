/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SYSTEM_INFO_H
#define MOD_SYSTEM_INFO_H

#include <fwk_id.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModuleSystemInfo System Information Abstraction Module
 *
 * \brief HAL Module used to get System Information.
 *
 * \details This module provides an abstraction layer to the modules which
 *      need System Information. This module binds to the register interface
 *      driver module to obtain a pointer to the system information data. Module
 *      which requires system information data then can use the api provided
 *      by this module to get the generic system information.
 *      @{
 */

/*!
 * \brief Generic System Information
 *
 * \details This structure holds the generic information about the current
 *      system. The register interface module should define this structure
 *      and provide a pointer to this module.
 */
struct mod_system_info {
    /*! Product identification number of the system */
    uint32_t product_id;

    /*! Configuration number of the system */
    uint32_t config_id;

    /*! Multi-chip mode tie-off value - enabled or disabled */
    bool multi_chip_mode;

    /*!
     * Chip id indicating unique identifier of the chip in a multi socket system
     */
    uint8_t chip_id;

    /*! Name of the system */
    const char *name;
};

/*!
 * \brief System Information configuration data
 */
struct mod_system_info_config {
    /*!
     * Module ID of the register interface driver module which provides a
     * pointer to system information data. If the product does not support any
     * driver, this can be set to FWK_ID_NONE.
     */
    fwk_id_t system_info_driver_module_id;

    /*! API ID for getting the system information data from the driver module */
    fwk_id_t system_info_driver_data_api_id;
};

/*!
 * \brief API structure to be defined by the driver module.
 */
struct mod_system_info_get_driver_data_api {
    /*!
     * \brief Get the system information data populated by the driver module.
     *
     * \details API to be implemented by the driver module which provides a
     *      pointer to the system information data.
     *
     * \retval NULL if the driver data is not initialized yet.
     * \return Pointer to the system information driver data.
     */
    struct mod_system_info *(*get_driver_data)(void);
};

/*!
 * \brief API structure used by the modules requesting for the system
 *      information data.
 */
struct mod_system_info_get_info_api {
    /*!
     * \brief Get system information data.
     *
     * \details API to be used by the module requesting for the system
     *      information data.
     *
     * \param[out] sys_info Pointer to the system information data.
     *
     * \retval FWK_SUCCESS if the sys_info pointer has been successfully set.
     * \retval FWK_E_SUPPORT if the system information is not supported by the
     *      product.
     */
    int (*get_system_info)(const struct mod_system_info **sys_info);
};

/*!
 * \brief Module API indicies.
 */
enum mod_system_info_api_idx {
    MOD_SYSTEM_INFO_GET_API_IDX,
    MOD_SYSTEM_INFO_API_COUNT
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SYSTEM_INFO_H */
