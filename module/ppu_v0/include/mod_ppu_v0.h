/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     PPU v0 driver module
 */

#ifndef MOD_PPU_V0_H
#define MOD_PPU_V0_H

#include <mod_power_domain.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupModulePPUv0 PPUv0 Driver
 * \{
 */

/*!
 * \brief Power domain PPU descriptor.
 */
struct mod_ppu_v0 {
    /*! Base address of the PPU registers */
    uintptr_t reg_base;

    /*! PPU's IRQ number */
    unsigned int irq;
};

/*!
 * \brief Timer for set_state.
 *
 * \details This structure is required to be filled in PPUv0 config file only
 *          when the timeout feature is required.
 */
struct mod_ppu_v0_timer_config {
    /*!
     * \brief Timer identifier.
     *
     * \details Used for binding with the timer API and waiting for specified
     *          delay after setting the PPU state.
     */
    fwk_id_t timer_id;

    /*!
     * PPU state change wait delay in micro seconds.
     * A valid non-zero value has to be specified when using this feature.
     */
    uint32_t set_state_timeout_us;
};

/*!
 * \brief Configuration data of a power domain of the PPU_V0 driver module.
 */
struct mod_ppu_v0_pd_config {
    /*! Power domain type */
    enum mod_pd_type pd_type;

    /*! PPU descriptor */
    struct mod_ppu_v0 ppu;

    /*! Timer descriptor */
    struct mod_ppu_v0_timer_config *timer_config;

    /*!
     * Flag indicating if this domain should be powered on during element init.
     * Timeout is not provided at this stage.
     */
    bool default_power_on;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_PPU_V0_H */
