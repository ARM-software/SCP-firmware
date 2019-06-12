/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <internal/pcie_ctrl_apb_reg.h>
#include <mod_n1sdp_pcie.h>
#include <mod_timer.h>
#include <n1sdp_pcie.h>
#include <n1sdp_scp_pik.h>

void pcie_phy_init(uint32_t phy_apb_base, enum pcie_gen gen)
{

    uint32_t j;

    for (j = 0; j < 16; j++) {
        *((unsigned int *)((j << 11) | 0x1000C | phy_apb_base)) = 0x0000E900;

        switch (gen) {
        case PCIE_GEN_1:
            *((unsigned int *)((j << 11) | 0x10220 | phy_apb_base)) = 0x0343;
            break;
        case PCIE_GEN_2:
            *((unsigned int *)((j << 11) | 0x1021C | phy_apb_base)) = 0x0342;
            break;
        case PCIE_GEN_3:
            *((unsigned int *)((j << 11) | 0x10218 | phy_apb_base)) = 0x0381;
            break;
        case PCIE_GEN_4:
            *((unsigned int *)((j << 11) | 0x10214 | phy_apb_base)) = 0x0180;
            break;
        default:
            return;
        }
    }

    *((unsigned int *)(0x30038 | phy_apb_base)) = 0x00000013;
    *((unsigned int *)(0x0010C | phy_apb_base)) = 0x0000002D;
    *((unsigned int *)(0x00138 | phy_apb_base)) = 0x00001005;
    *((unsigned int *)(0x00260 | phy_apb_base)) = 0x00002100;

    *((unsigned int *)(0x180EC | phy_apb_base)) = 0x00000055;

    for (j = 0; j < 16; j++) {
        *((unsigned int *)((j << 11) | 0x10320 | phy_apb_base)) = 0x00000500;
        *((unsigned int *)((j << 11) | 0x10328 | phy_apb_base)) = 0x0000A990;
        *((unsigned int *)((j << 11) | 0x10540 | phy_apb_base)) = 0x00005008;
        *((unsigned int *)((j << 11) | 0x10560 | phy_apb_base)) = 0x00007425;
        *((unsigned int *)((j << 11) | 0x10580 | phy_apb_base)) = 0x000000AC;
    }

    for (j = 0; j < 16; j++) {
        *((unsigned int *)((j << 11) | 0x10110 | phy_apb_base)) = 0x00000000;
        *((unsigned int *)((j << 11) | 0x10258 | phy_apb_base)) = 0x00008000;
        *((unsigned int *)((j << 11) | 0x10324 | phy_apb_base)) = 0x0000D664;
        *((unsigned int *)((j << 11) | 0x101C4 | phy_apb_base)) = 0x00000002;
        *((unsigned int *)((j << 11) | 0x10320 | phy_apb_base)) = 0x00000100;
        *((unsigned int *)((j << 11) | 0x10328 | phy_apb_base)) = 0x00008190;
        *((unsigned int *)((j << 11) | 0x10334 | phy_apb_base)) = 0x0000008A;
        *((unsigned int *)((j << 11) | 0x1034C | phy_apb_base)) = 0x0000000A;
        *((unsigned int *)((j << 11) | 0x1053C | phy_apb_base)) = 0x00005008;
        *((unsigned int *)((j << 11) | 0x10540 | phy_apb_base)) = 0x00005008;
        *((unsigned int *)((j << 11) | 0x10560 | phy_apb_base)) = 0x00003783;
        *((unsigned int *)((j << 11) | 0x1060C | phy_apb_base)) = 0x00001002;
        *((unsigned int *)((j << 11) | 0x10610 | phy_apb_base)) = 0x00002004;
        *((unsigned int *)((j << 11) | 0x107BC | phy_apb_base)) = 0x00001002;
        *((unsigned int *)((j << 11) | 0x107C0 | phy_apb_base)) = 0x00002004;
    }

    for (j = 0; j < 16; j++) {
        *((unsigned int *)((j << 11) | 0x1023C | phy_apb_base)) = 0x00008003;
        *((unsigned int *)((j << 11) | 0x10340 | phy_apb_base)) = 0x00004A4A;
        *((unsigned int *)((j << 11) | 0x10324 | phy_apb_base)) = 0x00005864;
        *((unsigned int *)((j << 11) | 0x10330 | phy_apb_base)) = 0x000001FF;
        *((unsigned int *)((j << 11) | 0x10334 | phy_apb_base)) = 0x0000000F;
        *((unsigned int *)((j << 11) | 0x10338 | phy_apb_base)) = 0x000000FF;
        *((unsigned int *)((j << 11) | 0x10360 | phy_apb_base)) = 0x000068F8;
        *((unsigned int *)((j << 11) | 0x10368 | phy_apb_base)) = 0x000000F8;
        *((unsigned int *)((j << 11) | 0x10500 | phy_apb_base)) = 0x0000009B;
        *((unsigned int *)((j << 11) | 0x10504 | phy_apb_base)) = 0x0000C0C0;
        *((unsigned int *)((j << 11) | 0x10508 | phy_apb_base)) = 0x0000818F;
        *((unsigned int *)((j << 11) | 0x1053C | phy_apb_base)) = 0x00002401;
        *((unsigned int *)((j << 11) | 0x10540 | phy_apb_base)) = 0x00003C03;
        *((unsigned int *)((j << 11) | 0x10544 | phy_apb_base)) = 0x00000A0A;
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
