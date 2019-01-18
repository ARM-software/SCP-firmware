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
              enum pcie_init_stage stage)
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
        ctrl_apb->RP_CONFIG_IN = (0x4 << RP_CONFIG_IN_LANE_CNT_IN_POS) |
                                 (0x1 << RP_CONFIG_IN_PCIE_GEN_SEL_POS);
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
