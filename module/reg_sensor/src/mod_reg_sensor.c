/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <mod_reg_sensor.h>
#include <mod_sensor.h>

static struct mod_reg_sensor_dev_config **config_table;

/*
 * Module API
 */
static int get_value(fwk_id_t id, uint64_t *value)
{
    int status;
    struct mod_reg_sensor_dev_config *config;

    status = fwk_module_check_call(id);
    if (status != FWK_SUCCESS) {
        assert(false);
        return status;
    }

    config = config_table[fwk_id_get_element_idx(id)];

    if (value == NULL) {
        assert(false);
        return FWK_E_PARAM;
    }

    *value = *(uint64_t*)config->reg;

    return FWK_SUCCESS;
}

static int get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    int status;
    struct mod_reg_sensor_dev_config *config;

    status = fwk_module_check_call(id);
    if (status != FWK_SUCCESS)
        return status;

    config = config_table[fwk_id_get_element_idx(id)];
    fwk_assert(config != NULL);

    if (info == NULL)
        return FWK_E_PARAM;

    *info = *(config->info);

    return FWK_SUCCESS;
}

static const struct mod_sensor_driver_api reg_sensor_api = {
    .get_value = get_value,
    .get_info = get_info,
};

/*
 * Framework handlers
 */
static int reg_sensor_init(fwk_id_t module_id,
                           unsigned int element_count,
                           const void *unused)
{
    config_table = fwk_mm_alloc(element_count, sizeof(*config_table));

    if (config_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int reg_sensor_element_init(fwk_id_t element_id,
                                   unsigned int sub_element_count,
                                   const void *data)
{
    struct mod_reg_sensor_dev_config *config =
        (struct mod_reg_sensor_dev_config *)data;

    if (config->reg == 0)
        return FWK_E_DATA;

    config_table[fwk_id_get_element_idx(element_id)] = config;

    return FWK_SUCCESS;
}

static int reg_sensor_process_bind_request(fwk_id_t source_id,
                                           fwk_id_t target_id,
                                           fwk_id_t api_type,
                                           const void **api)
{
    *api = &reg_sensor_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_reg_sensor = {
    .name = "Register Sensor",
    .api_count = 1,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = reg_sensor_init,
    .element_init = reg_sensor_element_init,
    .process_bind_request = reg_sensor_process_bind_request,
};
