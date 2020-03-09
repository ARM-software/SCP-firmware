/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP DDR-PHY driver
 */
#ifndef PHY_VALUES_H
#define PHY_VALUES_H

#include <internal/n1sdp_ddr_phy.h>

#include <mod_n1sdp_dmc620.h>

#include <stdint.h>

/*
 * \brief Function to configure and run DDR PHY at 800MHz frequency.
 *
 * \param ddr_phy Pointer to DDR PHY register space.
 * \param info Pointer to DIMM information.
 *
 * \retval NONE
 */
void ddr_phy_config_800(struct mod_n1sdp_ddr_phy_reg *ddr_phy,
                        struct dimm_info *info);

/*
 * \brief Function to configure and run DDR PHY at 1200MHz frequency.
 *
 * \param ddr_phy Pointer to DDR PHY register space.
 * \param info Pointer to DIMM information.
 * \param dmc_id Identifier of the DMC instance.
 *
 * \retval NONE
 */
void ddr_phy_config_1200(struct mod_n1sdp_ddr_phy_reg *ddr_phy,
                        struct dimm_info *info, int dmc_id);

/*
 * \brief Function to configure and run DDR PHY at 1333MHz frequency.
 *
 * \param ddr_phy Pointer to DDR PHY register space.
 * \param info Pointer to DIMM information.
 * \param dmc_id Identifier of the DMC instance.
 *
 * \retval NONE
 */
void ddr_phy_config_1333(struct mod_n1sdp_ddr_phy_reg *ddr_phy,
                        struct dimm_info *info, int dmc_id);

#endif /* PHY_VALUES_H */
