/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ssc_reg.h>

#include <mod_ssc.h>
#include <mod_system_info.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

static bool initialized;
static int chip_id, multi_chip_mode, part_number, revision_number;
static struct mod_system_info sys_info;

static struct mod_system_info *get_driver_data(void)
{
    if (!initialized)
        return NULL;

    return &sys_info;
}

/*
 * API for the use of system info module to get the driver data.
 */
static struct mod_system_info_get_driver_data_api get_sys_info_driver_data = {
    .get_driver_data = get_driver_data
};

/*
 * Framework Handlers
 */
static int ssc_init(fwk_id_t module_id, unsigned int count,
                          const void *data)
{
    const struct mod_ssc_config *config = data;
    struct ssc_reg *ssc_reg;

    if ((config == NULL) || (config->ssc_base == 0))
        return FWK_E_DATA;

    ssc_reg = (struct ssc_reg *)config->ssc_base;

    chip_id = (ssc_reg->SSC_CHIPID_ST) & SSC_CHIPID_ST_CHIP_ID_MASK;
    multi_chip_mode = ((ssc_reg->SSC_CHIPID_ST) &
                       SSC_CHIPID_ST_MULTI_CHIP_MODE_MASK) >>
                      SSC_CHIPID_ST_MULTI_CHIP_MODE_POS;
    part_number = (ssc_reg->PID0 & SSC_PID0_PART_NUMBER_MASK) |
                  ((ssc_reg->PID1 & SSC_PID1_PART_NUMBER_MASK) <<
                   SSC_PID1_PART_NUMBER_POS);
    revision_number = (ssc_reg->PID2 & SSC_PID2_REVISION_NUMBER_MASK) >>
                      SSC_PID2_REVISION_NUMBER_POS;

    if (config->ssc_debug_cfg_set != 0)
        ssc_reg->SSC_DBGCFG_SET = config->ssc_debug_cfg_set;

    /* Populate the system info structure */
    sys_info.product_id = part_number;
    sys_info.config_id = revision_number;
    sys_info.chip_id = chip_id;
    sys_info.multi_chip_mode = multi_chip_mode;
    sys_info.name = config->product_name;

    initialized = true;

    return FWK_SUCCESS;
}

static int ssc_process_bind_request(fwk_id_t requester_id, fwk_id_t targer_id,
        fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_SSC_SYSTEM_INFO_DRIVER_DATA_API_IDX:
        *api = &get_sys_info_driver_data;
        break;
    default:
        return FWK_E_PARAM;
    }
    return FWK_SUCCESS;
}

const struct fwk_module module_ssc = {
    .name = "Serial Security Control",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = ssc_init,
    .process_bind_request = ssc_process_bind_request,
    .api_count = MOD_SSC_API_COUNT,
};
