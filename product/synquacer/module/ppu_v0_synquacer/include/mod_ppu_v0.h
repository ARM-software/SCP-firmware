/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
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
 * \addtogroup GroupSYNQUACERModule SYNQUACER Product Modules
 * @{
 */

/*!
 * \defgroup GroupSYNQUACER_PPUv0 PPUv0 Driver
 *
 * \brief PPUv0 device driver.
 *
 * \details This module implements a device driver for the PPUv0.
 *    Threre are 3 reasons why SynQuacer has custom PPUv0 driver.
 *     1) Existing ppu_v0 does not support CLUSTER/CORE power management.
 *     2) Synquacer needs special handling when PPU is set to OFF.
 *     3) need special handling for setting MOD_PD_TYPE_SYSTEM to OFF.
 *
 * @{
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
 * \brief Configuration data of a power domain of the PPU_V0 driver module.
 */
struct mod_ppu_v0_pd_config {
    /*! Power domain type */
    enum mod_pd_type pd_type;

    /*! PPU descriptor */
    struct mod_ppu_v0 ppu;

    /*!
     * Flag indicating if this domain should be powered on during element init.
     */
    bool default_power_on;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_PPU_V0_H */
