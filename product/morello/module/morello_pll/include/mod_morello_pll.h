/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MORELLO_PLL_H
#define MOD_MORELLO_PLL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupMORELLOModule MORELLO Product Modules
 * \{
 */

/*!
 * \defgroup GroupMORELLOPLL MORELLO PLL Driver
 *
 * \details A driver for PLL hardware in MORELLO product.
 *
 * \{
 */

/*! Timeout value to wait for a PLL to lock. */
#define MOD_MORELLO_PLL_LOCK_TIMEOUT UINT32_C(0x100000)

/*! Indexes of APIs that the module offers for binding. */
enum mod_morello_pll_api_types {
    MOD_MORELLO_PLL_API_TYPE_DEFAULT,
    MOD_MORELLO_PLL_API_COUNT,
};

/*!
 * \brief PLL device configuration.
 */
struct mod_morello_pll_dev_config {
    /*! Pointer to the PLL's control register 0. */
    volatile uint32_t *const control_reg0;

    /*! Pointer to the PLL's control register 1. */
    volatile uint32_t *const control_reg1;

    /*! The initial rate the PLL is set to during initialization. */
    const uint64_t initial_rate;

    /*!
     * The frequency of the reference clock applied to the PLL. Each PLL
     * instance has a dedicated reference clock input configured through
     * the board controller and the same value should be used in module's
     * element configuration table. This value will be multiplied with a
     * multiplication factor by the PLL to generate the required output
     * frequency.
     */
    const uint64_t ref_rate;

    /*!
     * If \c true, the driver will not attempt to set a default frequency, or
     * to otherwise configure the PLL during the pre-runtime phase. The PLL is
     * expected to be initialized later in response to a notification or other
     * event.
     */
    const bool defer_initialization;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_MORELLO_PLL_H */
