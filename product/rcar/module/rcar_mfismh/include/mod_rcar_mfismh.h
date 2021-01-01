/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_MFISMH_H
#define MOD_RCAR_MFISMH_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * @{
 */

/*!
 * \defgroup GroupRCARMHU Message Handling Unit (MHU) Driver
 * @{
 */

/*!
 * \brief MFISMB device
 *
 * \details Abstract representation of a bidirectional
 *      MFISMB device that consists
 *      of a single receive interrupt line and a pair of register sets, one for
 *      each direction of communication.
 */
struct mod_rcar_mfismh_device_config {
    /*! IRQ number of the receive interrupt line */
    unsigned int irq;

    /*! Base address of the registers of the incoming MFISMHU */
    uintptr_t in;

    /*! Base address of the registers of the outgoing MFISMHU */
    uintptr_t out;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_MFISMH_H */
