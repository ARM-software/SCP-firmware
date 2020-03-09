/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit (MHU) v2 Device Driver.
 */

#ifndef MOD_MHU2_H
#define MOD_MHU2_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupMHUv2 Message Handling Unit (MHU) v2 Driver
 * @{
 */

/*!
 * \brief MHU v2 api indicies
 */
enum mod_mhu2_api_idx {
    /*! SMT driver API */
    MOD_MHU2_API_IDX_SMT_DRIVER,
    /*! Number of APIs */
    MOD_MHU2_API_IDX_COUNT,
};

/*!
 * \brief MHU v2 device
 *
 * \details Abstract representation of a bidirectional MHU channel that consists
 *      of a single receive interrupt line and a pair of register sets, one for
 *      each direction of communication.
 */
struct mod_mhu2_channel_config {
    /*! IRQ number of the receive interrupt line */
    unsigned int irq;

    /*! Base address of the registers of the incoming MHU */
    uintptr_t recv;

    /*! Base address of the registers of the outgoing MHU */
    uintptr_t send;

    /*! Channel number */
    unsigned int channel;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_MHU2_H */
