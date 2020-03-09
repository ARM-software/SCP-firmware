/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP DDR-PHY driver
 */
#ifndef MOD_N1SDP_DDR_PHY_H
#define MOD_N1SDP_DDR_PHY_H

#include <stdint.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup GroupModuleN1SDPDDRPhy N1SDP DDR PHY Driver
 *
 * \brief Driver support for N1SDP DDR PHY instances.
 *
 * \{
 */

/*!
 * \brief Element configuration.
 */
struct mod_n1sdp_ddr_phy_element_config {
    /*! Base address of a device configuration register. */
    uintptr_t ddr;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_N1SDP_DDR_PHY_H */
