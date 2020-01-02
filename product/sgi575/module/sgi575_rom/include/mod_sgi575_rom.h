/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SGI575_ROM_H
#define MOD_SGI575_ROM_H

#include <stdint.h>

/*!
 * \addtogroup GroupSGI575Module SGI575 Product Modules
 * @{
 */

/*!
 * \defgroup GroupSGI575 ROM Support
 * @{
 */

/*!
 * \brief Module configuration data.
 */
struct sgi575_rom_config {
    /*! Base address of the RAM firmware image */
    const uintptr_t ramfw_base;

    /*! Base address of the NOR flash memory */
    const uintptr_t nor_base;

    /*! The RAM size to load */
    const unsigned int load_ram_size;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SGI575_ROM_H */
