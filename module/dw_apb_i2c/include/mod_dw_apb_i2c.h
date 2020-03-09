/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     DesignWare DW_apb_i2c I2C controller
 */

#ifndef MOD_DW_APB_I2C_H
#define MOD_DW_APB_I2C_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModuleI2CController I2C Controller
 *
 * \brief Driver for I2C device.
 * @{
 */

/*!
 * \brief Element configuration
 */
struct mod_dw_apb_i2c_dev_config {
    /*!
     * \brief Identifier of the timer
     *
     * \details The timer is used to wait on the I2C bus status.
     */
    fwk_id_t timer_id;
    /*! Interrupt number of the I2C device */
    unsigned int i2c_irq;
    /*! Base address of the I2C device registers */
    uintptr_t reg;
};

/*! API indices */
enum mod_dw_apb_i2c_api_idx {
    MOD_DW_APB_I2C_API_IDX_DRIVER,
    MOD_DW_APB_I2C_API_IDX_COUNT,
};

/*! Identifier for the I2C driver API */
static const fwk_id_t mod_dw_apb_i2c_api_id_driver = FWK_ID_API_INIT(
    FWK_MODULE_IDX_DW_APB_I2C, MOD_DW_APB_I2C_API_IDX_DRIVER);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_DW_APB_I2C_H */
