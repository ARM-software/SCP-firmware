/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit (MHU) Device Driver.
 */

#ifndef MOD_MHU_H
#define MOD_MHU_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup GroupN1SDPMHU N1SDP Message Handling Unit (MHU) Driver
 * @{
 */

/*!
 * \brief MHU device
 *
 * \details Abstract representation of a bidirectional MHU device that consists
 *      of a single receive interrupt line and a pair of register sets, one for
 *      each direction of communication.
 */
struct mod_mhu_device_config {
    /*! IRQ number of the receive interrupt line */
    unsigned int irq;

    /*! Base address of the registers of the incoming MHU */
    uintptr_t in;

    /*! Base address of the registers of the outgoing MHU */
    uintptr_t out;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_MHU_H */
