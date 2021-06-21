/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "morello_scc_reg.h"
#include "morello_scp_pik.h"

#include <morello_pcie.h>

#include <internal/pcie_ctrl_apb_reg.h>

#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

void pcie_phy_init(uint32_t phy_apb_base, uint32_t pcie_lane_count)
{
    *((unsigned int *)(0x3000c | phy_apb_base)) = 0x00000144;
    *((unsigned int *)(0x38000 | phy_apb_base)) = 0x00000040;
    *((unsigned int *)(0x38014 | phy_apb_base)) = 0x00000100;

    if (pcie_lane_count == LAN_COUNT_IN_X_1) {
        *((unsigned int *)(0x38038 | phy_apb_base)) = 0x0000FFFE;
        *((unsigned int *)(0x3803C | phy_apb_base)) = 0x00009000;
    } else if (pcie_lane_count == LAN_COUNT_IN_X_2) {
        *((unsigned int *)(0x38038 | phy_apb_base)) = 0x0000FFFC;
        *((unsigned int *)(0x3803C | phy_apb_base)) = 0x00009000;
    } else if (pcie_lane_count == LAN_COUNT_IN_X_4) {
        *((unsigned int *)(0x38038 | phy_apb_base)) = 0x0000FFF0;
        *((unsigned int *)(0x3803C | phy_apb_base)) = 0x00009000;
    } else if (pcie_lane_count == LAN_COUNT_IN_X_8) {
        *((unsigned int *)(0x38038 | phy_apb_base)) = 0x0000FF00;
        *((unsigned int *)(0x3803C | phy_apb_base)) = 0x00009000;
    } else { /* LAN_COUNT_IN_X_16 */
        *((unsigned int *)(0x38038 | phy_apb_base)) = 0x00000000;
        *((unsigned int *)(0x3803C | phy_apb_base)) = 0x00000000;
    }

    *((unsigned int *)(0x00140 | phy_apb_base)) = 0x00008804;
    *((unsigned int *)(0x00188 | phy_apb_base)) = 0x00001219;

    *((unsigned int *)(0x18008 | phy_apb_base)) = 0x000055A0;
    *((unsigned int *)(0x1800c | phy_apb_base)) = 0x00006910;
    *((unsigned int *)(0x18030 | phy_apb_base)) = 0x0000001D;
    *((unsigned int *)(0x1812C | phy_apb_base)) = 0x00000143;
    *((unsigned int *)(0x18204 | phy_apb_base)) = 0x0000813E;
    *((unsigned int *)(0x18214 | phy_apb_base)) = 0x00001978;
    *((unsigned int *)(0x18218 | phy_apb_base)) = 0x00000389;
    *((unsigned int *)(0x18220 | phy_apb_base)) = 0x0000038C;
    *((unsigned int *)(0x18238 | phy_apb_base)) = 0x00003A7A;
    *((unsigned int *)(0x18244 | phy_apb_base)) = 0x0000033C;
    *((unsigned int *)(0x1824C | phy_apb_base)) = 0x00000000;
    *((unsigned int *)(0x18258 | phy_apb_base)) = 0x00000003;
    *((unsigned int *)(0x1825C | phy_apb_base)) = 0x000022CC;
    *((unsigned int *)(0x182A8 | phy_apb_base)) = 0x00007FF6;
    *((unsigned int *)(0x182AC | phy_apb_base)) = 0x00007FF6;
    *((unsigned int *)(0x182C0 | phy_apb_base)) = 0x00007FBF;
    *((unsigned int *)(0x182C4 | phy_apb_base)) = 0x00007FBF;
    *((unsigned int *)(0x182C8 | phy_apb_base)) = 0x00007FD8;
    *((unsigned int *)(0x18350 | phy_apb_base)) = 0x00008C67;
    *((unsigned int *)(0x18354 | phy_apb_base)) = 0x00008C67;
    *((unsigned int *)(0x18378 | phy_apb_base)) = 0x00002437;
    *((unsigned int *)(0x18388 | phy_apb_base)) = 0x00003C2C;

    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000000;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00000203;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000001;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00000403;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000002;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00000603;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000003;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00000803;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000004;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00000A03;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000005;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00000C03;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000006;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00000E03;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000007;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00000F03;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000008;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001003;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000009;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001103;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000000A;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00001E06;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001203;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000000B;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001303;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000000C;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001403;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000000D;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001503;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000000E;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00002A06;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001603;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000000F;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00002D06;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001703;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000010;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00002E06;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001803;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000011;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001903;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000012;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003006;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001A03;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000013;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003106;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001B03;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000014;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003106;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001C03;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000015;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003106;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001D03;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000016;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003106;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001E03;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000017;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003106;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00001F03;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000018;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003206;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00002003;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x00000019;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003306;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00002103;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000001A;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003606;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00002203;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000001B;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003906;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00002303;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000001C;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003B06;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00002403;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000001D;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00003F06;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00002603;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000001E;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00004306;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00002803;
    *((unsigned int *)(0x183A0 | phy_apb_base)) = 0x0000001F;
    *((unsigned int *)(0x183A4 | phy_apb_base)) = 0x00004706;
    *((unsigned int *)(0x183A8 | phy_apb_base)) = 0x00002A03;

    *((unsigned int *)(0x183D4 | phy_apb_base)) = 0x00001560;
    *((unsigned int *)(0x183E0 | phy_apb_base)) = 0x00001560;
    *((unsigned int *)(0x183E8 | phy_apb_base)) = 0x000000BC;
    *((unsigned int *)(0x183EC | phy_apb_base)) = 0x000000A6;
    *((unsigned int *)(0x183F0 | phy_apb_base)) = 0x000001B6;
    *((unsigned int *)(0x183FC | phy_apb_base)) = 0x00000333;
    *((unsigned int *)(0x18400 | phy_apb_base)) = 0x00000738;
    *((unsigned int *)(0x18404 | phy_apb_base)) = 0x00000665;
    *((unsigned int *)(0x18408 | phy_apb_base)) = 0x000009AC;
    *((unsigned int *)(0x1840C | phy_apb_base)) = 0x00000888;
    *((unsigned int *)(0x18410 | phy_apb_base)) = 0x00000CDD;
    *((unsigned int *)(0x18414 | phy_apb_base)) = 0x00000888;
    *((unsigned int *)(0x18418 | phy_apb_base)) = 0x00000CDD;
    *((unsigned int *)(0x1841C | phy_apb_base)) = 0x00000888;
    *((unsigned int *)(0x18420 | phy_apb_base)) = 0x00000CDD;
    *((unsigned int *)(0x18424 | phy_apb_base)) = 0x00000888;
    *((unsigned int *)(0x18428 | phy_apb_base)) = 0x00000CDD;
    *((unsigned int *)(0x1842C | phy_apb_base)) = 0x00000599;
    *((unsigned int *)(0x18430 | phy_apb_base)) = 0x00000DFE;
    *((unsigned int *)(0x18438 | phy_apb_base)) = 0x00000599;
    *((unsigned int *)(0x1843C | phy_apb_base)) = 0x00000DEE;
    *((unsigned int *)(0x18444 | phy_apb_base)) = 0x000028FF;
    *((unsigned int *)(0x1845C | phy_apb_base)) = 0x00001D3A;
    *((unsigned int *)(0x18460 | phy_apb_base)) = 0x00001B36;
    *((unsigned int *)(0x1846C | phy_apb_base)) = 0x0000000F;
    *((unsigned int *)(0x18470 | phy_apb_base)) = 0x00000002;
    *((unsigned int *)(0x18474 | phy_apb_base)) = 0x00000002;
    *((unsigned int *)(0x18478 | phy_apb_base)) = 0x0000003C;
    *((unsigned int *)(0x18484 | phy_apb_base)) = 0x0000001F;
    *((unsigned int *)(0x18488 | phy_apb_base)) = 0x00001CE7;
    *((unsigned int *)(0x1848C | phy_apb_base)) = 0x00001D67;
    *((unsigned int *)(0x184A0 | phy_apb_base)) = 0x000003F4;
    *((unsigned int *)(0x18530 | phy_apb_base)) = 0x00000E04;
    *((unsigned int *)(0x18534 | phy_apb_base)) = 0x00000101;
    *((unsigned int *)(0x18540 | phy_apb_base)) = 0x0000005A;
    *((unsigned int *)(0x1854C | phy_apb_base)) = 0x00006A10;
    *((unsigned int *)(0x18550 | phy_apb_base)) = 0x00000080;
    *((unsigned int *)(0x18554 | phy_apb_base)) = 0x00000002;
    *((unsigned int *)(0x18558 | phy_apb_base)) = 0x00000F21;
    *((unsigned int *)(0x1855C | phy_apb_base)) = 0x00001F21;
    *((unsigned int *)(0x18584 | phy_apb_base)) = 0x00000023;
    *((unsigned int *)(0x185F8 | phy_apb_base)) = 0x00000B24;
    *((unsigned int *)(0x18640 | phy_apb_base)) = 0x0000003F;
    *((unsigned int *)(0x187E0 | phy_apb_base)) = 0x00000551;
    *((unsigned int *)(0x187E4 | phy_apb_base)) = 0x00000022;
    *((unsigned int *)(0x187E8 | phy_apb_base)) = 0x00000010;
    *((unsigned int *)(0x187EC | phy_apb_base)) = 0x00000010;
    *((unsigned int *)(0x187F0 | phy_apb_base)) = 0x00000008;
    *((unsigned int *)(0x187F8 | phy_apb_base)) = 0x0000795B;
}

bool pcie_wait_condition(void *data)
{
    bool completed;
    fwk_assert(data != NULL);

    struct pcie_wait_condition_data *wait_data =
        (struct pcie_wait_condition_data *)data;
    struct pcie_ctrl_apb_reg *ctrl_apb =
        (struct pcie_ctrl_apb_reg *)(wait_data->ctrl_apb);

    switch (wait_data->stage) {
    case PCIE_INIT_STAGE_PCIE_POWER_ON:
        completed = ((SCC->PCIE_PM_CTRL & SCC_PCIE_PM_CTRL_PWR_ACK_MASK) != 0);
        break;

    case PCIE_INIT_STAGE_CCIX_POWER_ON:
        completed = ((SCC->CCIX_PM_CTRL & SCC_CCIX_PM_CTRL_PWR_ACK_MASK) != 0);
        break;

    case PCIE_INIT_STAGE_PHY:
        completed =
            ((ctrl_apb->RESET_STATUS & RESET_STATUS_PHY_REL_ST_MASK) != 0);
        break;

    case PCIE_INIT_STAGE_CTRL:
        completed =
            (((ctrl_apb->RESET_STATUS & RESET_STATUS_PHY_REL_ST_MASK) != 0) &&
             ((ctrl_apb->RESET_STATUS & RESET_STATUS_PLL_ST_MASK) != 0));
        break;

    case PCIE_INIT_STAGE_LINK_TRNG:

    case PCIE_INIT_STAGE_LINK_RE_TRNG:
        completed = ((ctrl_apb->RP_LTSSM_STATE & RP_LTSSM_STATE_MASK) == 0x10);
        break;

    default:
        fwk_unexpected();
        completed = false;
        break;
    }

    return completed;
}

int pcie_init(
    struct pcie_ctrl_apb_reg *ctrl_apb,
    struct mod_timer_api *timer_api,
    enum pcie_init_stage stage,
    enum pcie_gen gen,
    enum pcie_lane_count lane_count)
{
    struct pcie_wait_condition_data wait_data;
    int status;

    fwk_assert(ctrl_apb != NULL);
    fwk_assert(timer_api != NULL);
    fwk_assert(stage < PCIE_INIT_STAGE_COUNT);

    wait_data.ctrl_apb = ctrl_apb;
    wait_data.stage = stage;

    switch (stage) {
    /* PCIe PHY reset request */
    case PCIE_INIT_STAGE_PHY:
        ctrl_apb->RESET_CTRL = RESET_CTRL_PHY_REL_MASK;
        status = timer_api->wait(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            PCIE_PHY_PLL_LOCK_TIMEOUT,
            pcie_wait_condition,
            &wait_data);
        break;

    /* PCIe RC reset request */
    case PCIE_INIT_STAGE_CTRL:
        /* Clear ARI & SR_IOV bits */
        ctrl_apb->RP_CONFIG_IN &= ~RP_CONFIG_IN_ARI_EN_MASK;
        // ctrl_apb->RP_CONFIG_IN |= RP_CONFIG_IN_ARI_EN_MASK;
        ctrl_apb->RP_CONFIG_IN &= ~RP_CONFIG_IN_SR_IOV_EN_MASK;

        /* Clear the bits before writing to it */
        ctrl_apb->RP_CONFIG_IN &= ~RP_CONFIG_IN_PCIE_GEN_SEL_MASK;
        ctrl_apb->RP_CONFIG_IN &= ~RP_CONFIG_IN_LANE_CNT_IN_MASK;

        ctrl_apb->RP_CONFIG_IN |= (lane_count << RP_CONFIG_IN_LANE_CNT_IN_POS) |
            (gen << RP_CONFIG_IN_PCIE_GEN_SEL_POS);
        ctrl_apb->RESET_CTRL = RESET_CTRL_RC_REL_MASK;
        status = timer_api->wait(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            PCIE_CTRL_RC_RESET_TIMEOUT,
            pcie_wait_condition,
            &wait_data);
        break;

    /* PCIe link training request */
    case PCIE_INIT_STAGE_LINK_TRNG:
        ctrl_apb->RP_CONFIG_IN |= RP_CONFIG_IN_LINK_TRNG_EN_MASK;
        status = timer_api->wait(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            PCIE_LINK_TRAINING_TIMEOUT,
            pcie_wait_condition,
            &wait_data);
        break;

    default:
        fwk_unexpected();
        status = FWK_E_PARAM;
        break;
    }
    return status;
}

int pcie_link_retrain(
    struct pcie_ctrl_apb_reg *ctrl_apb,
    uint32_t rp_ep_config_base,
    struct mod_timer_api *timer_api)
{
    uint32_t reg_val = 0;
    struct pcie_wait_condition_data wait_data;

    fwk_assert(ctrl_apb != NULL);
    fwk_assert(rp_ep_config_base != 0x0);
    fwk_assert(timer_api != NULL);

    wait_data.ctrl_apb = ctrl_apb;
    wait_data.stage = PCIE_INIT_STAGE_LINK_RE_TRNG;

    pcie_rp_ep_config_read_word(
        rp_ep_config_base, PCIE_LINK_CTRL_STATUS_OFFSET, &reg_val);
    reg_val |= PCIE_LINK_CTRL_LINK_RETRAIN_MASK;
    pcie_rp_ep_config_write_word(
        rp_ep_config_base, PCIE_LINK_CTRL_STATUS_OFFSET, reg_val);

    return timer_api->wait(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        PCIE_LINK_RE_TRAINING_TIMEOUT,
        pcie_wait_condition,
        &wait_data);
}

int axi_outbound_region_setup(
    uint32_t axi_config_base_addr,
    uint64_t axi_base_addr,
    uint32_t region_size,
    uint8_t region_idx,
    uint8_t trans_type)
{
    volatile struct axi_ob_config ob_config = { 0 };
    volatile uint32_t *region_address = NULL;
    volatile uint32_t *ptr;
    int count;

    if (region_idx >= AXI_OB_REGIONS_MAX) {
        return FWK_E_RANGE;
    }

    (void)memset((void *)&ob_config, 0, sizeof(struct axi_ob_config));

    ob_config.addr0.num_bits = region_size - 1;
    if ((trans_type == TRANS_TYPE_MEM_IO) || (trans_type == TRANS_TYPE_IO)) {
        ob_config.addr0.address_bits =
            ((uint32_t)axi_base_addr >> AXI_LOW_ADDR_BIT_POS);
        ob_config.addr1.address_bits =
            (uint32_t)(axi_base_addr >> AXI_HIGH_ADDR_BIT_POS);
    } else {
        ob_config.addr0.address_bits = 0;
    }
    ob_config.desc0.bus_dev_num_from_addr_desc = 1;
    ob_config.desc0.trans_type = trans_type;
    ob_config.axi_base_addr0.region_sz = region_size - 1;
    ob_config.axi_base_addr0.axi_base_address =
        ((uint32_t)axi_base_addr >> AXI_LOW_ADDR_BIT_POS);
    ob_config.axi_base_addr1.axi_base_address =
        (uint32_t)(axi_base_addr >> AXI_HIGH_ADDR_BIT_POS);
    region_address =
        (volatile uint32_t
             *)(axi_config_base_addr + (region_idx * AXI_OB_REGISTER_SET_SIZE));

    ptr = (volatile uint32_t *)&ob_config;

    for (count = 0; count < AXI_OB_REGISTER_COUNT; count++) {
        region_address[count] = ptr[count];
    }

    return FWK_SUCCESS;
}

int axi_inbound_region_setup(
    uint32_t axi_config_base_addr,
    uint64_t axi_base_addr,
    uint32_t region_size,
    uint8_t bar)
{
    uint32_t offset;

    if ((bar >= AXI_IB_REGIONS_MAX) || (region_size > AXI_ADDR_NUM_BITS_MAX) ||
        (__builtin_ctz(axi_base_addr) < AXI_LOW_ADDR_BIT_POS)) {
        return FWK_E_PARAM;
    }

    offset = AXI_IB_REGION_REGS_OFFSET + (bar * AXI_IB_REGISTER_SET_SIZE);
    *(uint32_t *)(axi_config_base_addr + offset) =
        (uint32_t)axi_base_addr | (region_size - 1);
    *(uint32_t *)(axi_config_base_addr + offset + 4) =
        (uint32_t)(axi_base_addr >> AXI_HIGH_ADDR_BIT_POS);
    return FWK_SUCCESS;
}

int pcie_rp_ep_config_write_word(uint32_t base, uint32_t offset, uint32_t value)
{
    if ((offset % 4)) {
        return FWK_E_PARAM;
    }

    base |= ROOT_PORT_WRITE_ENABLE;
    *(volatile uint32_t *)(base + offset) = value;

    return FWK_SUCCESS;
}

int pcie_rp_ep_config_read_word(uint32_t base, uint32_t offset, uint32_t *value)
{
    if ((offset % 4) || (value == NULL)) {
        return FWK_E_PARAM;
    }

    *value = *(volatile uint32_t *)(base + offset);

    return FWK_SUCCESS;
}

int pcie_set_gen_tx_preset(
    uint32_t rp_ep_config_apb_base,
    uint32_t down_stream_tx_preset,
    uint32_t up_stream_tx_preset,
    enum pcie_gen gen)
{
    uint32_t i;
    uint32_t j;
    uint32_t offset;
    uint32_t reg_value;
    uint32_t preset_value = 0;
    uint32_t offset_min;
    uint32_t offset_max;
    uint32_t nibble;

    fwk_assert((gen == PCIE_GEN_3) || (gen == PCIE_GEN_4));

    offset_min = (gen == PCIE_GEN_3) ? GEN3_OFFSET_MIN : GEN4_OFFSET_MIN;
    offset_max = (gen == PCIE_GEN_3) ? GEN3_OFFSET_MAX : GEN4_OFFSET_MAX;
    nibble = (gen == PCIE_GEN_3) ? GEN3_PRESET : GEN4_PRESET;

    for (i = 0, j = 0; i < 32; i += nibble, j++) {
        if (j % 2 == 0) {
            preset_value |= (down_stream_tx_preset << i);
        } else {
            preset_value |= (up_stream_tx_preset << i);
        }
    }

    for (offset = offset_min; offset < offset_max; offset += 0x4) {
        pcie_rp_ep_config_write_word(
            rp_ep_config_apb_base, offset, preset_value);
        pcie_rp_ep_config_read_word(rp_ep_config_apb_base, offset, &reg_value);

        if (reg_value != preset_value) {
            return FWK_E_DATA;
        }
    }
    return FWK_SUCCESS;
}

int pcie_skip_ext_cap(uint32_t base, uint16_t ext_cap_id)
{
    uint32_t cap_hdr_now;
    uint32_t cap_hdr_next;
    uint32_t offset;
    uint32_t offset_next;
    uint32_t offset_target;

    offset = EXT_CAP_START_OFFSET;
    cap_hdr_now = 0;
    cap_hdr_next = 0;

    do {
        pcie_rp_ep_config_read_word(base, offset, &cap_hdr_now);
        offset_next = (cap_hdr_now >> EXT_CAP_NEXT_CAP_POS);
        pcie_rp_ep_config_read_word(base, offset_next, &cap_hdr_next);
        if (((uint16_t)(cap_hdr_next & EXT_CAP_ID_MASK)) == ext_cap_id) {
            offset_target =
                (cap_hdr_next &
                 (EXT_CAP_NEXT_CAP_MASK << EXT_CAP_NEXT_CAP_POS));
            cap_hdr_now = (cap_hdr_now &
                           ~(EXT_CAP_NEXT_CAP_MASK << EXT_CAP_NEXT_CAP_POS)) |
                offset_target;

            pcie_rp_ep_config_write_word(base, offset, cap_hdr_now);
            return FWK_SUCCESS;
        }
        offset = offset_next;
    } while ((offset != 0));

    return FWK_E_SUPPORT;
}

int pcie_vc_setup(uint32_t base, uint8_t vc1_tc)
{
    /* VC1 Traffic class cannot be greater than 7 or equal to 0 */
    if ((vc1_tc > 7) || (vc1_tc == 0)) {
        return FWK_E_PARAM;
    }

    /* Map all other TCs to VC0 */
    *(volatile uint32_t *)(base + PCIE_VC_RESOURCE_CTRL_0_OFFSET) =
        PCIE_VC_CTRL_VCEN_MASK | (0 << PCIE_VC_VCID_SHIFT) |
        (~(1 << vc1_tc) & 0xFF);

    /* Enable VC1 & map VC1 to TC7 */
    *(volatile uint32_t *)(base + PCIE_VC_RESOURCE_CTRL_1_OFFSET) =
        PCIE_VC_CTRL_VCEN_MASK | (1 << PCIE_VC_VCID_SHIFT) |
        ((1 << vc1_tc) & 0xFF);

    return FWK_SUCCESS;
}
