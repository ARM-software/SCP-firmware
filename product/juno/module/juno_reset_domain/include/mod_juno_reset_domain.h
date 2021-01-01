/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     JUNO Reset Domain module.
 */

#ifndef MOD_JUNO_RESET_DOMAIN_H
#define MOD_JUNO_RESET_DOMAIN_H

#include <fwk_id.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupJUNORESET JUNO RESET DOMAIN
 *
 * \{
 */

/*!
 * \brief Juno Reset domain indexes.
 *
 */
enum juno_reset_domain_idx {
    /*! UART element index. */
    JUNO_RESET_DOMAIN_IDX_UART,

    /*! Number of defined elements */
    JUNO_RESET_DOMAIN_IDX_COUNT,
};

/*!
 * \brief Element configuration.
 */
struct mod_juno_reset_uart_config {
    /*! Address of Juno VSYS Manual Reset Register.*/
    volatile uint32_t *reset_reg;

    /*! Bit Mask to enable/clear reset of the device in VSYS manual reset
     *  register.
     */
    unsigned int  reset_mask;
};

/*!
 * \brief Juno Reset Domain API indices.
 */
enum mod_juno_reset_domain_api_idx {
    /*! Index of the driver API */
    MOD_JUNO_RESET_DOMAIN_API_IDX_DRIVER,

    /*! Number of APIs for the Juno reset domain driver module */
    MOD_JUNO_RESET_DOMAIN_API_IDX_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif  /* MOD_JUNO_RESET_DOMAIN_DEBUG_H */
