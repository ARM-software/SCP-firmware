/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MORELLO_ROM_H
#define MOD_MORELLO_ROM_H

#include <mod_fip.h>

#include <stdint.h>

/*!
 * \addtogroup GroupMORELLOModule MORELLO Product Modules
 * \{
 */

/*!
 * \defgroup GroupMORELLORom MORELLO SCP ROM Support
 * \{
 */

/*!
 * \brief Module configuration data.
 */
struct morello_rom_config {
    /*! Base address of the RAM to which SCP BL2 will be copied to */
    const uintptr_t ramfw_base;

    /*! Type of RAM Firmware to load */
    enum mod_fip_toc_entry_type image_type;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_MORELLO_ROM_H */
