/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Clock driver for the clocks of the Juno SoC
 */

#ifndef MOD_JUNO_SOC_CLOCK_H
#define MOD_JUNO_SOC_CLOCK_H

#include <stdint.h>

/*!
 * \addtogroup GroupJunoModule Juno Product Modules
 * \{
 */

/*!
 * \defgroup GroupJunoSocClock Juno SoC Clock Driver
 *
 * \details A driver for clocks of the Juno SoC.
 *
 * \{
 */

/*!
 * \brief Sub-types of Juno SOC clock.
 */
enum mod_juno_soc_clock_type {
    /*! A clock with a fixed source. Only its divider can be changed. */
    MOD_JUNO_SOC_CLOCK_TYPE_SINGLE_SOURCE,

    /*! A clock with multiple, selectable sources and at least one divider */
    MOD_JUNO_SOC_CLOCK_TYPE_MULTI_SOURCE,
};

/*!
 * \brief Juno SoC clock configuration.
 */
struct mod_juno_soc_clock_dev_config {
    /*! The type of the clock */
    enum mod_juno_soc_clock_type type;

    /*! Pointer to the clock's control register */
    volatile uint32_t * const control_reg;

    /*! Clock's rate in Hertz */
    uint32_t rate;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_JUNO_SOC_CLOCK_H */
