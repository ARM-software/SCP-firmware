/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_armv7m_mpu.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

static int armv7m_mpu_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    const struct mod_armv7m_mpu_config *config;

    assert(element_count == 0);
    assert(data != NULL);

    config = data;

    ARM_MPU_Disable();
    ARM_MPU_Load(config->regions, config->region_count);
    ARM_MPU_Enable(MPU_CTRL_HFNMIENA_Msk);

    return FWK_SUCCESS;
}

/* Module description */
const struct fwk_module module_armv7m_mpu = {
    .name = "ARMV7M_MPU",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = armv7m_mpu_init,
};
