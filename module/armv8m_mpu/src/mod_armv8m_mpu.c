/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mod_armv8m_mpu.h"

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

static int armv8m_mpu_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    const struct mod_armv8m_mpu_config *config;
    uint8_t attr_index;
    int status;

    fwk_assert(element_count == 0);
    fwk_assert(data != NULL);

    config = data;

    if (config->attributes_count <= (uint8_t)MPU_MAX_ATTR_COUNT) {
        ARM_MPU_Disable();

        for (attr_index = 0U; attr_index < config->attributes_count;
             attr_index++) {
            ARM_MPU_SetMemAttr(attr_index, config->attributes[attr_index]);
        }
        ARM_MPU_Load(
            config->first_region_number, config->regions, config->region_count);
        ARM_MPU_Enable(MPU_CTRL_HFNMIENA_Msk);

        status = FWK_SUCCESS;
    } else {
        status = FWK_E_RANGE;
    }

    return status;
}

/* Module description */
const struct fwk_module module_armv8m_mpu = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = armv8m_mpu_init,
};
