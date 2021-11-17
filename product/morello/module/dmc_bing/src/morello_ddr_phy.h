/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Morello DDR-PHY driver
 */
#ifndef MORELLO_DDR_PHY_H
#define MORELLO_DDR_PHY_H

#include <internal/morello_ddr_phy_reg.h>

#include <mod_dmc_bing.h>

#include <stdint.h>

/*
 * \brief Function to configure and run DDR PHY at 800MHz frequency.
 *
 * \param ddr_phy Pointer to DDR PHY register space.
 * \param info Pointer to DIMM information.
 *
 * \retval NONE
 */
void ddr_phy_config_800(
    struct mod_morello_ddr_phy_reg *ddr_phy,
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
void ddr_phy_config_1200(
    struct mod_morello_ddr_phy_reg *ddr_phy,
    struct dimm_info *info,
    int dmc_id);

/*
 * \brief Function to configure and run DDR PHY at 1333MHz frequency.
 *
 * \param ddr_phy Pointer to DDR PHY register space.
 * \param info Pointer to DIMM information.
 * \param dmc_id Identifier of the DMC instance.
 *
 * \retval NONE
 */
void ddr_phy_config_1333(
    struct mod_morello_ddr_phy_reg *ddr_phy,
    struct dimm_info *info,
    int dmc_id);

/*
 * \brief Function to configure and run DDR PHY at 1466MHz frequency.
 *
 * \param ddr_phy Pointer to DDR PHY register space.
 * \param info Pointer to DIMM information.
 * \param dmc_id Identifier of the DMC instance.
 *
 * \retval NONE
 */
void ddr_phy_config_1466(
    struct mod_morello_ddr_phy_reg *ddr_phy,
    struct dimm_info *info,
    int dmc_id);

/*!
 * \brief Configure a DDR physical device
 *
 * \param element_id Element identifier corresponding to the device to
 *      configure.
 *
 * \retval FWK_SUCCESS if the operation succeed.
 * \return one of the error code otherwise.
 */
int morello_ddr_phy_config(fwk_id_t element_id, struct dimm_info *info);

/*!
 * \brief Post training setting for DDR physical device
 *
 * \param element_id Element identifier corresponding to the device to
 *      configure.
 *
 * \retval FWK_SUCCESS if the operation succeed.
 * \return one of the error code otherwise.
 */
int morello_ddr_phy_post_training_configure(
    fwk_id_t element_id,
    struct dimm_info *info);

/*!
 * \brief API to verify DDR PHY status at different training stage
 *
 * \param element_id Element identifier corresponding to the device to
 *      configure.
 * \param training_type Training type for which PHY status to be verified.
 *
 * \retval FWK_SUCCESS if the operation succeed.
 * \return one of the error code otherwise.
 */
int morello_verify_phy_status(
    fwk_id_t element_id,
    uint8_t training_type,
    struct dimm_info *info);

/*!
 * \brief API to tune write leveling registers
 *
 * \param element_id Element identifier corresponding to the device to
 *      configure.
 * \param rank The rank number to perform the tuning.
 *
 * \retval FWK_SUCCESS if the operation succeed.
 * \return one of the error code otherwise.
 */
int morello_wrlvl_phy_obs_regs(
    fwk_id_t element_id,
    uint32_t rank,
    struct dimm_info *info);

/*!
 * \brief API to tune read leveling registers
 *
 * \param element_id Element identifier corresponding to the device to
 *      configure.
 * \param rank The rank number to perform the tuning.
 *
 * \retval FWK_SUCCESS if the operation succeed.
 * \return one of the error code otherwise.
 */
int morello_read_gate_phy_obs_regs(
    fwk_id_t element_id,
    uint32_t rank,
    struct dimm_info *info);

/*!
 * \brief API to tune PHY training registers
 *
 * \param element_id Element identifier corresponding to the device to
 *      configure.
 * \param rank The rank number to perform the tuning.
 *
 * \retval FWK_SUCCESS if the operation succeed.
 * \return one of the error code otherwise.
 */
int morello_phy_obs_regs(
    fwk_id_t element_id,
    uint32_t rank,
    struct dimm_info *info);

#endif /* MORELLO_DDR_PHY_H */
