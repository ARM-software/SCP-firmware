/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_N1SDP_ROM_H
#define MOD_N1SDP_ROM_H

#include <stdint.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup GroupN1SDPRom N1SDP SCP ROM Support
 * @{
 */

/*!
 * \brief Module configuration data.
 */
struct n1sdp_rom_config {
    /*! Base address of the RAM to which SCP BL2 will be copied to */
    const uintptr_t ramfw_base;

    /*! Type of RAM Firmware to load */
    const uint8_t image_type;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_N1SDP_ROM_H */
