/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Common utility functions for the ATU driver.
 */

#ifndef ATU_COMMON_INTERNAL_H
#define ATU_COMMON_INTERNAL_H

#include <stdint.h>

/*!
 * \brief Validate the output bus attributes for the ATU region.
 *
 * \param attributes output bus attributes value.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid attributes configuration.
 */
int atu_validate_region_attributes(uint32_t attributes);

#endif /* ATU_COMMON_INTERNAL_H */
