/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_mm.h>
#include <mod_pcid.h>
#include <mod_sid.h>
#include <sid_reg.h>

static bool initialized;
static struct mod_sid_info info;

int mod_sid_get_system_info(const struct mod_sid_info **system_info)
{
    if (initialized) {
        *system_info = &info;

        return FWK_SUCCESS;
    }

    return FWK_E_INIT;
}

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

    info.config_number = sid_reg->SYSTEM_CFG;
    info.node_id = sid_reg->NODE_ID & SID_SYS_NODE_ID_IDENTIFIER_MASK;

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

        initialized = true;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_sid = {
    .name = "SID",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = sid_init,
    .element_init = sid_subsystem_init,
};
