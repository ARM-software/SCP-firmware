/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     XRP7724 driver (aka Juno PMIC)
 */

#ifndef MOD_JUNO_XRP7724_H
#define MOD_JUNO_XRP7724_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

/*!
 * \ingroup GroupJunoModule
 *
 * \defgroup GroupXRP7724 XRP7724 Driver
 * \{
 */

/*!
 * \brief Module configuration
 */
struct mod_juno_xrp7724_config {
    /*! Slave address of the I2C device */
    unsigned int slave_address;
    /*! Identifier of the I2C HAL */
    fwk_id_t i2c_hal_id;
    /*! Identifier of the timer */
    fwk_id_t timer_hal_id;
    /*! Identifier of the GPIO for the assert command */
    fwk_id_t gpio_assert_id;
    /*! Identifier of the GPIO for the system mode selection */
    fwk_id_t gpio_mode_id;
};

/*!
 * \brief Element type
 */
enum mod_juno_xrp7724_element_type {
    MOD_JUNO_XRP7724_ELEMENT_TYPE_GPIO,
    MOD_JUNO_XRP7724_ELEMENT_TYPE_COUNT,
};

/*!
 * \brief Element configuration
 */
struct mod_juno_xrp7724_dev_config {
    /*! Element type */
    enum mod_juno_xrp7724_element_type type;
};

/*! API for system mode */
struct mod_juno_xrp7724_api_system_mode {
    /*!
     * \brief Perform a shutdown.
     *
     * \note The caller cannot be running on the common thread.
     * \note This function returns only if the shutdown request has failed.
     */
    void (*shutdown)(void);

    /*!
     * \brief Perform a cold reset.
     *
     * \note The caller cannot be running on the common thread.
     * \note This function returns only if the reset request has failed.
     */
    void (*reset)(void);
};

/*! Index of the available APIs */
enum mod_juno_xrp7724_api_idx {
    MOD_JUNO_XRP7724_API_IDX_SYSTEM_MODE,
    MOD_JUNO_XRP7724_API_IDX_COUNT,
};

/*! Identifier of the system mode API */
static const fwk_id_t mod_juno_xrp7724_api_id_system_mode = FWK_ID_API_INIT(
    FWK_MODULE_IDX_JUNO_XRP7724, MOD_JUNO_XRP7724_API_IDX_SYSTEM_MODE);

/*!
 * \}
 */

#endif /* MOD_JUNO_XRP7724_H */
