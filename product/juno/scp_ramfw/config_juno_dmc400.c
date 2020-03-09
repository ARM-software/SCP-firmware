/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "juno_id.h"
#include "system_mmap.h"

#include <mod_juno_dmc400.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

/* All Juno variants but A */
#define COL_BITS                2
#define ROW_BITS                5
#define BANK_BITS               3
#define CHIP_BITS               0
#define CHANN_BITS              1

/* Juno variant A only */
#define COL_BITS_VARIANT_A      1
#define ROW_BITS_VARIANT_A      5
#define BANK_BITS_VARIANT_A     3
#define CHIP_BITS_VARIANT_A     1
#define CHANN_BITS_VARIANT_A    1

/* DDR layout */
#define DDR_CHIP_COUNT              1
#define DDR_CHIP_COUNT_VARIANT_A    2

static struct fwk_element juno_dmc400_element_table[] = {
    [0] = {
        .name = "",
        .data = &((struct mod_juno_dmc400_element_config) {
            .dmc = DMC400_BASE,
            .ddr_phy_0_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_DDR_PHY400,
                0),
            .ddr_phy_1_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_DDR_PHY400,
                1),
            .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
                POWER_DOMAIN_IDX_SYSTOP),
        }),
    },

    [1] = { 0 }, /* Termination description. */
};

static const struct fwk_element *juno_dmc400_get_element_table
    (fwk_id_t module_id)
{
    int status;
    struct mod_juno_dmc400_element_config *dmc400_config;

    enum juno_idx_platform platform_id = JUNO_IDX_PLATFORM_COUNT;
    enum juno_idx_variant variant_id = JUNO_IDX_VARIANT_A;
    enum juno_idx_revision revision = JUNO_IDX_REVISION_COUNT;

    dmc400_config = (struct mod_juno_dmc400_element_config *)
        juno_dmc400_element_table[0].data;

    status = juno_id_get_revision(&revision);
    if (status != FWK_SUCCESS)
        return NULL;

    /* On R2 boards only, the DDR is different for variants A and B */
    if (revision == JUNO_IDX_REVISION_R2) {
        status = juno_id_get_variant(&variant_id);
        if (!fwk_expect(status == FWK_SUCCESS))
            return NULL;
    }

    if (variant_id == JUNO_IDX_VARIANT_A) {
        dmc400_config->address_control =
            ADDRESS_CONTROL_VAL(CHANN_BITS_VARIANT_A,
                                CHIP_BITS_VARIANT_A,
                                BANK_BITS_VARIANT_A,
                                ROW_BITS_VARIANT_A,
                                COL_BITS_VARIANT_A);

        dmc400_config->ddr_chip_count = DDR_CHIP_COUNT_VARIANT_A;
    } else {
        dmc400_config->address_control =
            ADDRESS_CONTROL_VAL(CHANN_BITS,
                                CHIP_BITS,
                                BANK_BITS,
                                ROW_BITS,
                                COL_BITS);

        dmc400_config->ddr_chip_count = DDR_CHIP_COUNT;
    }

    status = juno_id_get_platform(&platform_id);
    if (!fwk_expect(status == FWK_SUCCESS))
        return NULL;

    dmc400_config->is_platform_fvp =
        (platform_id == JUNO_IDX_PLATFORM_FVP);

    return juno_dmc400_element_table;
}

/* Configuration of the Juno DMC400 module */
struct fwk_module_config config_juno_dmc400 = {
    .get_element_table = juno_dmc400_get_element_table,
    .data = &((struct mod_juno_dmc400_module_config) {
        .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
        .ddr_phy_module_id =
            FWK_ID_MODULE_INIT(FWK_MODULE_IDX_JUNO_DDR_PHY400),
        .ddr_phy_api_id =
            FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_DDR_PHY400, 0),
    })
};
