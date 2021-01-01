/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SYNQUACER_ROM_H
#define MOD_SYNQUACER_ROM_H

#include <stdint.h>

/*!
 * \addtogroup GroupSYNQUACERModule SYNQUACER Product Modules
 * \{
 */

/*!
 * \defgroup GroupSYNQUACER_ROM ROM Support
 *
 * \brief SynQuacer ROM support.
 *
 * \details This module implements a service for the ROM firmware
 *
 * \{
 */

/*!
 * \brief Module configuration data.
 */
struct synquacer_rom_config {
    /*! Base address of the RAM firmware image */
    const uintptr_t ramfw_base;

    /*! Base address of the NOR flash memory */
    const uintptr_t nor_base;

    /*! The RAM size to load */
    const unsigned int load_ram_size;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SYNQUACER_ROM_H */
