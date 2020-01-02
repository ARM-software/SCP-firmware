/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RDN1E1_ROM_H
#define MOD_RDN1E1_ROM_H

#include <stdint.h>

/*!
 * \addtogroup GroupRDN1E1Module RDN1E1 Product Modules
 * @{
 */

/*!
 * \defgroup GroupRDN1E1 ROM Support
 * @{
 */

/*!
 * \brief Module configuration data.
 */
struct rdn1e1_rom_config {
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

#endif /* MOD_RDN1E1_ROM_H */
