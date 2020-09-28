/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
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
 * \brief PLL parameter values for non-absolute frequencies.
 */
struct morello_pll_custom_freq_param_entry {
    /*! Required output frequency value in MHz */
    uint16_t freq_value_mhz;

    /*! Feedback divider value for this frequency */
    uint16_t fbdiv;

    /*! Reference clock divider value for this frequency */
    uint8_t refdiv;

    /*! Post divider 1 value for this frequency */
    uint8_t postdiv;
};

/*!
 * \brief MORELLO PLL module configuration.
 */
struct morello_pll_module_config {
    /*! Pointer to custom frequency table */
    struct morello_pll_custom_freq_param_entry *custom_freq_table;

    /*! Size of custom frequency table */
    size_t custom_freq_table_size;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_MORELLO_PLL_H */
