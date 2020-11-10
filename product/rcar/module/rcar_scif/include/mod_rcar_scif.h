/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_SCIF_H
#define MOD_RCAR_SCIF_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * \{
 */

/*!
 * \defgroup GroupRCARSCIF SCIF Driver
 *
 * \brief Device driver module for the Primecell® SCIF UART.
 *
 * \{
 */

/*!
 * \brief SCIF element configuration data.
 */
struct mod_rcar_scif_element_cfg {
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
};

/*!
 * \brief APIs provided by the driver.
 */
enum mod_rcar_scif_api_type {
    MOD_RCAR_SCIF_API_TYPE_NORMAL,
    MOD_RCAR_SCIF_API_TYPE_SYSTEM,
    MOD_RCAR_SCIF_API_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_RCAR_SCIF_H */
