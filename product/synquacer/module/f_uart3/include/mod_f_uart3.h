/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_F_UART3_H
#define MOD_F_UART3_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupSYNQUACERModule SYNQUACER Product Modules
 * @{
 */

/*!
 * \defgroup GroupF_UART3 F_UART3 Driver
 *
 * \brief SynQuacer F_UART3 device driver.
 *
 * \details This module implements a device driver for the F_UART3
 *
 * @{
 */

/*!
 * \brief F_UART3 device configuration data.
 */
struct mod_f_uart3_device_config {
    /*! Base address of the device registers */
    uintptr_t reg_base;

    /*! Base address of the DivLatch access device registers */
    uintptr_t dla_reg_base;

    /*! Baud rate (bits per second) */
    unsigned int baud_rate_bps;

    /*! Reference clock (Hertz) */
    uint64_t clock_rate_hz;

    /*! Identifier of the clock that this device depends on */
    fwk_id_t clock_id;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_F_UART3_H */
