/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_id.h>
#include <fwk_math.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <mod_clock.h>
#include <mod_n1sdp_pcie.h>
#include <mod_log.h>
#include <mod_timer.h>
#include <config_clock.h>
#include <n1sdp_pcie.h>
#include <n1sdp_scc_reg.h>
#include <n1sdp_scp_pik.h>
#include <internal/pcie_ctrl_apb_reg.h>

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
    /* Log module API */
    struct mod_log_api *log_api;

    /* Timer module API */
    struct mod_timer_api *timer_api;

    /* Table of PCIe device contexts */
    struct n1sdp_pcie_dev_ctx *device_ctx_table;

    /* Number of PCIe root complexes/endpoints in the system */
    unsigned int pcie_instance_count;
};

struct n1sdp_pcie_ctx pcie_ctx;

/*
 * Module functions
 */
static int n1sdp_pcie_setup(struct n1sdp_pcie_dev_ctx *dev_ctx)
{
    uint32_t ecam_base_addr;
    uint8_t neg_config;
    struct pcie_wait_condition_data wait_data;
    int status;
    enum pcie_gen gen_speed;

    gen_speed = dev_ctx->config->ccix_capable ? PCIE_GEN_4 : PCIE_GEN_3;

    /* Enable the CCIX/PCIe controller */
    wait_data.ctrl_apb = NULL;
    if (dev_ctx->config->ccix_capable) {
        SCC->CCIX_PM_CTRL = SCC_CCIX_PM_CTRL_PWR_REQ_POS;
        wait_data.stage = PCIE_INIT_STAGE_CCIX_POWER_ON;
        status = pcie_ctx.timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER,
                                                         0),
                                          PCIE_POWER_ON_TIMEOUT,
                                          pcie_wait_condition,
                                          &wait_data);
        if (status != FWK_SUCCESS) {
            pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
                "[PCIe] Controller power-on failed!\n");
            return status;
        }
        SCC->SYS_MAN_RESET &= ~(1 << SCC_SYS_MAN_RESET_CCIX_POS);
    } else {
        SCC->PCIE_PM_CTRL = SCC_PCIE_PM_CTRL_PWR_REQ_POS;
        wait_data.stage = PCIE_INIT_STAGE_PCIE_POWER_ON;
        status = pcie_ctx.timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER,
                                                         0),
                                          PCIE_POWER_ON_TIMEOUT,
                                          pcie_wait_condition,
                                          &wait_data);
        if (status != FWK_SUCCESS) {
            pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
                "[PCIe] Controller power-on failed!\n");
            return status;
        }
        SCC->SYS_MAN_RESET &= ~(1 << SCC_SYS_MAN_RESET_PCIE_POS);
    }

    /* PHY initialization */
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "[PCIe] Initializing PHY...");

    pcie_phy_init(dev_ctx->phy_apb, gen_speed);
    status = pcie_init(dev_ctx->ctrl_apb,
                       pcie_ctx.timer_api,
                       PCIE_INIT_STAGE_PHY,
                       gen_speed);
    if (status != FWK_SUCCESS) {
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Timeout!\n");
        return status;
    }
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    /* Controller initialization */
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
                          "[PCIe] Initializing controller...");
    status = pcie_init(dev_ctx->ctrl_apb,
                       pcie_ctx.timer_api,
                       PCIE_INIT_STAGE_CTRL,
                       gen_speed);
    if (status != FWK_SUCCESS) {
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Timeout!\n");
        return status;
    }
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    status = pcie_set_gen_tx_preset(dev_ctx->rp_ep_config_apb,
                                    TX_PRESET_VALUE,
                                    gen_speed);
    if (status != FWK_SUCCESS) {
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Equalization failed!\n");
        return status;
    }

    /* Link training */
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
                          "[PCIe] Starting link training...");
    status = pcie_init(dev_ctx->ctrl_apb,
                       pcie_ctx.timer_api,
                       PCIE_INIT_STAGE_LINK_TRNG,
                       gen_speed);
    if (status != FWK_SUCCESS) {
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Timeout!\n");
        return dev_ctx->config->ccix_capable ? FWK_SUCCESS : status;
    }
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    neg_config = (dev_ctx->ctrl_apb->RP_CONFIG_OUT &
        RP_CONFIG_OUT_NEGOTIATED_SPD_MASK) >> RP_CONFIG_OUT_NEGOTIATED_SPD_POS;
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[PCIe] Negotiated speed: GEN%d\n", neg_config + 1);

    neg_config = (dev_ctx->ctrl_apb->RP_CONFIG_OUT &
        RP_CONFIG_OUT_NEGOTIATED_LINK_WIDTH_MASK) >>
        RP_CONFIG_OUT_NEGOTIATED_LINK_WIDTH_POS;
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[PCIe] Negotiated link width: x%d\n", fwk_math_pow2(neg_config));

    /* Root Complex setup */
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
                          "[PCIe] Setup Type0 configuration...");
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
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Error!\n");
        return status;
    }
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[PCIe] Setup Type1 configuration...");
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
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Error!\n");
        return status;
    }
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[PCIe] Setup MMIO32 configuration...");
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
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Error!\n");
        return status;
    }
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[PCIe] Setup IO configuration...");
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
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Error!\n");
        return status;
    }
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[PCIe] Setup MMIO64 configuration...");
    status = axi_outbound_region_setup(dev_ctx->rc_axi_config_apb,
                 dev_ctx->config->axi_slave_base64,
                 __builtin_ctz(AXI_MMIO64_SIZE),
                 TRANS_TYPE_MEM_IO);
    if (status != FWK_SUCCESS) {
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Error!\n");
        return status;
    }
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[PCIe] Setup RP classcode...");
    status = pcie_rp_ep_config_write_word(dev_ctx->rp_ep_config_apb,
                                          PCIE_CLASS_CODE_OFFSET,
                                          PCIE_CLASS_CODE_PCI_BRIDGE);
    if (status != FWK_SUCCESS) {
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Error!\n");
        return status;
    }
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[PCIe] Enable inbound region in BAR 2...");
    status = axi_inbound_region_setup(dev_ctx->rc_axi_config_apb,
                 AXI_IB_REGION_BASE,
                 AXI_IB_REGION_SIZE_MSB, 2);
    if (status != FWK_SUCCESS) {
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Error!\n");
        return status;
    }
    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[PCIe] Enable Type 1 I/O configuration\n");
    *(uint32_t *)(dev_ctx->lm_apb + PCIE_LM_RC_BAR_CONFIG_REG) =
        (TYPE1_PREF_MEM_BAR_ENABLE_MASK |
         TYPE1_PREF_MEM_BAR_SIZE_64BIT_MASK |
         TYPE1_PREF_IO_BAR_ENABLE_MASK |
         TYPE1_PREF_IO_BAR_SIZE_32BIT_MASK);

    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[PCIe] Skipping ATS capability...");
    status = pcie_skip_ext_cap(dev_ctx->rp_ep_config_apb, EXT_CAP_ID_ATS);
    if (status != FWK_SUCCESS)
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Not found!\n");
    else
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

    pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[PCIe] Skipping PRI capability...");
    status = pcie_skip_ext_cap(dev_ctx->rp_ep_config_apb, EXT_CAP_ID_PRI);
    if (status != FWK_SUCCESS)
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Not found!\n");
    else
        pcie_ctx.log_api->log(MOD_LOG_GROUP_INFO, "Done\n");

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
    if (pcie_ctx.device_ctx_table == NULL)
        return FWK_E_NOMEM;

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
        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_LOG),
            FWK_ID_API(FWK_MODULE_IDX_LOG, 0), &pcie_ctx.log_api);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_module_bind(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER),
            &pcie_ctx.timer_api);
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

    return fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        FWK_ID_ELEMENT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_INTERCONNECT),
        id);
}

static int n1sdp_pcie_process_notification(const struct fwk_event *event,
                                          struct fwk_event *resp)
{
    struct n1sdp_pcie_dev_ctx *dev_ctx;

    dev_ctx = &pcie_ctx.device_ctx_table[
                  fwk_id_get_element_idx(event->target_id)];
    if (dev_ctx == NULL)
        return FWK_E_PARAM;

    return n1sdp_pcie_setup(dev_ctx);
}

const struct fwk_module module_n1sdp_pcie = {
    .name = "N1SDP PCIe",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 0,
    .init = n1sdp_pcie_init,
    .element_init = n1sdp_pcie_element_init,
    .bind = n1sdp_pcie_bind,
    .start = n1sdp_pcie_start,
    .process_notification = n1sdp_pcie_process_notification,
};
