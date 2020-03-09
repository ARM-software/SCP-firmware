/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SYSTEM_PLL_H
#define MOD_SYSTEM_PLL_H

#include <fwk_element.h>
#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupSystemPll System PLL Driver
 *
 * \details A driver for system PLL devices.
 *
 * @{
 */

/*! The slowest rate at which the PLL hardware can operate. */
#define MOD_SYSTEM_PLL_MIN_RATE (50UL * FWK_MHZ)

/*! The fastest rate at which the PLL hardware can operate. */
#define MOD_SYSTEM_PLL_MAX_RATE (4UL * FWK_GHZ)

/*! The maximum precision that can be used when setting the PLL rate. */
#define MOD_SYSTEM_PLL_MIN_INTERVAL (1UL * FWK_KHZ)

/*! Indexes of APIs that the module offers for binding. */
enum mod_system_pll_api_types {
    MOD_SYSTEM_PLL_API_TYPE_DEFAULT,
    MOD_SYSTEM_PLL_API_COUNT,
};

/*!
 * \brief PLL device configuration.
 */
struct mod_system_pll_dev_config {
    /*! Pointer to the PLL's control register. */
    volatile uint32_t * const control_reg;

    /*! Pointer to the PLL's status register, if any. */
    volatile uint32_t * const status_reg;

    /*!
     * Mask for the bit within the status register that indicates whether the
     * PLL has locked at the programmed rate.
     */
    const uint32_t lock_flag_mask;

    /*! The initial rate the PLL is set to during initialization. */
    const uint64_t initial_rate;

    /*!
     * The slowest rate the PLL can be set to. This may be different from the
     * hardware-imposed limit).
     */
    const uint64_t min_rate;

    /*!
     * The fastest rate the PLL can be set to. This may be different from the
     * hardware-imposed limit).
     */
    const uint64_t max_rate;

    /*!
     * The maximum precision that can be used when setting the PLL rate. This
     * may be different from the hardware-imposed limit).
     */
    const uint64_t min_step;

    /*!
     * If \c true, the driver will not attempt to set a default frequency, or to
     * otherwise configure the PLL during the pre-runtime phase. The PLL is
     * expected to be initialized later in response to a notification or other
     * event.
     */
    const bool defer_initialization;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SYSTEM_PLL_H */
