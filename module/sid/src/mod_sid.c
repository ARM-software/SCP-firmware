/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sid_reg.h>

#include <mod_pcid.h>
#include <mod_sid.h>
#include <mod_system_info.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

static bool initialized;
static struct mod_sid_info info;
static struct mod_system_info sys_info;

int mod_sid_get_system_info(const struct mod_sid_info **system_info)
{
    if (initialized) {
        *system_info = &info;

        return FWK_SUCCESS;
    }

    return FWK_E_INIT;
}

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
static int sid_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    const struct mod_sid_config *config = data;
    struct sid_reg *sid_reg;

    if ((config == NULL) || (config->sid_base == 0) || (element_count == 0)) {
        assert(false);
        return FWK_E_DATA;
    }

    sid_reg = (struct sid_reg *)config->sid_base;

    assert(mod_pcid_check_registers(&sid_reg->pcid, &config->pcid_expected));

    info.system_major_revision =
        (sid_reg->SYSTEM_ID & SID_SYS_SOC_ID_MAJOR_REVISION_MASK)
        >> SID_SYS_SOC_ID_MAJOR_REVISION_POS;

    info.system_minor_revision =
        (sid_reg->SYSTEM_ID & SID_SYS_SOC_ID_MINOR_REVISION_MASK)
        >> SID_SYS_SOC_ID_MINOR_REVISION_POS;

    info.system_designer_id =
        (sid_reg->SYSTEM_ID & SID_SYS_SOC_ID_DESIGNER_ID_MASK)
        >> SID_SYS_SOC_ID_DESIGNER_ID_POS;

    info.system_part_number =
        sid_reg->SYSTEM_ID & SID_SYS_SOC_ID_PART_NUMBER_MASK;

    info.soc_major_revision =
        (sid_reg->SOC_ID & SID_SYS_SOC_ID_MAJOR_REVISION_MASK)
        >> SID_SYS_SOC_ID_MAJOR_REVISION_POS;

    info.soc_minor_revision =
        (sid_reg->SOC_ID & SID_SYS_SOC_ID_MINOR_REVISION_MASK)
        >> SID_SYS_SOC_ID_MINOR_REVISION_POS;

    info.soc_designer_id =
        (sid_reg->SOC_ID & SID_SYS_SOC_ID_DESIGNER_ID_MASK)
        >> SID_SYS_SOC_ID_DESIGNER_ID_POS;

    info.soc_part_number = sid_reg->SOC_ID & SID_SYS_SOC_ID_PART_NUMBER_MASK;

    info.multi_chip_mode = (sid_reg->NODE_ID & SID_SYS_MULTI_CHIP_MODE_MASK)
                            >> SID_SYS_MULTI_CHIP_MODE_POS;
    info.node_number = sid_reg->NODE_ID & SID_SYS_NODE_NUMBER_MASK;

    info.config_number = sid_reg->SYSTEM_CFG;

    return FWK_SUCCESS;
}

static int sid_subsystem_init(
    fwk_id_t subsystem_id,
    unsigned int unused,
    const void *data)
{
    const struct mod_sid_subsystem_config *subsystem_config;

    assert(data != NULL);

    subsystem_config = data;
    if (subsystem_config->part_number == info.system_part_number) {
        info.system_idx = fwk_id_get_element_idx(subsystem_id);
        info.name = fwk_module_get_name(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_SID, info.system_idx));

        /* Populate the system info structure */
        sys_info.product_id = info.system_part_number;
        sys_info.config_id = info.config_number;
        sys_info.chip_id = info.node_number;
        sys_info.multi_chip_mode = info.multi_chip_mode;
        sys_info.name = info.name;

        initialized = true;
    }

    return FWK_SUCCESS;
}

static int sid_process_bind_request(fwk_id_t requester_id, fwk_id_t targer_id,
        fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_SID_SYSTEM_INFO_DRIVER_DATA_API_IDX:
        *api = &get_sys_info_driver_data;
        break;
    default:
        return FWK_E_PARAM;
    }
    return FWK_SUCCESS;
}

const struct fwk_module module_sid = {
    .name = "SID",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = sid_init,
    .element_init = sid_subsystem_init,
    .process_bind_request = sid_process_bind_request,
    .api_count = MOD_SID_API_COUNT,
};
