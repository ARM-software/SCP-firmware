/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "n1sdp_core.h"
#include "n1sdp_scc_reg.h"
#include "n1sdp_scp_pik.h"

#include <n1sdp_pcie.h>

#include <internal/pcie_ctrl_apb_reg.h>

#include <mod_clock.h>
#include <mod_n1sdp_c2c_i2c.h>
#include <mod_n1sdp_pcie.h>
#include <mod_timer.h>

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_math.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <string.h>

void pcie_bus_enumeration(struct n1sdp_pcie_dev_config *config);
void pcie_init_bdf_table(struct n1sdp_pcie_dev_config *config);

/*
 * Device context
 */
struct n1sdp_pcie_dev_ctx {
    /* Pointer to PCIe device configuration */
    struct n1sdp_pcie_dev_config *config;

    /*
     * Pointer to PCIe Controller IP configuration APB registers.
     * Accessible in both RC & EP modes.
     */
    struct pcie_ctrl_apb_reg *ctrl_apb;

    /*
     * Base address of the PCIe PHY APB registers.
     * Accessible in both RC & EP modes.
     */
    uintptr_t phy_apb;

    /*
     * Base address of the PCIe configuration APB registers.
     * Accessible in both RP & EP mode.
     */
    uintptr_t rp_ep_config_apb;

    /*
     * Base address of the PCIe Local Management (LM) registers.
     * Accessible in both RC & EP modes.
     */
    uintptr_t lm_apb;

    /*
     * Base address of the AXI configuration registers for RC.
     * Accessible in RC mode.
     */
    uintptr_t rc_axi_config_apb;

    /*
     * Base address of the AXI configuration registers for EP.
     * Accessible in EP mode.
     */
    uintptr_t ep_axi_config_apb;
};

/*
 * Module context
 */
struct n1sdp_pcie_ctx {
    /* Timer module API */
    struct mod_timer_api *timer_api;

    /* C2C API to check if slave chip is connected */
    struct n1sdp_c2c_slave_info_api *c2c_api;

    /* Table of PCIe device contexts */
    struct n1sdp_pcie_dev_ctx *device_ctx_table;

    /* Number of PCIe root complexes/endpoints in the system */
    unsigned int pcie_instance_count;
};

struct n1sdp_pcie_ctx pcie_ctx;

static const char * const pcie_type[2] = {"PCIe", "CCIX"};

/*
 * CCIX configuration API
 */
static int n1sdp_pcie_ccix_enable_opt_tlp(bool enable)
{
    uint32_t value;
    unsigned int i;
    struct n1sdp_pcie_dev_ctx *dev_ctx = NULL;
    struct n1sdp_pcie_dev_config *config = NULL;

    for (i = 0; i <= pcie_ctx.pcie_instance_count; i++) {
        dev_ctx = &pcie_ctx.device_ctx_table[i];
        if (dev_ctx->config->ccix_capable) {
            config = dev_ctx->config;
            break;
        }
    }

    if (config == NULL)
        return FWK_E_DATA;

    /* Configure for the optimized header or pcie compatible header*/
    if (enable)
        value = (CCIX_CTRL_CAW | CCIX_CTRL_EN_OPT_TLP | CCIX_CTRL_CSTT_V0_V1 |
                 CCIX_VENDER_ID);
    else
        value = (CCIX_CTRL_CAW | CCIX_VENDER_ID);

    FWK_LOG_INFO("[CCIX] CCIX_CONTROL: 0x%" PRIX32, value);

    *(uint32_t *)(dev_ctx->lm_apb + PCIE_LM_RC_CCIX_CTRL_REG) = value;

    if (enable)
        dev_ctx->ctrl_apb->CCIX_CTRL = 0x1;

    return FWK_SUCCESS;
}

static const struct n1sdp_pcie_ccix_config_api pcie_ccix_config_api = {
    .enable_opt_tlp = n1sdp_pcie_ccix_enable_opt_tlp,
};

/*
 * PCIe initialization APIs
 */
static int n1sdp_pcie_power_on(fwk_id_t id)
{
    struct pcie_wait_condition_data wait_data;
    struct n1sdp_pcie_dev_ctx *dev_ctx;
    int status;
    unsigned int did;

    did = fwk_id_get_element_idx(id);
    dev_ctx = &pcie_ctx.device_ctx_table[did];
    if (dev_ctx == NULL)
        return FWK_E_PARAM;

    FWK_LOG_INFO("[%s] Powering ON controller...", pcie_type[did]);
    wait_data.ctrl_apb = NULL;
    if (dev_ctx->config->ccix_capable) {
        SCC->AXI_OVRD_CCIX = AXI_OVRD_VAL_CCIX;
        SCC->CCIX_PM_CTRL = SCC_CCIX_PM_CTRL_PWR_REQ_POS;
        wait_data.stage = PCIE_INIT_STAGE_CCIX_POWER_ON;
        status = pcie_ctx.timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER,
                                                         0),
                                          PCIE_POWER_ON_TIMEOUT,
                                          pcie_wait_condition,
                                          &wait_data);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[%s] Timeout!", pcie_type[did]);
            return status;
        }
        SCC->SYS_MAN_RESET &= ~(1 << SCC_SYS_MAN_RESET_CCIX_POS);
    } else {
        SCC->AXI_OVRD_PCIE = AXI_OVRD_VAL_PCIE;
        SCC->PCIE_PM_CTRL = SCC_PCIE_PM_CTRL_PWR_REQ_POS;
        wait_data.stage = PCIE_INIT_STAGE_PCIE_POWER_ON;
        status = pcie_ctx.timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER,
                                                         0),
                                          PCIE_POWER_ON_TIMEOUT,
                                          pcie_wait_condition,
                                          &wait_data);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[%s] Timeout!", pcie_type[did]);
            return status;
        }
        SCC->SYS_MAN_RESET &= ~(1 << SCC_SYS_MAN_RESET_PCIE_POS);
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    return FWK_SUCCESS;
}

static int n1sdp_pcie_phy_init(fwk_id_t id)
{
    struct n1sdp_pcie_dev_ctx *dev_ctx;
    enum pcie_gen gen_speed;
    int status;
    unsigned int did;

    did = fwk_id_get_element_idx(id);
    dev_ctx = &pcie_ctx.device_ctx_table[did];
    if (dev_ctx == NULL)
        return FWK_E_PARAM;

    gen_speed = dev_ctx->config->ccix_capable ? PCIE_GEN_4 : PCIE_GEN_3;

    FWK_LOG_INFO("[%s] Initializing PHY...", pcie_type[did]);

    pcie_phy_init(dev_ctx->phy_apb);
    status = pcie_init(dev_ctx->ctrl_apb,
                       pcie_ctx.timer_api,
                       PCIE_INIT_STAGE_PHY,
                       gen_speed);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[%s] Timeout!", pcie_type[did]);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    return FWK_SUCCESS;
}

static int n1sdp_pcie_controller_init(fwk_id_t id, bool ep_mode)
{
    struct n1sdp_pcie_dev_ctx *dev_ctx;
    enum pcie_gen gen_speed;
    int status;
    int did;

    did = fwk_id_get_element_idx(id);
    dev_ctx = &pcie_ctx.device_ctx_table[did];
    if (dev_ctx == NULL)
        return FWK_E_PARAM;

    gen_speed = dev_ctx->config->ccix_capable ? PCIE_GEN_4 : PCIE_GEN_3;

    if (ep_mode) {
        dev_ctx->ctrl_apb->MODE_CTRL = 0x0;
        dev_ctx->ctrl_apb->EP_MISC_CTRL |= 0x100;
    }

    FWK_LOG_INFO(
        "[%s] Initializing controller in %s mode...",
        pcie_type[did],
        (ep_mode ? "endpoint" : "root port"));
    status = pcie_init(dev_ctx->ctrl_apb,
                       pcie_ctx.timer_api,
                       PCIE_INIT_STAGE_CTRL,
                       gen_speed);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[%s] Timeout!", pcie_type[did]);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    return FWK_SUCCESS;
}

static int n1sdp_pcie_link_training(fwk_id_t id, bool ep_mode)
{
    struct n1sdp_pcie_dev_ctx *dev_ctx;
    enum pcie_gen gen_speed;
    uint8_t neg_config;
    uint32_t reg_val;
    int status;
    unsigned int did;

    did = fwk_id_get_element_idx(id);
    dev_ctx = &pcie_ctx.device_ctx_table[did];
    if (dev_ctx == NULL)
        return FWK_E_PARAM;

    gen_speed = dev_ctx->config->ccix_capable ? PCIE_GEN_4 : PCIE_GEN_3;

    if (gen_speed >= PCIE_GEN_3 && !ep_mode) {
        FWK_LOG_INFO(
            "[%s] Setting TX Preset for GEN%d...",
            pcie_type[did],
            gen_speed + 1);
        status = pcie_set_gen_tx_preset(dev_ctx->rp_ep_config_apb,
                                        TX_PRESET_VALUE,
                                        gen_speed);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[%s] Error!", pcie_type[did]);
            return status;
        }
        FWK_LOG_INFO("[%s] Done", pcie_type[did]);
    }

    /* Link training */
    FWK_LOG_INFO("[%s] Starting link training...", pcie_type[did]);
    status = pcie_init(dev_ctx->ctrl_apb,
                       pcie_ctx.timer_api,
                       PCIE_INIT_STAGE_LINK_TRNG,
                       gen_speed);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[%s] Timeout!", pcie_type[did]);
        pcie_init_bdf_table(dev_ctx->config);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    neg_config = (dev_ctx->ctrl_apb->RP_CONFIG_OUT &
        RP_CONFIG_OUT_NEGOTIATED_SPD_MASK) >> RP_CONFIG_OUT_NEGOTIATED_SPD_POS;
    FWK_LOG_INFO(
        "[%s] Negotiated speed: GEN%d", pcie_type[did], neg_config + 1);

    neg_config = (dev_ctx->ctrl_apb->RP_CONFIG_OUT &
        RP_CONFIG_OUT_NEGOTIATED_LINK_WIDTH_MASK) >>
        RP_CONFIG_OUT_NEGOTIATED_LINK_WIDTH_POS;
    FWK_LOG_INFO(
        "[%s] Negotiated link width: x%d",
        pcie_type[did],
        fwk_math_pow2(neg_config));

    if (gen_speed == PCIE_GEN_4) {
        FWK_LOG_INFO("[%s] Re-training link to GEN4 speed...", pcie_type[did]);
        /* Set GEN4 as target speed */
        pcie_rp_ep_config_read_word(dev_ctx->rp_ep_config_apb,
                                    PCIE_LINK_CTRL_STATUS_2_OFFSET, &reg_val);
        reg_val &= ~PCIE_LINK_CTRL_2_TARGET_SPEED_MASK;
        reg_val |= PCIE_LINK_CTRL_2_TARGET_SPEED_GEN4;
        pcie_rp_ep_config_write_word(dev_ctx->rp_ep_config_apb,
                                     PCIE_LINK_CTRL_STATUS_2_OFFSET, reg_val);

        /* Start link retraining */
        status = pcie_link_retrain(dev_ctx->ctrl_apb,
                                   dev_ctx->rp_ep_config_apb,
                                   pcie_ctx.timer_api);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[%s] TIMEOUT", pcie_type[did]);
            return FWK_SUCCESS;
        }
        FWK_LOG_INFO("[%s] Done", pcie_type[did]);

        pcie_rp_ep_config_read_word(dev_ctx->rp_ep_config_apb,
                                    PCIE_LINK_CTRL_STATUS_OFFSET, &reg_val);
        neg_config = (reg_val >> PCIE_LINK_CTRL_NEG_SPEED_POS) &
                     PCIE_LINK_CTRL_NEG_SPEED_MASK;
        FWK_LOG_INFO(
            "[%s] Re-negotiated speed: GEN%d", pcie_type[did], neg_config);

        neg_config = (reg_val >> PCIE_LINK_CTRL_NEG_WIDTH_POS) &
                     PCIE_LINK_CTRL_NEG_WIDTH_MASK;
        FWK_LOG_INFO(
            "[%s] Re-negotiated link width: x%d", pcie_type[did], neg_config);
    }

    return FWK_SUCCESS;
}

static int n1sdp_pcie_rc_setup(fwk_id_t id)
{
    struct n1sdp_pcie_dev_ctx *dev_ctx;
    uint32_t ecam_base_addr;
    int status;
    unsigned int did;

    did = fwk_id_get_element_idx(id);
    dev_ctx = &pcie_ctx.device_ctx_table[did];
    if (dev_ctx == NULL)
        return FWK_E_PARAM;

    FWK_LOG_INFO("[%s] Setup Type0 configuration...", pcie_type[did]);
    if (dev_ctx->config->ccix_capable)
        ecam_base_addr = dev_ctx->config->axi_slave_base32 +
                         CCIX_AXI_ECAM_TYPE0_OFFSET;
    else
        ecam_base_addr = dev_ctx->config->axi_slave_base32 +
                         PCIE_AXI_ECAM_TYPE0_OFFSET;
    status = axi_outbound_region_setup(dev_ctx->rc_axi_config_apb,
                 (ecam_base_addr - SCP_AP_AXI_OFFSET),
                 __builtin_ctz(AXI_ECAM_TYPE0_SIZE),
                 TRANS_TYPE_0_CFG);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[%s] Error!", pcie_type[did]);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    FWK_LOG_INFO("[%s] Setup Type1 configuration...", pcie_type[did]);
    if (dev_ctx->config->ccix_capable)
        ecam_base_addr = dev_ctx->config->axi_slave_base32 +
                         CCIX_AXI_ECAM_TYPE1_OFFSET;
    else
        ecam_base_addr = dev_ctx->config->axi_slave_base32 +
                         PCIE_AXI_ECAM_TYPE1_OFFSET;
    status = axi_outbound_region_setup(dev_ctx->rc_axi_config_apb,
                 (ecam_base_addr - SCP_AP_AXI_OFFSET),
                 __builtin_ctz(AXI_ECAM_TYPE1_SIZE),
                 TRANS_TYPE_1_CFG);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[%s] Error!", pcie_type[did]);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    FWK_LOG_INFO("[%s] Setup MMIO32 configuration...", pcie_type[did]);
    if (dev_ctx->config->ccix_capable)
        ecam_base_addr = dev_ctx->config->axi_slave_base32 +
                             CCIX_AXI_MMIO32_OFFSET;
    else
        ecam_base_addr = dev_ctx->config->axi_slave_base32 +
                             PCIE_AXI_MMIO32_OFFSET;
    status = axi_outbound_region_setup(dev_ctx->rc_axi_config_apb,
                 (ecam_base_addr - SCP_AP_AXI_OFFSET),
                 __builtin_ctz(AXI_MMIO32_SIZE),
                 TRANS_TYPE_MEM_IO);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[%s] Error!", pcie_type[did]);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    FWK_LOG_INFO("[%s] Setup IO configuration...", pcie_type[did]);
    if (dev_ctx->config->ccix_capable)
        ecam_base_addr = dev_ctx->config->axi_slave_base32 +
                             CCIX_AXI_IO_OFFSET;
    else
        ecam_base_addr = dev_ctx->config->axi_slave_base32 +
                             PCIE_AXI_IO_OFFSET;
    status = axi_outbound_region_setup(dev_ctx->rc_axi_config_apb,
                 (ecam_base_addr - SCP_AP_AXI_OFFSET),
                 __builtin_ctz(AXI_IO_SIZE),
                 TRANS_TYPE_IO);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[%s] Error!", pcie_type[did]);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    FWK_LOG_INFO("[CCIX] [%s] Setup MMIO64 configuration...", pcie_type[did]);
    status = axi_outbound_region_setup(dev_ctx->rc_axi_config_apb,
                 dev_ctx->config->axi_slave_base64,
                 __builtin_ctz(AXI_MMIO64_SIZE),
                 TRANS_TYPE_MEM_IO);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[%s] Error!", pcie_type[did]);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    FWK_LOG_INFO("[%s] Setup RP classcode...", pcie_type[did]);
    status = pcie_rp_ep_config_write_word(dev_ctx->rp_ep_config_apb,
                                          PCIE_CLASS_CODE_OFFSET,
                                          PCIE_CLASS_CODE_PCI_BRIDGE);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("Error!");
        return status;
    }
    FWK_LOG_INFO("Done");

    FWK_LOG_INFO(
        "[%s] Enable inbound region in BAR 2...",
        pcie_type[did]);
    status = axi_inbound_region_setup(dev_ctx->rc_axi_config_apb,
                 AXI_IB_REGION_BASE,
                 AXI_IB_REGION_SIZE_MSB, 2);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[%s] Error!", pcie_type[did]);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    FWK_LOG_INFO("[%s] Enable Type 1 I/O configuration", pcie_type[did]);
    *(uint32_t *)(dev_ctx->lm_apb + PCIE_LM_RC_BAR_CONFIG_REG) =
        (TYPE1_PREF_MEM_BAR_ENABLE_MASK |
         TYPE1_PREF_MEM_BAR_SIZE_64BIT_MASK |
         TYPE1_PREF_IO_BAR_ENABLE_MASK |
         TYPE1_PREF_IO_BAR_SIZE_32BIT_MASK);

    FWK_LOG_INFO("[%s] Skipping ATS capability...", pcie_type[did]);
    status = pcie_skip_ext_cap(dev_ctx->rp_ep_config_apb, EXT_CAP_ID_ATS);
    if (status != FWK_SUCCESS)
        FWK_LOG_INFO("[%s] Not found!", pcie_type[did]);
    else
        FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    FWK_LOG_INFO("[%s] Skipping PRI capability...", pcie_type[did]);
    status = pcie_skip_ext_cap(dev_ctx->rp_ep_config_apb, EXT_CAP_ID_PRI);
    if (status != FWK_SUCCESS)
        FWK_LOG_INFO("[%s] Not found!", pcie_type[did]);
    else
        FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    /*
     * Wait until devices connected in downstream ports
     * finish link training before doing bus enumeration
     */
    pcie_ctx.timer_api->delay(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                 PCIE_LINK_TRAINING_TIMEOUT);

    pcie_bus_enumeration(dev_ctx->config);

    return FWK_SUCCESS;
}

static int n1sdp_pcie_vc1_setup(fwk_id_t id, uint8_t vc1_tc)
{
    struct n1sdp_pcie_dev_ctx *dev_ctx;
    uint32_t config_base_addr;
    int status;

    dev_ctx = &pcie_ctx.device_ctx_table[fwk_id_get_element_idx(id)];
    if (dev_ctx == NULL)
        return FWK_E_PARAM;

    if (!dev_ctx->config->ccix_capable || (vc1_tc > 7) || (vc1_tc == 0))
        return FWK_E_PARAM;

    config_base_addr = dev_ctx->rp_ep_config_apb;

    FWK_LOG_INFO(
        "[CCIX] Enabling VC1 in RP 0x%" PRIX32 "...", config_base_addr);

    status = pcie_vc_setup(config_base_addr, vc1_tc);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[CCIX] Error!");
        return status;
    }
    FWK_LOG_INFO("[CCIX] Done");

    /* Set max payload size to 512 */
    *(volatile uint32_t *)(config_base_addr + PCIE_DEV_CTRL_STATUS_OFFSET) |=
        (0x2 << PCIE_DEV_CTRL_MAX_PAYLOAD_SHIFT);

    config_base_addr = dev_ctx->config->axi_slave_base32 + 0x100000;

    FWK_LOG_INFO(
        "[CCIX] Enabling VC1 in EP 0x%" PRIX32 "...", config_base_addr);

    status = pcie_vc_setup(config_base_addr, vc1_tc);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[CCIX] Error!");
        return status;
    }
    FWK_LOG_INFO("[CCIX] Done");

    *(volatile uint32_t *)(config_base_addr + PCIE_DEV_CTRL_STATUS_OFFSET) |=
        (0x2 << PCIE_DEV_CTRL_MAX_PAYLOAD_SHIFT);

    return FWK_SUCCESS;
}

static const struct n1sdp_pcie_init_api pcie_init_api = {
    .power_on = n1sdp_pcie_power_on,
    .phy_init = n1sdp_pcie_phy_init,
    .controller_init = n1sdp_pcie_controller_init,
    .link_training = n1sdp_pcie_link_training,
    .rc_setup = n1sdp_pcie_rc_setup,
    .vc1_setup = n1sdp_pcie_vc1_setup,
};

/*
 * Module functions
 */
static int n1sdp_pcie_setup(fwk_id_t id)
{
    struct n1sdp_pcie_dev_ctx *dev_ctx;
    int status;

    dev_ctx = &pcie_ctx.device_ctx_table[fwk_id_get_element_idx(id)];
    if (dev_ctx == NULL)
        return FWK_E_PARAM;

    /* PCIe controller power ON */
    status = n1sdp_pcie_power_on(id);
    if (status != FWK_SUCCESS)
        return status;

    /* PCIe PHY initialization */
    status = n1sdp_pcie_phy_init(id);
    if (status != FWK_SUCCESS)
        return status;

    /* PCIe controller initialization */
    status = n1sdp_pcie_controller_init(id, false);
    if (status != FWK_SUCCESS)
        return status;

    /* Link training */
    status = n1sdp_pcie_link_training(id, false);
    if (status != FWK_SUCCESS)
        return dev_ctx->config->ccix_capable ? FWK_SUCCESS : status;

    /* Root Complex setup */
    status = n1sdp_pcie_rc_setup(id);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

/*
 * Framework handlers
 */
static int n1sdp_pcie_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    if (element_count == 0)
        return FWK_E_DATA;

    pcie_ctx.device_ctx_table = fwk_mm_calloc(element_count,
        sizeof(pcie_ctx.device_ctx_table[0]));

    pcie_ctx.pcie_instance_count = element_count;

    return FWK_SUCCESS;
}

static int n1sdp_pcie_element_init(fwk_id_t element_id, unsigned int unused,
                                  const void *data)
{
    struct n1sdp_pcie_dev_ctx *dev_ctx;
    struct n1sdp_pcie_dev_config *config;

    if (data == NULL)
        return FWK_E_PARAM;

    config = (struct n1sdp_pcie_dev_config *)data;

    dev_ctx = &pcie_ctx.device_ctx_table[fwk_id_get_element_idx(element_id)];
    if (dev_ctx == NULL)
        return FWK_E_DATA;

    dev_ctx->config = config;

    dev_ctx->ctrl_apb = (struct pcie_ctrl_apb_reg *)
                            (config->ctrl_base + APB_OFFSET_CTRL_REGS);
    dev_ctx->phy_apb = config->ctrl_base + APB_OFFSET_PHY_REGS;
    dev_ctx->rp_ep_config_apb = config->global_config_base +
                                APB_OFFSET_RP_EP_CONFIG_REGS;
    dev_ctx->lm_apb = config->global_config_base + APB_OFFSET_LM_REGS;
    dev_ctx->rc_axi_config_apb = config->global_config_base +
                                 APB_OFFSET_RC_AXI_CONFIG_REGS;
    dev_ctx->ep_axi_config_apb = config->global_config_base +
                                 APB_OFFSET_EP_AXI_CONFIG_REGS;

    return FWK_SUCCESS;
}


static int n1sdp_pcie_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 0) {
        status = fwk_module_bind(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER),
            &pcie_ctx.timer_api);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_C2C),
            FWK_ID_API(FWK_MODULE_IDX_N1SDP_C2C, N1SDP_C2C_API_IDX_SLAVE_INFO),
            &pcie_ctx.c2c_api);
        if (status != FWK_SUCCESS)
            return status;
    }
    return FWK_SUCCESS;
}

static int n1sdp_pcie_start(fwk_id_t id)
{
    struct n1sdp_pcie_dev_ctx *dev_ctx;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /*
         * Enable AP core to access PCIe root port's
         * configuration space
         */
        *(volatile uint32_t *)(NIC400_SOC_GPV_BASE + 0x0C) = 0x7F;
        return FWK_SUCCESS;
    }

    dev_ctx = &pcie_ctx.device_ctx_table[fwk_id_get_element_idx(id)];
    if (dev_ctx == NULL)
        return FWK_E_PARAM;

    /* Do not initialize PCIe RP in slave chip */
    if (!dev_ctx->config->ccix_capable) {
        if (n1sdp_get_chipid() != 0)
            return FWK_SUCCESS;
    }

    return fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        FWK_ID_ELEMENT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_INTERCONNECT),
        id);
}

static int n1sdp_pcie_process_bind_request(fwk_id_t requester_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case N1SDP_PCIE_API_IDX_PCIE_INIT:
        *api = &pcie_init_api;
        break;
    case N1SDP_PCIE_API_IDX_CCIX_CONFIG:
        *api = &pcie_ccix_config_api;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int n1sdp_pcie_process_notification(const struct fwk_event *event,
                                          struct fwk_event *resp)
{
    struct n1sdp_pcie_dev_ctx *dev_ctx;

    dev_ctx = &pcie_ctx.device_ctx_table[
                  fwk_id_get_element_idx(event->target_id)];
    if (dev_ctx == NULL)
        return FWK_E_PARAM;

    /*
     * The CCIX RP should not be initialized by n1sdp_pcie_setup() function
     * in two special cases:
     *     1. In case of slave chip as it will be initialized by C2C module
     *        in endpoint mode.
     *     2. In case of master chip if the slave I2C is alive & responding
     *        then it will be initialized by C2C module in RP mode.
     */
    if (dev_ctx->config->ccix_capable) {
        if (pcie_ctx.c2c_api->is_slave_alive() || (n1sdp_get_chipid() != 0))
            return FWK_SUCCESS;
    }

    return n1sdp_pcie_setup(event->target_id);
}

const struct fwk_module module_n1sdp_pcie = {
    .name = "N1SDP PCIe",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = N1SDP_PCIE_API_COUNT,
    .init = n1sdp_pcie_init,
    .element_init = n1sdp_pcie_element_init,
    .bind = n1sdp_pcie_bind,
    .start = n1sdp_pcie_start,
    .process_bind_request = n1sdp_pcie_process_bind_request,
    .process_notification = n1sdp_pcie_process_notification,
};
