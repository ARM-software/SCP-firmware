/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef NOC_S3_PSAM_H
#define NOC_S3_PSAM_H

#include <mod_noc_s3.h>

#include <stdint.h>

/*!
 * \brief Map the list of statically mapped regions in the NoC S3.
 *
 * \details This function maps all the regions in the config in the non hash
 *          region. It assumes that the NoC S3 is being programmed for the first
 *          time and start from the region 0 and all the regions are non
 *          overlapping.
 *
 * \param[in] config Configuration to region memory map in the target NoC S3 NCI
 *                   block.
 * \param[in] dev    Device handler.
 *
 * \return FWK_E_SUCCESS Regions mapped successfully.
 * \return FWK_E_PARAM   An invalid parameter was encountered.
 * \return FWK_E_RANGE   Out of regions.
 */
int program_static_mapped_regions(
    struct mod_noc_s3_element_config *config,
    struct mod_noc_s3_dev *dev);

/*!
 * \brief Map a region in the target xSNI node.
 *
 * \details This API maps a carveout in xSNI's PSAM. The function looks for the
 *          next immediate available region to map. It also updates the region
 *          number in the location passed by the caller. This function also
 *          checks for overlap between the carveouts.
 *
 * \param[in] dev          Device handler containing base address of the
 *                         registers to configure NoC S3.
 * \param[in] comp_config  Configuration info that contains the target and
 *                         source node ids and carveout's base and size.
 * \param[out] region[out] Index of the mapped PSAM region.
 *
 * \return FWK_E_SUCCESS Regions mapped successfully.
 * \return FWK_E_PARAM   An invalid parameter was encountered.
 * \return FWK_E_RANGE   Out of regions.
 */
int map_region_in_psam(
    struct mod_noc_s3_dev *dev,
    struct mod_noc_s3_comp_config *comp_config,
    uint8_t *region);

/*!
 * \brief Disable and reset the register of the region in PSAM.
 *
 * \param[in] dev         Device handler containing base address of the
 *                        registers to configure NoC S3.
 * \param[in] comp_config Configuration info that contains the target and source
 *                        node ids and carveout's base and size.
 * \param[in] region      Index of the region
 *
 * \return FWK_E_SUCCESS Regions mapped successfully.
 * \return FWK_E_PARAM   An invalid parameter was encountered.
 * \return FWK_E_RANGE   Out of regions.
 */
int unmap_region_in_psam(
    struct mod_noc_s3_dev *dev,
    struct mod_noc_s3_comp_config *comp_config,
    uint8_t region);

#endif /* NOC_S3_PSAM_H */
