/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_F_UART3_H
#define MOD_F_UART3_H

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupSYNQUACERModule SYNQUACER Product Modules
 * \{
 */

/*!
 * \defgroup GroupF_UART3 F_UART3 Driver
 *
 * \brief SynQuacer F_UART3 device driver.
 *
 * \details This module implements a device driver for the F_UART3
 *
 * \{
 */

/*!
 * \brief Support baud rates.
 */
enum mod_f_uart3_baud_rate {
    MOD_F_UART3_BAUD_RATE_9600, /*!< 9600 bits per second */
    MOD_F_UART3_BAUD_RATE_19200, /*!< 19200 bits per second */
    MOD_F_UART3_BAUD_RATE_38400, /*!< 38400 bits per second */
    MOD_F_UART3_BAUD_RATE_57600, /*!< 57600 bits per second */
    MOD_F_UART3_BAUD_RATE_115200, /*!< 115200 bits per second */
    MOD_F_UART3_BAUD_RATE_230400, /*!< 230400 bits per second */
};

/*!
 * \brief F_UART3 device configuration data.
 */
struct mod_f_uart3_element_cfg {
    /*! Base address of the device registers */
    uintptr_t reg_base;

    /*! Base address of the DivLatch access device registers */
    uintptr_t dla_reg_base;

    /*! Parity enable */
    bool parity_enable_flag;

    /*! Even parity select */
    bool even_parity_flag;

    /*! Divider latch table index */
    enum mod_f_uart3_baud_rate baud_rate;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_F_UART3_H */
