/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SC_PLL_H
#define MOD_SC_PLL_H

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupSCModule Silicon Creations (SC) Product Modules
 * \{
 */

/*!
 * \defgroup GroupSC_PLL SC PLL Driver
 *
 * \details A driver for PLL hardware in Silicon Creations (SC) product.
 *
 * \{
 */

/*! Timeout value to wait for a PLL to lock. */
#define MOD_SC_PLL_LOCK_TIMEOUT UINT32_C(0x100000)

/*! Indexes of APIs that the module offers for binding. */
enum mod_sc_pll_api_types {
    MOD_SC_PLL_API_TYPE_DEFAULT,
    MOD_SC_PLL_API_COUNT,
};

static const fwk_id_t mod_sc_pll_api_id_pll =
    FWK_ID_API_INIT(FWK_MODULE_IDX_SC_PLL, MOD_SC_PLL_API_TYPE_DEFAULT);

/*!
 * \brief PLL Parameters.
 */
struct mod_sc_pll_dev_param {
    /*! The minimum post divider 1 value */
    const uint8_t postdiv1_min;
    /*! The maximum post divider 1 value */
    const uint8_t postdiv1_max;
    /*! The minimum post divider 2 value */
    const uint8_t postdiv2_min;
    /*! The maximum post divider 2 value */
    const uint8_t postdiv2_max;
    /*! The minimum frequency that the PLL hardware can output. */
    const uint64_t pll_rate_min;
    /*! The maximum frequency that the PLL hardware can output. */
    const uint64_t pll_rate_max;
};

/*!
 * \brief PLL device configuration.
 */
struct mod_sc_pll_dev_config {
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

    /*! PLL Device Parameters used to configure it during init time */
    struct mod_sc_pll_dev_param *dev_param;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SC_PLL_H */
