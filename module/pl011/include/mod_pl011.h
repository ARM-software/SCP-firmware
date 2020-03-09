
/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_PL011_H
#define MOD_PL011_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModulePl011 PL011 Driver
 *
 * \brief Arm PL011 device driver, fulfilling the Log module's driver API.
 *
 * \details This module implements a device driver for the Primecell® PL011
 *      UART.
 * @{
 */

/*!
 * \brief PL011 device configuration data.
 */
struct mod_pl011_device_config {
    /*! Base address of the device registers */
    uintptr_t reg_base;

    /*! Baud rate (bits per second) */
    unsigned int baud_rate_bps;

    /*! Reference clock (Hertz) */
    uint64_t clock_rate_hz;

    /*! Identifier of the clock that this device depends on */
    fwk_id_t clock_id;

    /*! Identifier of the power domain that this device depends on */
    fwk_id_t pd_id;
};

/*!
 * \brief Set the baud rate of the PL011 device
 *
 * \param baud_rate_bps The desired baudrate in bps
 * \param clock_rate_hz The clock rate as specified in the config in MHz
 * \param reg_ptr Pointer to the PL011 register to use
 *
 * \retval FWK_E_PARAM if one of the given parameters is invalid
 * \retval FWK_E_RANGE if a calculated value from the parameters is out of range
 * \retval FWK_SUCCESS if operation is successful
 */
int mod_pl011_set_baud_rate(unsigned int baud_rate_bps, uint64_t clock_rate_hz,
    uintptr_t reg_ptr);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_PL011_H */
