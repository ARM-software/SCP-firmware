/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for direct ATU configuration.
 */

#ifndef ATU_MANAGE_UTILS_INTERNAL_H
#define ATU_MANAGE_UTILS_INTERNAL_H

#include <mod_atu.h>

#include <stdint.h>

/*!
 * \brief Get available ATU region index.
 *
 * \param device_ctx Pointer to the ATU device context.
 * \param[out] region_idx Index of the available ATU region.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_SUPPORT No available ATU regions.
 */
int atu_get_available_region_idx(void *device_ctx, uint8_t *region_idx);

/*!
 * \brief Validate the translation region.
 *
 * \param map Mapping of the region to be checked.
 * \param device_ctx Pointer to the ATU device context.
 *
 * \retval ::FWK_SUCCESS The region can be mapped.
 * \retval ::FWK_E_PARAM The region does not align with the ATU page size.
 *         configure this region.
 * \retval ::FWK_E_SUPPORT The region overlaps with an existing translation
 *          region.
 */
int atu_validate_region(const struct atu_region_map *region, void *device_ctx);

/*!
 * \brief Map a new translation region.
 *
 * \param region Mapping of the translation region to be added.
 * \param region_idx Index of the translation region to be mapped.
 * \param device_ctx Pointer to the ATU device context.
 *
 * \retval ::FWK_SUCCESS The requested translation region has been mapped.
 * \retval ::FWK_E_SUPPORT The translation region exceeds the number of active
 *         translation regions supported by the ATU.
 */
int atu_map_region(
    const struct atu_region_map *region,
    uint8_t region_idx,
    void *device_ctx);

/*!
 * \brief Unmap a translation region.
 *
 * \param region_idx Index of the translation region to be removed.
 * \param device_ctx Pointer to the ATU device context.
 *
 * \retval ::FWK_SUCCESS The translation region has been unmapped.
 * \retval ::FWK_E_SUPPORT The translation region exceeds the number of active
 *         translation regions supported by the ATU.
 */
int atu_unmap_region(uint8_t region_idx, void *device_ctx);

#endif /* ATU_MANAGE_UTILS_INTERNAL_H */
