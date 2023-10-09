/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_CONFIG_SENSOR_SMCF_DRV_H
#define TEST_CONFIG_SENSOR_SMCF_DRV_H

#include <mod_sensor_smcf_drv.h>

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define SENSOR_DATA_MAX_SZ_IN_BYTES 28
#define SENSOR_DATA_MAX_SZ_IN_WORDS 7

enum sensor_device_set {
    SENSOR_DEVICE0,
    SENSOR_DEVICE1,
    SENSOR_DEVICE2,
    SENSOR_DEVICE_CNT,
};

static struct sensor_smcf_drv_element_config
    device_config_table[SENSOR_DEVICE_CNT] = {
    [SENSOR_DEVICE0] = {
        .smcf_mli_id = FWK_ID_SUB_ELEMENT(
            FWK_MODULE_IDX_SMCF, SENSOR_DEVICE_CNT + SENSOR_DEVICE0, 0),
        .max_samples_size = SENSOR_DATA_MAX_SZ_IN_BYTES,
    },
    [SENSOR_DEVICE1] = {
        .smcf_mli_id = FWK_ID_SUB_ELEMENT(
            FWK_MODULE_IDX_SMCF, SENSOR_DEVICE_CNT + SENSOR_DEVICE1, 0),
        .max_samples_size = SENSOR_DATA_MAX_SZ_IN_BYTES - sizeof(uint32_t),
    },
    [SENSOR_DEVICE2] = {
        .smcf_mli_id = FWK_ID_SUB_ELEMENT(
            FWK_MODULE_IDX_SMCF, SENSOR_DEVICE_CNT + SENSOR_DEVICE2, 0),
        .max_samples_size =
            SENSOR_DATA_MAX_SZ_IN_BYTES - (2 * sizeof(uint32_t)),
    },
};

static struct fwk_element test_element_table[SENSOR_DEVICE_CNT] = {
    [SENSOR_DEVICE0] = {
        .name = "device0 SENSOR element config",
        .data = &device_config_table[SENSOR_DEVICE0],
    },
    [SENSOR_DEVICE1] = {
        .name = "device1 SENSOR element config",
        .data = &device_config_table[SENSOR_DEVICE1],
    },
    [SENSOR_DEVICE2] = {
        .name = "device2 SENSOR element config",
        .data = &device_config_table[SENSOR_DEVICE2],
    }
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return test_element_table;
}

struct fwk_module_config config_sensor_smcf_drv = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

#endif /* TEST_CONFIG_SENSOR_SMCF_DRV_H */
