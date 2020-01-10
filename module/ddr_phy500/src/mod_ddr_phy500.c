/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     DDR-PHY500 driver
 */

#include <mod_ddr_phy500.h>

#ifdef BUILD_HAS_MOD_DMC500
#    include <mod_dmc500.h>
#endif

#ifdef BUILD_HAS_MOD_DMC620
#    include <mod_dmc620.h>
#endif

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

/*
 * Functions fulfilling this module's interface
 */
static int ddr_phy500_config(fwk_id_t element_id)
{
    struct mod_ddr_phy500_reg *ddr;
    const struct mod_ddr_phy500_module_config *module_config;
    const struct mod_ddr_phy500_element_config *element_config;


    module_config = fwk_module_get_data(fwk_module_id_ddr_phy500);

    element_config = fwk_module_get_data(element_id);

    ddr = (struct mod_ddr_phy500_reg *)element_config->ddr;

    FWK_LOG_INFO("[DDR] Initializing PHY at 0x%x", (uintptr_t)ddr);

    if (module_config->initialize_init_complete)
        ddr->INIT_COMPLETE = module_config->ddr_reg_val->INIT_COMPLETE;

    ddr->T_CTRL_DELAY   = module_config->ddr_reg_val->T_CTRL_DELAY;
    ddr->READ_DELAY     = module_config->ddr_reg_val->READ_DELAY;
    ddr->T_CTRL_UPD_MIN = module_config->ddr_reg_val->T_CTRL_UPD_MIN;
    ddr->DELAY_SEL      = module_config->ddr_reg_val->DELAY_SEL;

    ddr->CAPTURE_MASK  = module_config->ddr_reg_val->CAPTURE_MASK;
    ddr->T_RDDATA_EN   = module_config->ddr_reg_val->T_RDDATA_EN;
    ddr->T_RDLAT       = module_config->ddr_reg_val->T_RDLAT;
    ddr->T_WRLAT       = module_config->ddr_reg_val->T_WRLAT;
    ddr->DFI_WR_PREMBL = module_config->ddr_reg_val->DFI_WR_PREMBL;

    ddr->DFI_LP_ACK    = module_config->ddr_reg_val->DFI_LP_ACK;

    if (module_config->initialize_ref_en)
        ddr->REF_EN = module_config->ddr_reg_val->REF_EN;

    return FWK_SUCCESS;
}

static struct mod_dmc_ddr_phy_api ddr_phy500_api = {
    .configure = ddr_phy500_config,
};

/*
 * Functions fulfilling the framework's module interface
 */

static int ddr_phy500_init(fwk_id_t module_id, unsigned int element_count,
                           const void *config)
{
    assert(config != NULL);

    return FWK_SUCCESS;
}

static int ddr_phy500_element_init(fwk_id_t element_id, unsigned int unused,
                                   const void *data)
{
    assert(data != NULL);

    return FWK_SUCCESS;
}

static int ddr_phy500_bind(fwk_id_t id, unsigned int round)
{
    /* Skip the second round (rounds are zero-indexed) */
    if (round == 1)
        return FWK_SUCCESS;

    /* Nothing to be done for element-level binding */
    if (fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    return FWK_SUCCESS;
}

static int ddr_phy500_process_bind_request(fwk_id_t requester_id, fwk_id_t id,
    fwk_id_t api_type, const void **api)
{
    /* Binding to elements is not permitted. */
    if (fwk_module_is_valid_element_id(id))
        return FWK_E_ACCESS;

    *((const void**)api) = &ddr_phy500_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_ddr_phy500 = {
    .name = "DDR-PHY500",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = ddr_phy500_init,
    .element_init = ddr_phy500_element_init,
    .bind = ddr_phy500_bind,
    .process_bind_request = ddr_phy500_process_bind_request,
    .api_count = 1,
};
