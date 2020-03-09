/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno DDR-PHY400 driver
 */

#include "juno_scc.h"

#include <mod_juno_ddr_phy400.h>
#include <mod_juno_dmc400.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

static struct mod_timer_api *timer_api;

#define DDR_PHY_400_CONFIG_IDLE_WAIT_TIMEOUT_US     1000
#define DDR_PHY_PTM_PHY_STATUS_IDLE                 UINT32_C(0xFFFFF7FF)

/*
 * Module API functions
 */

static int juno_ddr_phy400_config_ddr(fwk_id_t element_id)
{
    struct mod_juno_ddr_phy400_ptm_reg *phy_ptm;
    struct mod_juno_ddr_phy400_c3a_reg *phy_c3a;
    struct mod_juno_ddr_phy400_bl_reg *phy_bl0;
    const struct mod_juno_ddr_phy400_element_config *element_config;

    element_config = fwk_module_get_data(element_id);
    if (element_config == NULL)
        return FWK_E_DATA;

    phy_ptm = (struct mod_juno_ddr_phy400_ptm_reg *)element_config->ddr_phy_ptm;
    phy_c3a = (struct mod_juno_ddr_phy400_c3a_reg *)element_config->ddr_phy_c3a;
    phy_bl0 = (struct mod_juno_ddr_phy400_bl_reg *)element_config->ddr_phy_bl0;

    if ((phy_ptm == NULL) || (phy_c3a == NULL) || (phy_bl0 == NULL))
        return FWK_E_DATA;

    /* All writes to BL0 are copied to BL1-3 */
    phy_ptm->BL_APB_CTRL = 0x00000001;

    /*
     * Phase 1: DLL frequency range selection
     */
    phy_ptm->PHY_MISC_CTRL = 0x00000002;
    phy_bl0->DLL_MISC_CTRL = 0x00000500;

    /*
     * Phase 2: Matched on-chip CK path wrt DQS
     *        : clkgate_en = 0, ck_dqs_tmatch = 1
     *        : clkgate_en = 1
     */
    phy_c3a->CAC_CK_CTRL1 = 0x80000004;

    /*
     * Phase 3: Disable edge detection during write leveling
     */
    phy_bl0->WRLVL_CTRL = 0x000AC25C;
    phy_bl0->SQUELCH_CTRL = 0x0001A804;

    /*
     * Phase 4: Disable write leveling tracking
     */
    phy_ptm->WRLVL_TRK_DC_DRIFT_CTRL = 0x00000004;

    /*
     * Phase 5: Set VT sleep duration to 1ms
     */
    phy_ptm->PHY_LP_CTRL = 0x06A8D190;

    /*
     * Phase 6: Set VT tracking interval to 1s
     */
    phy_ptm->PHY_TRK_CTRL = 0x00000FB5;

    /*
     * Phase 7: Read leveling and Write PBE training
     *        : averaging iteration to 8
     */
    phy_bl0->RDLVL_WREQ_CTRL = 0x00001517;

    /*
     * Phase 8: Power down CKE, ODT, CS and CK Pads
     *        : CKE 2
     *        : ODT 2
     *        : CS  2
     *        : CK  1
     */
    phy_c3a->IO_CTRL1 = 0x001FCCF3;

    /*
     * Phase 9: DDR3 type, speed, OCD and ODT
     *        : DDR3L up-to 1600 Mbps
     *        : OCD   34 Ohm
     *        : ODT   30 Ohm
     */
    phy_c3a->IO_CTRL2 = 0x0226FFFF;
    phy_bl0->IO_CTRL  = 0x000002A6;

    /*
     * Phase 10: Copy training values of CS0 -> CS1
     */
    phy_bl0->RDLVL_GATE_RANK_MAPPING = 0x000000E0;
    phy_bl0->WRLVL_RANK_MAPPING = 0x000000E0;
    phy_bl0->RDLVL_RANK_MAPPING = 0x000000E0;
    phy_bl0->WREQ_RANK_MAPPING = 0x000000E0;

    /*
     * Phase 11: Set WL and RL for write equalization
     *         : WL = 8
     *         : RL = 11
     *         : 2T
     *         : tRFC = 280
     */
    phy_ptm->PHY_AU_CTRL1 = 0x000C6168;

    /*
     * Phase 12: Set scratch base address for write equalization
     *         : wreq_ca [30:19]
     *         : wreq_ra [18:3]
     *         : wreq_ba [2:0]
     *         : Training Base: 0xFFFF_8000
     */
    phy_ptm->PHY_AU_CTRL2 = 0x0001FFF8;

    return FWK_SUCCESS;
}

static int juno_ddr_phy400_config_clk(fwk_id_t module_id)
{
    /* Complete clock settings */
    SCC->DDR_PHY0_PLL = SCC_DDR_PHY_PLL_BYPASS_EN;
    SCC->DDR_PHY1_PLL = SCC_DDR_PHY_PLL_BYPASS_EN;

    return FWK_SUCCESS;
}

static bool ddr_phy_idle_check(void *data)
{
    struct mod_juno_ddr_phy400_ptm_reg *phy_ptm =
        (struct mod_juno_ddr_phy400_ptm_reg *)data;

    return (!(phy_ptm->PHY_STATUS & DDR_PHY_PTM_PHY_STATUS_IDLE));
}

static int juno_ddr_phy400_config_idle(fwk_id_t element_id)
{
    struct mod_juno_ddr_phy400_ptm_reg *phy_ptm;
    const struct mod_juno_ddr_phy400_element_config * element_config;
    const struct mod_juno_ddr_phy400_config *module_config;

    module_config = fwk_module_get_data(fwk_module_id_juno_ddr_phy400);

    element_config = fwk_module_get_data(element_id);
    if (element_config == NULL)
        return FWK_E_DATA;

    phy_ptm = (struct mod_juno_ddr_phy400_ptm_reg *)element_config->ddr_phy_ptm;

    return timer_api->wait(module_config->timer_id,
                           DDR_PHY_400_CONFIG_IDLE_WAIT_TIMEOUT_US,
                           ddr_phy_idle_check,
                           phy_ptm);
}

static int juno_ddr_phy400_config_retention(fwk_id_t module_id, bool enable)
{
    if (enable) {
        SCC->DDR_PHY0_RETNCTRL = SCC_DDR_PHY_RETNCTRL_ENABLE;
        SCC->DDR_PHY1_RETNCTRL = SCC_DDR_PHY_RETNCTRL_ENABLE;
    } else {
        SCC->DDR_PHY0_RETNCTRL = SCC_DDR_PHY_RETNCTRL_DISABLE;
        SCC->DDR_PHY1_RETNCTRL = SCC_DDR_PHY_RETNCTRL_DISABLE;
    }

    return FWK_SUCCESS;
}

static struct mod_juno_dmc400_ddr_phy_api ddr_phy400_api = {
    .configure_ddr = juno_ddr_phy400_config_ddr,
    .configure_clk = juno_ddr_phy400_config_clk,
    .configure_idle = juno_ddr_phy400_config_idle,
    .configure_retention = juno_ddr_phy400_config_retention,
};

/*
 * Framework handlers
 */
static int juno_ddr_phy400_init(fwk_id_t module_id,
                                unsigned int element_count,
                                const void *config)
{
    return FWK_SUCCESS;
}

static int juno_ddr_phy400_element_init(fwk_id_t element_id,
                                        unsigned int unused,
                                        const void *data)
{
    fwk_assert(data != NULL);

    return FWK_SUCCESS;
}

static int juno_ddr_phy400_bind(fwk_id_t id, unsigned int round)
{
    int status;
    const struct mod_juno_ddr_phy400_config *module_config;

    /* Skip the second round */
    if (round == 1)
        return FWK_SUCCESS;

    /* Nothing to be done for element-level binding */
    if (fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    module_config = fwk_module_get_data(fwk_module_id_juno_ddr_phy400);
    fwk_assert(module_config != NULL);

    status = fwk_module_bind(module_config->timer_id,
        MOD_TIMER_API_ID_TIMER, &timer_api);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int juno_ddr_phy400_process_bind_request(fwk_id_t requester_id,
                                                fwk_id_t id,
                                                fwk_id_t api_type,
                                                const void **api)
{
    /* Binding to elements is not permitted. */
    if (fwk_module_is_valid_element_id(id))
        return FWK_E_ACCESS;

    *api = &ddr_phy400_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_juno_ddr_phy400 = {
    .name = "JUNO DDR-PHY400",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = juno_ddr_phy400_init,
    .element_init = juno_ddr_phy400_element_init,
    .bind = juno_ddr_phy400_bind,
    .process_bind_request = juno_ddr_phy400_process_bind_request,
    .api_count = 1,
};
