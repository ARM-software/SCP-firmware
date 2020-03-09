/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_scc_reg.h"
#include "n1sdp_scp_pik.h"

#include <n1sdp_pcie.h>

#include <internal/pcie_ctrl_apb_reg.h>

#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

void pcie_phy_init(uint32_t phy_apb_base)
{
    uint32_t j;

    *((unsigned int *)(0x30038 | phy_apb_base)) = 0x00000013;
    *((unsigned int *)(0x0010C | phy_apb_base)) = 0x0000002D;
    *((unsigned int *)(0x00138 | phy_apb_base)) = 0x00001005;
    *((unsigned int *)(0x00260 | phy_apb_base)) = 0x00002100;

    for (j = 0; j < 16; j++) {
        *((unsigned int *)((j << 11) | 0x1000C | phy_apb_base)) = 0x00006910;
        *((unsigned int *)((j << 11) | 0x100EC | phy_apb_base)) = 0x00000055;

        *((unsigned int *)((j << 11) | 0x100FC | phy_apb_base)) = 0x00003222;
        *((unsigned int *)((j << 11) | 0x10214 | phy_apb_base)) = 0x00000180;
        *((unsigned int *)((j << 11) | 0x10218 | phy_apb_base)) = 0x00000381;
        *((unsigned int *)((j << 11) | 0x1021C | phy_apb_base)) = 0x00000342;
        *((unsigned int *)((j << 11) | 0x10220 | phy_apb_base)) = 0x00000384;

        *((unsigned int *)((j << 11) | 0x1023C | phy_apb_base)) = 0x00000000;

        *((unsigned int *)((j << 11) | 0x10324 | phy_apb_base)) = 0x00005864;
        *((unsigned int *)((j << 11) | 0x10330 | phy_apb_base)) = 0x000001FF;
        *((unsigned int *)((j << 11) | 0x10334 | phy_apb_base)) = 0x0000000F;
        *((unsigned int *)((j << 11) | 0x10338 | phy_apb_base)) = 0x000000FF;
        *((unsigned int *)((j << 11) | 0x10340 | phy_apb_base)) = 0x00004A4A;
        *((unsigned int *)((j << 11) | 0x10360 | phy_apb_base)) = 0x000068F8;
        *((unsigned int *)((j << 11) | 0x10368 | phy_apb_base)) = 0x000000F8;
        *((unsigned int *)((j << 11) | 0x10500 | phy_apb_base)) = 0x0000009B;
        *((unsigned int *)((j << 11) | 0x10504 | phy_apb_base)) = 0x0000C0C0;
        *((unsigned int *)((j << 11) | 0x10508 | phy_apb_base)) = 0x0000818F;

        *((unsigned int *)((j << 11) | 0x1053C | phy_apb_base)) = 0x00002401;
        *((unsigned int *)((j << 11) | 0x10540 | phy_apb_base)) = 0x00003C03;
        *((unsigned int *)((j << 11) | 0x10544 | phy_apb_base)) = 0x00000A0A;
        *((unsigned int *)((j << 11) | 0x10588 | phy_apb_base)) = 0x00000012;

        *((unsigned int *)((j << 11) | (0x40DF * 4) | phy_apb_base)) =
            0x00000A92;
        *((unsigned int *)((j << 11) | (0x40E4 * 4) | phy_apb_base)) =
            0x00000020;
        *((unsigned int *)((j << 11) | (0x40E5 * 4) | phy_apb_base)) =
            0x00007C10;
        *((unsigned int *)((j << 11) | (0x40E6 * 4) | phy_apb_base)) =
            0x00000020;
        *((unsigned int *)((j << 11) | (0x40E7 * 4) | phy_apb_base)) =
            0x00007C10;
        *((unsigned int *)((j << 11) | (0x40E8 * 4) | phy_apb_base)) =
            0x00005300;
        *((unsigned int *)((j << 11) | (0x40E9 * 4) | phy_apb_base)) =
            0x00001C91;
        *((unsigned int *)((j << 11) | (0x40EA * 4) | phy_apb_base)) =
            0x00005300;
        *((unsigned int *)((j << 11) | (0x40EB * 4) | phy_apb_base)) =
            0x00002890;
        *((unsigned int *)((j << 11) | (0x40EC * 4) | phy_apb_base)) =
            0x00002300;
        *((unsigned int *)((j << 11) | (0x40ED * 4) | phy_apb_base)) =
            0x00001C91;
        *((unsigned int *)((j << 11) | (0x40EE * 4) | phy_apb_base)) =
            0x00002300;
        *((unsigned int *)((j << 11) | (0x40EF * 4) | phy_apb_base)) =
            0x00002890;
        *((unsigned int *)((j << 11) | (0x40F0 * 4) | phy_apb_base)) =
            0x00002280;
        *((unsigned int *)((j << 11) | (0x40F1 * 4) | phy_apb_base)) =
            0x00003EAE;
        *((unsigned int *)((j << 11) | (0x40F2 * 4) | phy_apb_base)) =
            0x00002200;
        *((unsigned int *)((j << 11) | (0x40F3 * 4) | phy_apb_base)) =
            0x0000489E;
        *((unsigned int *)((j << 11) | (0x40F4 * 4) | phy_apb_base)) =
            0x00002200;
        *((unsigned int *)((j << 11) | (0x40F5 * 4) | phy_apb_base)) =
            0x00004F73;
        *((unsigned int *)((j << 11) | (0x40F6 * 4) | phy_apb_base)) =
            0x00002200;
        *((unsigned int *)((j << 11) | (0x40F7 * 4) | phy_apb_base)) =
            0x00004C68;
        *((unsigned int *)((j << 11) | (0x40F8 * 4) | phy_apb_base)) =
            0x0000221C;
        *((unsigned int *)((j << 11) | (0x40F9 * 4) | phy_apb_base)) =
            0x00006A6F;
        *((unsigned int *)((j << 11) | (0x40FA * 4) | phy_apb_base)) =
            0x0000221C;
        *((unsigned int *)((j << 11) | (0x40FB * 4) | phy_apb_base)) =
            0x00006B6E;
        *((unsigned int *)((j << 11) | (0x40FC * 4) | phy_apb_base)) =
            0x0000221C;
        *((unsigned int *)((j << 11) | (0x40FD * 4) | phy_apb_base)) =
            0x00006831;
        *((unsigned int *)((j << 11) | (0x40FE * 4) | phy_apb_base)) =
            0x0000221C;
        *((unsigned int *)((j << 11) | (0x40FF * 4) | phy_apb_base)) =
            0x00006F34;
        *((unsigned int *)((j << 11) | (0x4100 * 4) | phy_apb_base)) =
            0x0000221C;
        *((unsigned int *)((j << 11) | (0x4101 * 4) | phy_apb_base)) =
            0x00007A34;
        *((unsigned int *)((j << 11) | (0x4102 * 4) | phy_apb_base)) =
            0x00002228;
        *((unsigned int *)((j << 11) | (0x4103 * 4) | phy_apb_base)) =
            0x00007B25;
        *((unsigned int *)((j << 11) | (0x4104 * 4) | phy_apb_base)) =
            0x0000222F;
        *((unsigned int *)((j << 11) | (0x4105 * 4) | phy_apb_base)) =
            0x00007E25;
        *((unsigned int *)((j << 11) | (0x4106 * 4) | phy_apb_base)) =
            0x0000222F;
        *((unsigned int *)((j << 11) | (0x4107 * 4) | phy_apb_base)) =
            0x00007F21;
        *((unsigned int *)((j << 11) | (0x4108 * 4) | phy_apb_base)) =
            0x0000222C;
        *((unsigned int *)((j << 11) | (0x4109 * 4) | phy_apb_base)) =
            0x00007C21;
        *((unsigned int *)((j << 11) | (0x410A * 4) | phy_apb_base)) =
            0x00002222;
        *((unsigned int *)((j << 11) | (0x410B * 4) | phy_apb_base)) =
            0x00007224;
        *((unsigned int *)((j << 11) | (0x410C * 4) | phy_apb_base)) =
            0x00002222;
        *((unsigned int *)((j << 11) | (0x410D * 4) | phy_apb_base)) =
            0x00008E3F;
        *((unsigned int *)((j << 11) | (0x410E * 4) | phy_apb_base)) =
            0x00002223;
        *((unsigned int *)((j << 11) | (0x410F * 4) | phy_apb_base)) =
            0x00009A3F;
        *((unsigned int *)((j << 11) | (0x4110 * 4) | phy_apb_base)) =
            0x00002220;
        *((unsigned int *)((j << 11) | (0x4111 * 4) | phy_apb_base)) =
            0x00008C2C;
        *((unsigned int *)((j << 11) | (0x4112 * 4) | phy_apb_base)) =
            0x00002220;
        *((unsigned int *)((j << 11) | (0x4113 * 4) | phy_apb_base)) =
            0x00009828;
        *((unsigned int *)((j << 11) | (0x4114 * 4) | phy_apb_base)) =
            0x00002220;
        *((unsigned int *)((j << 11) | (0x4115 * 4) | phy_apb_base)) =
            0x00009F2F;
        *((unsigned int *)((j << 11) | (0x4116 * 4) | phy_apb_base)) =
            0x0000223C;
        *((unsigned int *)((j << 11) | (0x4117 * 4) | phy_apb_base)) =
            0x0000AB2F;
        *((unsigned int *)((j << 11) | (0x4118 * 4) | phy_apb_base)) =
            0x0000223C;
        *((unsigned int *)((j << 11) | (0x4119 * 4) | phy_apb_base)) =
            0x0000A81C;
        *((unsigned int *)((j << 11) | (0x411A * 4) | phy_apb_base)) =
            0x0000223C;
        *((unsigned int *)((j << 11) | (0x411B * 4) | phy_apb_base)) =
            0x0000BA22;
        *((unsigned int *)((j << 11) | (0x411C * 4) | phy_apb_base)) =
            0x0000223C;
        *((unsigned int *)((j << 11) | (0x411D * 4) | phy_apb_base)) =
            0x0000BB13;
        *((unsigned int *)((j << 11) | (0x411E * 4) | phy_apb_base)) =
            0x0000223C;
        *((unsigned int *)((j << 11) | (0x411F * 4) | phy_apb_base)) =
            0x0000B800;
        *((unsigned int *)((j << 11) | (0x4120 * 4) | phy_apb_base)) =
            0x00002232;
        *((unsigned int *)((j << 11) | (0x4121 * 4) | phy_apb_base)) =
            0x0000BF00;
        *((unsigned int *)((j << 11) | (0x4122 * 4) | phy_apb_base)) =
            0x00002233;
        *((unsigned int *)((j << 11) | (0x4123 * 4) | phy_apb_base)) =
            0x0000B200;
        *((unsigned int *)((j << 11) | (0x4124 * 4) | phy_apb_base)) =
            0x00002233;
        *((unsigned int *)((j << 11) | (0x4125 * 4) | phy_apb_base)) =
            0x0000B300;
        *((unsigned int *)((j << 11) | (0x4126 * 4) | phy_apb_base)) =
            0x00002230;
        *((unsigned int *)((j << 11) | (0x4127 * 4) | phy_apb_base)) =
            0x0000B000;

        *((unsigned int *)((j << 11) | (0x4099 * 4) | phy_apb_base)) =
            0x00008000;
        *((unsigned int *)((j << 11) | (0x409A * 4) | phy_apb_base)) =
            0x00007777;
        *((unsigned int *)((j << 11) | (0x409B * 4) | phy_apb_base)) =
            0x00000100;
        *((unsigned int *)((j << 11) | (0x409C * 4) | phy_apb_base)) =
            0x00007777;
        *((unsigned int *)((j << 11) | (0x409D * 4) | phy_apb_base)) =
            0x00000101;
        *((unsigned int *)((j << 11) | (0x409E * 4) | phy_apb_base)) =
            0x00007777;
        *((unsigned int *)((j << 11) | (0x409F * 4) | phy_apb_base)) =
            0x00000102;
        *((unsigned int *)((j << 11) | (0x40A0 * 4) | phy_apb_base)) =
            0x00007777;
        *((unsigned int *)((j << 11) | (0x40A1 * 4) | phy_apb_base)) =
            0x00000002;
        *((unsigned int *)((j << 11) | (0x40A2 * 4) | phy_apb_base)) =
            0x00007777;
        *((unsigned int *)((j << 11) | (0x40A3 * 4) | phy_apb_base)) =
            0x00001002;
        *((unsigned int *)((j << 11) | (0x40A4 * 4) | phy_apb_base)) =
            0x00003777;
        *((unsigned int *)((j << 11) | (0x40A5 * 4) | phy_apb_base)) =
            0x00001003;
        *((unsigned int *)((j << 11) | (0x40A6 * 4) | phy_apb_base)) =
            0x00003777;
        *((unsigned int *)((j << 11) | (0x40A7 * 4) | phy_apb_base)) =
            0x00001004;
        *((unsigned int *)((j << 11) | (0x40A8 * 4) | phy_apb_base)) =
            0x00003777;
        *((unsigned int *)((j << 11) | (0x40A9 * 4) | phy_apb_base)) =
            0x00002004;
        *((unsigned int *)((j << 11) | (0x40AA * 4) | phy_apb_base)) =
            0x00003777;
        *((unsigned int *)((j << 11) | (0x40AB * 4) | phy_apb_base)) =
            0x00002005;
        *((unsigned int *)((j << 11) | (0x40AC * 4) | phy_apb_base)) =
            0x00003777;
        *((unsigned int *)((j << 11) | (0x40AD * 4) | phy_apb_base)) =
            0x00002006;
        *((unsigned int *)((j << 11) | (0x40AE * 4) | phy_apb_base)) =
            0x00003777;
        *((unsigned int *)((j << 11) | (0x40AF * 4) | phy_apb_base)) =
            0x00002007;
        *((unsigned int *)((j << 11) | (0x40B0 * 4) | phy_apb_base)) =
            0x00003777;
        *((unsigned int *)((j << 11) | (0x40B1 * 4) | phy_apb_base)) =
            0x00002008;
        *((unsigned int *)((j << 11) | (0x40B2 * 4) | phy_apb_base)) =
            0x00003777;
        *((unsigned int *)((j << 11) | (0x40B3 * 4) | phy_apb_base)) =
            0x00002009;
        *((unsigned int *)((j << 11) | (0x40B4 * 4) | phy_apb_base)) =
            0x00003777;
        *((unsigned int *)((j << 11) | (0x40B5 * 4) | phy_apb_base)) =
            0x0000200B;
        *((unsigned int *)((j << 11) | (0x40B6 * 4) | phy_apb_base)) =
            0x00003777;
        *((unsigned int *)((j << 11) | (0x40B7 * 4) | phy_apb_base)) =
            0x0000200D;
        *((unsigned int *)((j << 11) | (0x40B8 * 4) | phy_apb_base)) =
            0x00003777;
        *((unsigned int *)((j << 11) | (0x40B9 * 4) | phy_apb_base)) =
            0x0000200F;
    }
}

bool pcie_wait_condition(void *data)
{
    assert(data != NULL);

    struct pcie_wait_condition_data *wait_data =
        (struct pcie_wait_condition_data *)data;
    struct pcie_ctrl_apb_reg *ctrl_apb =
        (struct pcie_ctrl_apb_reg *)(wait_data->ctrl_apb);

    switch (wait_data->stage) {
    case PCIE_INIT_STAGE_PCIE_POWER_ON:
        return ((SCC->PCIE_PM_CTRL & SCC_PCIE_PM_CTRL_PWR_ACK_MASK) != 0);
    case PCIE_INIT_STAGE_CCIX_POWER_ON:
        return ((SCC->CCIX_PM_CTRL & SCC_CCIX_PM_CTRL_PWR_ACK_MASK) != 0);
    case PCIE_INIT_STAGE_PHY:
        return ((ctrl_apb->RESET_STATUS &
                 RESET_STATUS_PHY_REL_ST_MASK) != 0);
    case PCIE_INIT_STAGE_CTRL:
        return ((ctrl_apb->RESET_STATUS &
                 RESET_STATUS_RC_REL_ST_MASK) != 0);
    case PCIE_INIT_STAGE_LINK_TRNG:
    case PCIE_INIT_STAGE_LINK_RE_TRNG:
        return ((ctrl_apb->RP_LTSSM_STATE & RP_LTSSM_STATE_MASK) == 0x10);
    default:
        assert(false);
        return false;
    }
}

int pcie_init(struct pcie_ctrl_apb_reg *ctrl_apb,
              struct mod_timer_api *timer_api,
              enum pcie_init_stage stage,
              enum pcie_gen gen)
{
    assert(ctrl_apb != NULL);
    assert(timer_api != NULL);
    assert(stage < PCIE_INIT_STAGE_COUNT);

    struct pcie_wait_condition_data wait_data;
    int status;

    wait_data.ctrl_apb = ctrl_apb;
    wait_data.stage = stage;

    switch (stage) {
    /* PCIe PHY reset request */
    case PCIE_INIT_STAGE_PHY:
        ctrl_apb->RESET_CTRL = RESET_CTRL_PHY_REL_MASK;
        status = timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                 PCIE_PHY_PLL_LOCK_TIMEOUT,
                                 pcie_wait_condition,
                                 &wait_data);
        if (status != FWK_SUCCESS)
            return status;
        break;

    /* PCIe RC reset request */
    case PCIE_INIT_STAGE_CTRL:
        /* Clear ARI & SR_IOV bits */
        ctrl_apb->RP_CONFIG_IN &= ~RP_CONFIG_IN_ARI_EN_MASK;
        ctrl_apb->RP_CONFIG_IN &= ~RP_CONFIG_IN_SR_IOV_EN_MASK;

        /* Clear the bits before writing to it */
        ctrl_apb->RP_CONFIG_IN &= ~RP_CONFIG_IN_PCIE_GEN_SEL_MASK;
        ctrl_apb->RP_CONFIG_IN &= ~RP_CONFIG_IN_LANE_CNT_IN_MASK;

        ctrl_apb->RP_CONFIG_IN |= (0x4 << RP_CONFIG_IN_LANE_CNT_IN_POS) |
                                  (gen << RP_CONFIG_IN_PCIE_GEN_SEL_POS);
        ctrl_apb->RESET_CTRL = RESET_CTRL_RC_REL_MASK;
        status = timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                 PCIE_CTRL_RC_RESET_TIMEOUT,
                                 pcie_wait_condition,
                                 &wait_data);
        if (status != FWK_SUCCESS)
            return status;
        break;

    /* PCIe link training request */
    case PCIE_INIT_STAGE_LINK_TRNG:
        ctrl_apb->RP_CONFIG_IN |= RP_CONFIG_IN_LINK_TRNG_EN_MASK;
        status = timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                 PCIE_LINK_TRAINING_TIMEOUT,
                                 pcie_wait_condition,
                                 &wait_data);
        if (status != FWK_SUCCESS)
            return status;
        break;

    default:
        assert(false);
        return FWK_E_PARAM;
    }
    return FWK_SUCCESS;
}

int pcie_link_retrain(struct pcie_ctrl_apb_reg *ctrl_apb,
                      uint32_t rp_ep_config_base,
                      struct mod_timer_api *timer_api)
{
    uint32_t reg_val = 0;
    struct pcie_wait_condition_data wait_data;

    assert(ctrl_apb != NULL);
    assert(rp_ep_config_base != 0x0);
    assert(timer_api != NULL);

    wait_data.ctrl_apb = ctrl_apb;
    wait_data.stage = PCIE_INIT_STAGE_LINK_RE_TRNG;

    pcie_rp_ep_config_read_word(rp_ep_config_base,
                                PCIE_LINK_CTRL_STATUS_OFFSET, &reg_val);
    reg_val |= PCIE_LINK_CTRL_LINK_RETRAIN_MASK;
    pcie_rp_ep_config_write_word(rp_ep_config_base,
                                 PCIE_LINK_CTRL_STATUS_OFFSET, reg_val);

    return timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                           PCIE_LINK_RE_TRAINING_TIMEOUT,
                           pcie_wait_condition,
                           &wait_data);
}

int axi_outbound_region_setup(uint32_t axi_config_base_addr,
                              uint64_t axi_base_addr,
                              uint32_t region_size,
                              uint8_t trans_type)
{
    static uint8_t region_count;
    volatile struct axi_ob_config ob_config = {0};
    volatile uint32_t *region_address = NULL;
    volatile uint32_t *ptr;
    int count;

    if (region_count >= AXI_OB_REGIONS_MAX)
        return FWK_E_RANGE;

    memset((void *)&ob_config, 0, sizeof(struct axi_ob_config));

    ob_config.addr0.num_bits = region_size;
    if ((trans_type == TRANS_TYPE_MEM_IO) ||
        (trans_type == TRANS_TYPE_IO)) {
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
    region_address = (volatile uint32_t *)(axi_config_base_addr +
                         (region_count * AXI_OB_REGISTER_SET_SIZE));
    region_count++;

    ptr = (volatile uint32_t *)&ob_config;

    for (count = 0; count < AXI_OB_REGISTER_COUNT; count++)
        region_address[count] = ptr[count];

    return FWK_SUCCESS;
}

int axi_inbound_region_setup(uint32_t axi_config_base_addr,
                             uint64_t axi_base_addr,
                             uint32_t region_size,
                             uint8_t bar)
{
    uint32_t offset;

    if ((bar >= AXI_IB_REGIONS_MAX) ||
        (region_size > AXI_ADDR_NUM_BITS_MAX) ||
        (__builtin_ctz(axi_base_addr) < AXI_LOW_ADDR_BIT_POS))
        return FWK_E_PARAM;

    offset = AXI_IB_REGION_REGS_OFFSET + (bar * AXI_IB_REGISTER_SET_SIZE);
    *(uint32_t *)(axi_config_base_addr + offset) = (uint32_t)axi_base_addr |
                                                   (region_size - 1);
    *(uint32_t *)(axi_config_base_addr + offset + 4) =
        (uint32_t)(axi_base_addr >> AXI_HIGH_ADDR_BIT_POS);
    return FWK_SUCCESS;
}

int pcie_rp_ep_config_write_word(uint32_t base,
                                 uint32_t offset,
                                 uint32_t value)
{
    if ((offset % 4))
        return FWK_E_PARAM;

    base |= ROOT_PORT_WRITE_ENABLE;
    *(uint32_t *)(base + offset) = value;

    return FWK_SUCCESS;
}

int pcie_rp_ep_config_read_word(uint32_t base,
                                uint32_t offset,
                                uint32_t *value)
{
    if ((offset % 4) || (value == NULL))
        return FWK_E_PARAM;

    *value = *(uint32_t *)(base + offset);

    return FWK_SUCCESS;
}

int pcie_set_gen_tx_preset(uint32_t rp_ep_config_apb_base,
                           uint32_t preset,
                           enum pcie_gen gen)
{
    uint32_t i;
    uint32_t offset;
    uint32_t reg_value;
    uint32_t preset_reg = 0;
    uint32_t offset_min;
    uint32_t offset_max;
    uint32_t nibble;

    assert((gen == PCIE_GEN_3) || (gen == PCIE_GEN_4));

    offset_min = (gen == PCIE_GEN_3) ? GEN3_OFFSET_MIN : GEN4_OFFSET_MIN;
    offset_max = (gen == PCIE_GEN_3) ? GEN3_OFFSET_MAX : GEN4_OFFSET_MAX;
    nibble = (gen == PCIE_GEN_3) ? GEN3_PRESET : GEN4_PRESET;

    for (i = 0; i < 32; i += nibble)
        preset_reg |= (preset << i);

    for (offset = offset_min; offset < offset_max; offset += 0x4) {
        pcie_rp_ep_config_write_word(rp_ep_config_apb_base, offset, preset_reg);
        pcie_rp_ep_config_read_word(rp_ep_config_apb_base, offset, &reg_value);

        if (reg_value != preset_reg)
            return FWK_E_DATA;
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
            offset_target = (cap_hdr_next &
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
    if ((vc1_tc > 7) || (vc1_tc == 0))
        return FWK_E_PARAM;

    /* Map all other TCs to VC0 */
    *(volatile uint32_t *)(base + PCIE_VC_RESOURCE_CTRL_0_OFFSET) =
        PCIE_VC_CTRL_VCEN_MASK |
        (0 << PCIE_VC_VCID_SHIFT) |
        (~(1 << vc1_tc) & 0xFF);

    /* Enable VC1 & map VC1 to TC7 */
    *(volatile uint32_t *)(base + PCIE_VC_RESOURCE_CTRL_1_OFFSET) =
        PCIE_VC_CTRL_VCEN_MASK |
        (1 << PCIE_VC_VCID_SHIFT) |
        ((1 << vc1_tc) & 0xFF);

    return FWK_SUCCESS;
}
