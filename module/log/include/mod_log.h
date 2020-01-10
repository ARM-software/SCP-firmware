/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_LOG_H
#define MOD_LOG_H

#include <fwk_id.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModuleLog Logging
 *
 * \brief Module used to write messages to an output device.
 *
 * \details This module is used to log messages (similar to a printf() call)
 *      using a configurable output device (for example, a UART).
 *
 *      A grouping feature allows logged messages to be organized into
 *      categories that can be enabled or disabled through the module
 *      configuration.
 * @{
 */

/*!
 * \brief Module configuration.
 */
struct mod_log_config {
    /*! Module or element identifier of the device providing the I/O stream */
    const fwk_id_t device_id;

    /*! API identifier used to select the API provided by the I/O device */
    const fwk_id_t api_id;

    /*!
     * \brief A string that will be written automatically after the module
     *      is initialized.
     *
     * \details Allows a firmware to provide a string that is written before any
     *      other log messages.
     *
     * \note May be NULL, in which case the banner functionality is not used.
     */
    const char *banner;
};

/*!
 * \brief Log driver interface.
 */
struct mod_log_driver_api {
    /*!
     * \brief Pointer to the device's buffer flush function.
     *
     * \details When invoked, the device must ensure any buffered data is
     *      flushed out before it returns.
     *
     * \note This function is \b mandatory.
     *
     * \param device_id Device identifier.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_DEVICE Internal device error.
     */
    int (*flush)(fwk_id_t device_id);

    /*!
     * \brief Pointer to the function used to write a single character.
     *
     * \note This function is \b mandatory.
     *
     * \param device_id Device identifier.
     * \param c The character to be written.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_DEVICE Internal device error.
     */
    int (*putchar)(fwk_id_t device_id, char c);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_LOG_H */
