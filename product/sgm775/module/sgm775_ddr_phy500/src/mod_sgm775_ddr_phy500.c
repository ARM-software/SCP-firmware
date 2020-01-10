/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SGM775 DDR-PHY500 module
 */

#include <mod_sgm775_ddr_phy500.h>
#include <mod_sgm775_dmc500.h>

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
static int sgm775_ddr_phy500_config(fwk_id_t element_id)
{
    struct mod_sgm775_ddr_phy500_reg *ddr;
    const struct mod_sgm775_ddr_phy500_element_config *element_config;

    element_config = fwk_module_get_data(element_id);

    ddr = (struct mod_sgm775_ddr_phy500_reg *)element_config->ddr;

    FWK_LOG_INFO("[DDR] Initializing PHY at 0x%x", (uintptr_t)ddr);

    ddr->T_CTRL_DELAY   = 0x00000000;
    ddr->READ_DELAY     = 0x00000003;
    ddr->T_CTRL_UPD_MIN = 0x00000000;
    ddr->DELAY_SEL      = 0x0000000A;
    ddr->CAPTURE_MASK  = 0x0000001F;
    ddr->T_RDDATA_EN   = 0x00001C00;
    ddr->T_RDLAT       = 0x00000016;
    ddr->T_WRLAT       = 0x01000000;
    ddr->DFI_WR_PREMBL = 0x00000002;
    ddr->DFI_LP_ACK    = 0x00641300;

    return FWK_SUCCESS;
}

static struct mod_sgm775_dmc_ddr_phy_api ddr_phy500_api = {
    .configure = sgm775_ddr_phy500_config,
};

/*
 * Framework APIs
 */

static int sgm775_ddr_phy500_init(fwk_id_t module_id,
    unsigned int element_count, const void *config)
{
    return FWK_SUCCESS;
}

static int sgm775_ddr_phy500_element_init(fwk_id_t element_id,
    unsigned int unused, const void *data)
{
    fwk_assert(data != NULL);

    return FWK_SUCCESS;
}

static int sgm775_ddr_phy500_process_bind_request(fwk_id_t requester_id,
    fwk_id_t id, fwk_id_t api_id, const void **api)
{
    /* Only binding to module is permitted */
    if (!fwk_module_is_valid_module_id(id))
        return FWK_E_ACCESS;

    *((const void**)api) = &ddr_phy500_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_sgm775_ddr_phy500 = {
    .name = "SGM775-DDR-PHY500",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = sgm775_ddr_phy500_init,
    .element_init = sgm775_ddr_phy500_element_init,
    .process_bind_request = sgm775_ddr_phy500_process_bind_request,
    .api_count = 1,
};
