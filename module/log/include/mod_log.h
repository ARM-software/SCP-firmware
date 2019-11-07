/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
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
 * \brief Log groups.
 *
 * \details Log groups are used for:
 *      * Categorizing a message during the \ref mod_log_api::log()
 *        call.
 *      * Selectively enabling or disabling certain message types.
 *
 * \note Log groups are not organized in a heirarchy and do not indicate any
 *      kind of priority.
 */
enum mod_log_group {
    /*! Used only to indicate all log groups are muted */
    MOD_LOG_GROUP_NONE = 0,

    /*! The debug log group */
    MOD_LOG_GROUP_DEBUG = (1 << 0),

    /*! The error log group */
    MOD_LOG_GROUP_ERROR = (1 << 1),

    /*! The info log group */
    MOD_LOG_GROUP_INFO = (1 << 2),

    /*! The warning log group */
    MOD_LOG_GROUP_WARNING = (1 << 3),
};

/*!
 * \brief Log module API identifier.
 *
 * \details The log module only has a single API, which is identified by this
 *      identifier.
 */
#define MOD_LOG_API_ID  FWK_ID_API(FWK_MODULE_IDX_LOG, 0)

/*!
 * \brief Module configuration.
 */
struct mod_log_config {
    /*! Module or element identifier of the device providing the I/O stream */
    const fwk_id_t device_id;

    /*! API identifier used to select the API provided by the I/O device */
    const fwk_id_t api_id;

    /*! Default log groups. Value is a mask (see \ref mod_log_group) */
    const unsigned int log_groups;

    /*!
     * \brief A string that will be written automatically after the module
     *      is initialized.
     *
     * \details Allows a firmware to provide a string that is written before any
     *      other log messages. The banner uses the \ref MOD_LOG_GROUP_INFO log
     *      group.
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
 * \brief Module interface
 */
struct mod_log_api {
    /*!
     * \brief Log formatted data.
     *
     * \details Log formatted data assigned to a log group. Only logs that are
     *      assigned to enabled groups will be output. The module configuration
     *      can be used to enable and disable log groups.
     *
     * \param group One of the log groups that the log is assigned to (\see
     *      log_group).
     *
     * \param fmt String containing the formatted data to be printed out. The
     *      following formats are supported:
     *      * \%c - character format
     *      * \%d and \%i - signed 32-bit decimal format
     *      * \%lx and \%llx - 64-bit hexadecimal format
     *      * \%s - string format
     *      * \%u - unsigned 32-bit decimal format
     *      * \%x - 32-bit hexadecimal format
     *
     *      Numeric formats also accept a padding flag '0\<width\>' between the
     *      '\%' and the format specifier where the resulting string number will
     *      be left padded with zeros. Examples:
     *      * log("%04d", 1) results in "0001"
     *      * log("%04d", 9999) results in "9999"
     *      * __Note__: \<width\> must be a number between 0 and 9 inclusive.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_DATA Invalid format specifier(s).
     * \retval FWK_E_DEVICE Internal device error.
     * \retval FWK_E_PARAM Invalid group.
     * \retval FWK_E_PARAM Invalid 'fmt' pointer.
     * \retval FWK_E_STATE Log module is not ready.
     */
    int (*log)(enum mod_log_group group, const char *fmt, ...);

    /*!
     * \brief Function used to flush the log's buffer.
     *
     * \details When invoked, any buffered log data will be flushed out before
     *      this function returns.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_DEVICE Internal device error.
     * \retval FWK_E_STATE Log module is not ready.
     */
    int (*flush)(void);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_LOG_H */
