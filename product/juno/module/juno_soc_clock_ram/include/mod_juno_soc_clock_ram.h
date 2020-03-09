/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_JUNO_SOC_CLOCK_RAM_H
#define MOD_JUNO_SOC_CLOCK_RAM_H

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupJunoSocClockRam Juno Soc Clock Ram Driver
 *
 * \details A driver for Juno Soc clock devices for GPU and
 *          big/little core clusters.
 *
 * @{
 */

/*! Juno SoC clock API indexes */
enum mod_juno_soc_clock_ram_api_idx {
    MOD_JUNO_SOC_CLOCK_RAM_API_IDX_DRIVER,
    MOD_JUNO_SOC_CLOCK_RAM_API_IDX_COUNT,
};

static const fwk_id_t mod_juno_soc_clock_ram_api_id_driver =
    FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_SOC_CLOCK_RAM,
        MOD_JUNO_SOC_CLOCK_RAM_API_IDX_DRIVER);

/*!
 * \brief Juno SoC clock module configuration.
 */
struct mod_juno_soc_clock_ram_config {
    /*!
     *  Identifier of the timer to use for timeout checking.
     */
    fwk_id_t timer_id;

    /*!
     * Identifier of the debug power domain.
     */
    fwk_id_t debug_pd_id;

    /*!
     * Identifier of the systop power domain.
     */
    fwk_id_t systop_pd_id;
};

/*!
 * \brief PLL setting definition.
 */
struct juno_soc_clock_ram_pll_setting {
    /*! PLL feedback divider */
    uint16_t nf;

    /*! PLL reference clock divider */
    uint8_t nr;

    /*! PLL output divider */
    uint8_t od;
};

/*!
 * \brief Rate lookup table entry.
 */
struct juno_soc_clock_ram_rate {
    /*! Rate of the clock in Hertz. */
    uint32_t rate;

    /*! The clock source used to attain the rate. */
    uint32_t source;

    /*! The clock divider used to attain the rate. */
    unsigned int divider;

    /*!
     *  Dedicated PLL setting to be used to attain the rate if the PLL is the
     *  source of the clock.
     */
    struct juno_soc_clock_ram_pll_setting pll_setting;
};

/*!
 * \brief Juno SoC clock device configuration.
 */
struct mod_juno_soc_clock_ram_dev_config {
    /*! Pointer to the registers of the dedicated PLL. */
    struct pll_reg *pll;

    /*! Pointer to the clock control register. */
    FWK_RW uint32_t *clock_control;

    /*! Pointer to the clock's rate lookup table. */
    const struct juno_soc_clock_ram_rate *rate_table;

    /*! The number of rates in the rate lookup table. */
    unsigned int rate_count;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_JUNO_SOC_CLOCK_RAM_H */
