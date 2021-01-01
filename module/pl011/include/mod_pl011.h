
/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_PL011_H
#define MOD_PL011_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupModulePl011 PL011 Driver
 *
 * \brief Device driver module for the Primecell® PL011 UART.
 *
 * \{
 */

/*!
 * \brief PL011 element configuration data.
 */
struct mod_pl011_element_cfg {
    /*!
     * \brief Base address of the device registers.
     */
    uintptr_t reg_base;

    /*!
     * \brief Baud rate in bits per second.
     */
    unsigned int baud_rate_bps;

    /*!
     * \brief Reference clock in Hertz.
     */
    uint64_t clock_rate_hz;

#ifdef BUILD_HAS_MOD_CLOCK
    /*!
     * \brief Identifier of the clock that this device depends on.
     *
     * \note If set to a value other than ::FWK_ID_NONE, this device will not be
     *      enabled until the element has received confirmation that the clock
     *      domain is available.
     */
    fwk_id_t clock_id;
#endif

#ifdef BUILD_HAS_MOD_POWER_DOMAIN
    /*!
     * \brief Identifier of the power domain that this device depends on.
     *
     * \note If set to a value other than ::FWK_ID_NONE, this device will not be
     *      enabled until the element has received confirmation that the power
     *      domain is available.
     */
    fwk_id_t pd_id;
#endif
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_PL011_H */
