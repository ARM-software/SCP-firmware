/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MSCP_ROM_H
#define MOD_MSCP_ROM_H

#include <stdint.h>

/*!
 * \addtogroup GroupSCPModule MSCP Product Modules
 * @{
 */

/*!
 * \defgroup GroupSCP ROM Support
 * @{
 */

/*!
 * \brief Module configuration data.
 */
struct mscp_rom_config {
    /*! Base address in RAM at which RAM firmware image has to be loaded */
    const uintptr_t ramfw_ram_base;

    /*! Base address of RAM firmware image on the NOR flash memory */
    const uintptr_t ramfw_flash_base;

    /*! The size of the RAM firmware image on the NOR flash memory */
    const unsigned int ramfw_flash_size;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_MSCP_ROM_H */
