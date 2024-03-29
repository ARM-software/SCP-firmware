/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MOCK_PPU_H
#define MOD_MOCK_PPU_H

#include <stdbool.h>
#include <stdint.h>
#include <mod_power_domain.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModulePPUv0 PPUv0 Driver
 * @{
 */

/*!
 * \brief Power domain PPU descriptor.
 */
struct mod_mock_ppu {
    /*! Base address of the PPU registers */
    uintptr_t reg_base;

    /*! PPU's IRQ number */
    unsigned int irq;
};

/*!
 * \brief Configuration data of a power domain of the PPU_V0 driver module.
 */
struct mod_mock_ppu_pd_config {
    /*! Power domain type */
    enum mod_pd_type pd_type;

    /*! PPU descriptor */
    struct mod_mock_ppu ppu;

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
