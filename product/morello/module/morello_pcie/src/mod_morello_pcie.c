/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "morello_core.h"
#include "morello_scc_reg.h"
#include "morello_scp_pik.h"

#include <morello_pcie.h>

#include <internal/pcie_ctrl_apb_reg.h>

#include <mod_clock.h>
#include <mod_morello_pcie.h>
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

/*
 * Device context
 */
struct morello_pcie_dev_ctx {
    /* Pointer to PCIe device configuration */
    struct morello_pcie_dev_config *config;

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
struct morello_pcie_ctx {
    /* Timer module API */
    struct mod_timer_api *timer_api;

    /* Table of PCIe device contexts */
    struct morello_pcie_dev_ctx *device_ctx_table;

    /* Number of PCIe root complexes/endpoints in the system */
    unsigned int pcie_instance_count;
};

static struct morello_pcie_ctx pcie_ctx;

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
static const char *const pcie_type[2] = { "PCIe", "CCIX" };
#endif

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
static const char *const axi_ob_mmap_type_name[] = {
    [PCIE_AXI_OB_REGION_TYPE_MMIO] = "MMIO",
    [PCIE_AXI_OB_REGION_TYPE_IO] = "IO",
    [PCIE_AXI_OB_REGION_TYPE_ECAM] = "ECAM",
    [PCIE_AXI_OB_REGION_TYPE_MSG] = "MSG",
    [PCIE_AXI_OB_REGION_TYPE_VDM] = "VDM",
};
#endif

/*
 * CCIX configuration API
 */
static int morello_pcie_ccix_enable_opt_tlp(bool enable)
{
    uint32_t value;
    unsigned int i;
    struct morello_pcie_dev_ctx *dev_ctx = NULL;
    struct morello_pcie_dev_config *config = NULL;

    for (i = 0; i <= pcie_ctx.pcie_instance_count; i++) {
        dev_ctx = &pcie_ctx.device_ctx_table[i];
        if (dev_ctx->config->ccix_capable) {
            config = dev_ctx->config;
            break;
        }
    }

    if (config == NULL) {
        return FWK_E_DATA;
    }

    /* Configure for the optimized header or pcie compatible header*/
    if (enable) {
        value =
            (CCIX_CTRL_CAW | CCIX_CTRL_EN_OPT_TLP | CCIX_CTRL_CSTT_V0_V1 |
             CCIX_VENDOR_ID);
    } else {
        value = (CCIX_CTRL_CAW | CCIX_VENDOR_ID);
    }

    FWK_LOG_INFO("[CCIX] CCIX_CONTROL: 0x%" PRIX32, value);

    *(uint32_t *)(dev_ctx->lm_apb + PCIE_LM_RC_CCIX_CTRL_REG) = value;

    if (enable) {
        dev_ctx->ctrl_apb->CCIX_CTRL = 0x1;
    }

    return FWK_SUCCESS;
}

static const struct morello_pcie_ccix_config_api pcie_ccix_config_api = {
    .enable_opt_tlp = morello_pcie_ccix_enable_opt_tlp,
};

/*
 * PCIe initialization APIs
 */
static int morello_pcie_power_on(fwk_id_t id)
{
    struct pcie_wait_condition_data wait_data;
    struct morello_pcie_dev_ctx *dev_ctx;
    int status;
    unsigned int did;

    did = fwk_id_get_element_idx(id);
    dev_ctx = &pcie_ctx.device_ctx_table[did];
    if (dev_ctx == NULL) {
        return FWK_E_PARAM;
    }

    FWK_LOG_INFO("[%s] Powering ON controller...", pcie_type[did]);
    wait_data.ctrl_apb = NULL;
    if (dev_ctx->config->ccix_capable) {
        SCC->AXI_OVRD_CCIX = AXI_OVRD_VAL_CCIX;
        SCC->CCIX_PM_CTRL = SCC_CCIX_PM_CTRL_PWR_REQ_POS;
        wait_data.stage = PCIE_INIT_STAGE_CCIX_POWER_ON;
        status = pcie_ctx.timer_api->wait(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            PCIE_POWER_ON_TIMEOUT,
            pcie_wait_condition,
            &wait_data);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR("[%s] Timeout!", pcie_type[did]);
            return status;
        }
        SCC->SYS_MAN_RESET &= ~(1 << SCC_SYS_MAN_RESET_CCIX_POS);
    } else {
        SCC->AXI_OVRD_PCIE = AXI_OVRD_VAL_PCIE;
        SCC->PCIE_PM_CTRL = SCC_PCIE_PM_CTRL_PWR_REQ_POS;
        wait_data.stage = PCIE_INIT_STAGE_PCIE_POWER_ON;
        status = pcie_ctx.timer_api->wait(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            PCIE_POWER_ON_TIMEOUT,
            pcie_wait_condition,
            &wait_data);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR("[%s] Timeout!", pcie_type[did]);
            return status;
        }
        SCC->SYS_MAN_RESET &= ~(1 << SCC_SYS_MAN_RESET_PCIE_POS);
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    return FWK_SUCCESS;
}

static int morello_pcie_phy_init(fwk_id_t id)
{
    struct morello_pcie_dev_ctx *dev_ctx;
    enum pcie_gen gen_speed;
    int status;
    unsigned int did;
    enum pcie_lane_count lane_count;

    did = fwk_id_get_element_idx(id);
    dev_ctx = &pcie_ctx.device_ctx_table[did];
    if (dev_ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (dev_ctx->config->ccix_capable) {
        gen_speed = PCIE_GEN_4;
    } else {
        gen_speed = PCIE_GEN_3;
    }

    lane_count = LAN_COUNT_IN_X_16;

    FWK_LOG_INFO("[%s] Initializing PHY...", pcie_type[did]);

    pcie_phy_init(dev_ctx->phy_apb, lane_count);

    status = pcie_init(
        dev_ctx->ctrl_apb,
        pcie_ctx.timer_api,
        PCIE_INIT_STAGE_PHY,
        gen_speed,
        lane_count);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[%s] Timeout!", pcie_type[did]);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    return FWK_SUCCESS;
}

static int morello_pcie_controller_init(fwk_id_t id, bool ep_mode)
{
    struct morello_pcie_dev_ctx *dev_ctx;
    enum pcie_gen gen_speed;
    int status;
    int did;
    enum pcie_lane_count lane_count;

    did = fwk_id_get_element_idx(id);
    dev_ctx = &pcie_ctx.device_ctx_table[did];
    if (dev_ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (dev_ctx->config->ccix_capable) {
        gen_speed = PCIE_GEN_4;
    } else {
        gen_speed = PCIE_GEN_3;
    }

    lane_count = LAN_COUNT_IN_X_16;

    if (ep_mode) {
        dev_ctx->ctrl_apb->MODE_CTRL = 0x0;
        dev_ctx->ctrl_apb->EP_MISC_CTRL |= 0x100;
    }

    FWK_LOG_INFO(
        "[%s] Initializing controller in %s mode...",
        pcie_type[did],
        (ep_mode ? "endpoint" : "root port"));
    status = pcie_init(
        dev_ctx->ctrl_apb,
        pcie_ctx.timer_api,
        PCIE_INIT_STAGE_CTRL,
        gen_speed,
        lane_count);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[%s] Timeout!", pcie_type[did]);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    return FWK_SUCCESS;
}

static int morello_pcie_link_training(fwk_id_t id, bool ep_mode)
{
    struct morello_pcie_dev_ctx *dev_ctx;
    enum pcie_gen gen_speed;
    uint32_t reg_val;
    int status;
    unsigned int did;
    enum pcie_lane_count lane_count;
    uint32_t down_stream_tx_preset = 0;
    uint32_t up_stream_tx_preset = 0;
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
    uint8_t neg_config;
#endif

    did = fwk_id_get_element_idx(id);
    dev_ctx = &pcie_ctx.device_ctx_table[did];
    if (dev_ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (dev_ctx->config->ccix_capable) {
        gen_speed = PCIE_GEN_4;
        down_stream_tx_preset = CCIX_RC_TX_PRESET_VALUE;
        up_stream_tx_preset = CCIX_RC_TX_PRESET_VALUE;
    } else {
        gen_speed = PCIE_GEN_3;
        down_stream_tx_preset = PCIE_RC_TX_PRESET_VALUE;
        up_stream_tx_preset = PCIE_RC_TX_PRESET_VALUE;
    }

    lane_count = LAN_COUNT_IN_X_16;

    if (gen_speed >= PCIE_GEN_3 && !ep_mode) {
        FWK_LOG_INFO(
            "[%s] Setting TX Preset for GEN%d...",
            pcie_type[did],
            PCIE_GEN_3 + 1);
        status = pcie_set_gen_tx_preset(
            dev_ctx->rp_ep_config_apb,
            down_stream_tx_preset,
            up_stream_tx_preset,
            PCIE_GEN_3);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR("[%s] Error!", pcie_type[did]);
            return status;
        }
        if (gen_speed == PCIE_GEN_4) {
            FWK_LOG_INFO(
                "[%s] Setting TX Preset for GEN%d...",
                pcie_type[did],
                PCIE_GEN_4 + 1);
            status = pcie_set_gen_tx_preset(
                dev_ctx->rp_ep_config_apb,
                down_stream_tx_preset,
                up_stream_tx_preset,
                PCIE_GEN_4);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR("[%s] Error!", pcie_type[did]);
                return status;
            }
        }
        FWK_LOG_INFO("[%s] Done", pcie_type[did]);
    }

    /* Link training */
    FWK_LOG_INFO("[%s] Starting link training...", pcie_type[did]);
    status = pcie_init(
        dev_ctx->ctrl_apb,
        pcie_ctx.timer_api,
        PCIE_INIT_STAGE_LINK_TRNG,
        gen_speed,
        lane_count);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO("[%s] Timeout!", pcie_type[did]);
        return FWK_E_TIMEOUT;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
    (void)neg_config;
    neg_config = (dev_ctx->ctrl_apb->RP_CONFIG_OUT &
                  RP_CONFIG_OUT_NEGOTIATED_SPD_MASK) >>
        RP_CONFIG_OUT_NEGOTIATED_SPD_POS;
    FWK_LOG_INFO(
        "[%s] Negotiated speed: GEN%d", pcie_type[did], neg_config + 1);

    neg_config = (dev_ctx->ctrl_apb->RP_CONFIG_OUT &
                  RP_CONFIG_OUT_NEGOTIATED_LINK_WIDTH_MASK) >>
        RP_CONFIG_OUT_NEGOTIATED_LINK_WIDTH_POS;
    FWK_LOG_INFO(
        "[%s] Negotiated link width: x%d",
        pcie_type[did],
        fwk_math_pow2(neg_config));
#endif

    if (gen_speed == PCIE_GEN_4) {
        FWK_LOG_INFO("[%s] Re-training link to GEN4 speed...", pcie_type[did]);
        /* Set GEN4 as target speed */
        pcie_rp_ep_config_read_word(
            dev_ctx->rp_ep_config_apb,
            PCIE_LINK_CTRL_STATUS_2_OFFSET,
            &reg_val);
        reg_val &= ~PCIE_LINK_CTRL_2_TARGET_SPEED_MASK;
        reg_val |= PCIE_LINK_CTRL_2_TARGET_SPEED_GEN4;
        pcie_rp_ep_config_write_word(
            dev_ctx->rp_ep_config_apb, PCIE_LINK_CTRL_STATUS_2_OFFSET, reg_val);

        /* Start link retraining */
        status = pcie_link_retrain(
            dev_ctx->ctrl_apb, dev_ctx->rp_ep_config_apb, pcie_ctx.timer_api);
        if (status != FWK_SUCCESS) {
            FWK_LOG_INFO("[%s] TIMEOUT", pcie_type[did]);
            return FWK_E_TIMEOUT;
        }
        FWK_LOG_INFO("[%s] Done", pcie_type[did]);

        pcie_rp_ep_config_read_word(
            dev_ctx->rp_ep_config_apb, PCIE_LINK_CTRL_STATUS_OFFSET, &reg_val);

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
        neg_config = (reg_val >> PCIE_LINK_CTRL_NEG_SPEED_POS) &
            PCIE_LINK_CTRL_NEG_SPEED_MASK;
        FWK_LOG_INFO(
            "[%s] Re-negotiated speed: GEN%d", pcie_type[did], neg_config);

        neg_config = (reg_val >> PCIE_LINK_CTRL_NEG_WIDTH_POS) &
            PCIE_LINK_CTRL_NEG_WIDTH_MASK;
        FWK_LOG_INFO(
            "[%s] Re-negotiated link width: x%d", pcie_type[did], neg_config);
#endif
    }

    return FWK_SUCCESS;
}

static int morello_pcie_rc_setup(fwk_id_t id)
{
    struct morello_pcie_axi_ob_region_map *region;
    struct morello_pcie_dev_ctx *dev_ctx;
    unsigned int region_idx;
    int status;
    unsigned int did;

    did = fwk_id_get_element_idx(id);
    dev_ctx = &pcie_ctx.device_ctx_table[did];
    if (dev_ctx == NULL) {
        return FWK_E_PARAM;
    }

    FWK_LOG_INFO("[%s] AXI Outbound Region Setup:", pcie_type[did]);

    for (region_idx = 0; region_idx < dev_ctx->config->axi_ob_count;
         region_idx++) {
        region = &dev_ctx->config->axi_ob_table[region_idx];
        FWK_LOG_INFO(
            "[%s]    [0x%08" PRIX32 "%08" PRIX32 " - 0x%08" PRIX32 "%08" PRIX32
            "] %s",
            pcie_type[did],
            (uint32_t)(region->base >> 32),
            (uint32_t)region->base,
            (uint32_t)((region->base + region->size - 1) >> 32),
            (uint32_t)(region->base + region->size - 1),
            axi_ob_mmap_type_name[region->type]);

        if ((region->base % region->size) != 0) {
            FWK_LOG_ERR(
                "[%s]    Region base not aligned with size!", pcie_type[did]);
            return FWK_E_DATA;
        }

        status = axi_outbound_region_setup(
            dev_ctx->rc_axi_config_apb,
            region->base,
            __builtin_ctz(region->size),
            region_idx,
            (uint8_t)region->type);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                "[%s] Error during region setup: %d", pcie_type[did], status);
            return status;
        }
    }

    FWK_LOG_INFO("[%s] Setup RP classcode...", pcie_type[did]);
    status = pcie_rp_ep_config_write_word(
        dev_ctx->rp_ep_config_apb,
        PCIE_CLASS_CODE_OFFSET,
        PCIE_CLASS_CODE_PCI_BRIDGE);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("Error!");
        return status;
    }
    FWK_LOG_INFO("Done");

    FWK_LOG_INFO("[%s] Setup Primary bus number...", pcie_type[did]);
    *(uint32_t *)(dev_ctx->lm_apb + PCIE_LM_AXI_FEATURE_CONFIG) |=
        (dev_ctx->config->pri_bus_num << 8);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("Error!");
        return status;
    }
    FWK_LOG_INFO("Done");

    FWK_LOG_INFO("[%s] Enable inbound region in BAR 2...", pcie_type[did]);
    status = axi_inbound_region_setup(
        dev_ctx->rc_axi_config_apb,
        AXI_IB_REGION_BASE,
        AXI_IB_REGION_SIZE_MSB,
        2);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[%s] Error!", pcie_type[did]);
        return status;
    }
    FWK_LOG_INFO("[%s] Done", pcie_type[did]);

    FWK_LOG_INFO("[%s] Enable Type 1 I/O configuration", pcie_type[did]);
    *(uint32_t *)(dev_ctx->lm_apb + PCIE_LM_RC_BAR_CONFIG_REG) =
        (TYPE1_PREF_MEM_BAR_ENABLE_MASK | TYPE1_PREF_MEM_BAR_SIZE_64BIT_MASK |
         TYPE1_PREF_IO_BAR_ENABLE_MASK | TYPE1_PREF_IO_BAR_SIZE_32BIT_MASK);

    /*
     * Wait until devices connected in downstream ports
     * finish link training before doing bus enumeration
     */
    return pcie_ctx.timer_api->delay(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0), PCIE_LINK_TRAINING_TIMEOUT);
}

static int morello_pcie_vc1_setup(fwk_id_t id, uint8_t vc1_tc)
{
    struct morello_pcie_dev_ctx *dev_ctx;
    uint32_t config_base_addr;
    int status;

    dev_ctx = &pcie_ctx.device_ctx_table[fwk_id_get_element_idx(id)];
    if (dev_ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (!dev_ctx->config->ccix_capable || (vc1_tc > 7) || (vc1_tc == 0)) {
        return FWK_E_PARAM;
    }

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

    config_base_addr = dev_ctx->config->axi_subordinate_base32 + 0x100000;

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

static const struct morello_pcie_init_api pcie_init_api = {
    .power_on = morello_pcie_power_on,
    .phy_init = morello_pcie_phy_init,
    .controller_init = morello_pcie_controller_init,
    .link_training = morello_pcie_link_training,
    .rc_setup = morello_pcie_rc_setup,
    .vc1_setup = morello_pcie_vc1_setup,
};

/*
 * Module functions
 */
static int morello_pcie_setup(fwk_id_t id)
{
    struct morello_pcie_dev_ctx *dev_ctx;
    int status;

    dev_ctx = &pcie_ctx.device_ctx_table[fwk_id_get_element_idx(id)];
    if (dev_ctx == NULL) {
        return FWK_E_PARAM;
    }

    /* PCIe controller power ON */
    status = morello_pcie_power_on(id);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* PCIe PHY initialization */
    status = morello_pcie_phy_init(id);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* PCIe controller initialization */
    status = morello_pcie_controller_init(id, false);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Link training */
    status = morello_pcie_link_training(id, false);
    if (status != FWK_SUCCESS && !dev_ctx->config->ccix_capable) {
        return status;
    }

    /* Root Complex setup */
    status = morello_pcie_rc_setup(id);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

/*
 * Framework handlers
 */
static int morello_pcie_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0) {
        return FWK_E_DATA;
    }

    pcie_ctx.device_ctx_table =
        fwk_mm_calloc(element_count, sizeof(pcie_ctx.device_ctx_table[0]));

    pcie_ctx.pcie_instance_count = element_count;

    return FWK_SUCCESS;
}

static int morello_pcie_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    struct morello_pcie_dev_ctx *dev_ctx;
    struct morello_pcie_dev_config *config;

    if (data == NULL) {
        return FWK_E_PARAM;
    }

    config = (struct morello_pcie_dev_config *)data;

    dev_ctx = &pcie_ctx.device_ctx_table[fwk_id_get_element_idx(element_id)];
    if (dev_ctx == NULL) {
        return FWK_E_DATA;
    }

    dev_ctx->config = config;

    dev_ctx->ctrl_apb =
        (struct pcie_ctrl_apb_reg *)(config->ctrl_base + APB_OFFSET_CTRL_REGS);
    dev_ctx->phy_apb = config->ctrl_base + APB_OFFSET_PHY_REGS;
    dev_ctx->rp_ep_config_apb =
        config->global_config_base + APB_OFFSET_RP_EP_CONFIG_REGS;
    dev_ctx->lm_apb = config->global_config_base + APB_OFFSET_LM_REGS;
    dev_ctx->rc_axi_config_apb =
        config->global_config_base + APB_OFFSET_RC_AXI_CONFIG_REGS;
    dev_ctx->ep_axi_config_apb =
        config->global_config_base + APB_OFFSET_EP_AXI_CONFIG_REGS;

    return FWK_SUCCESS;
}

static int morello_pcie_bind(fwk_id_t id, unsigned int round)
{
    if ((round == 0) && fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return fwk_module_bind(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER),
            &pcie_ctx.timer_api);
    }

    return FWK_SUCCESS;
}

static int morello_pcie_start(fwk_id_t id)
{
    struct morello_pcie_dev_ctx *dev_ctx;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /*
         * Enable AP core to access PCIe root port's
         * configuration space
         */
        *(volatile uint32_t *)(NIC400_SOC_GPV_BASE + 0x0C) = 0x7F;
        return FWK_SUCCESS;
    }

    dev_ctx = &pcie_ctx.device_ctx_table[fwk_id_get_element_idx(id)];
    if (dev_ctx == NULL) {
        return FWK_E_PARAM;
    }

    /* Do not initialize PCIe RP in remote chip */
    if ((!dev_ctx->config->ccix_capable) && (morello_get_chipid() != 0)) {
        return FWK_SUCCESS;
    }

    return fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        FWK_ID_ELEMENT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_INTERCONNECT),
        id);
}

static int morello_pcie_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    int status = FWK_E_PARAM;

    switch (fwk_id_get_api_idx(api_id)) {
    case MORELLO_PCIE_API_IDX_PCIE_INIT:
        *api = &pcie_init_api;
        status = FWK_SUCCESS;
        break;
    case MORELLO_PCIE_API_IDX_CCIX_CONFIG:
        *api = &pcie_ccix_config_api;
        status = FWK_SUCCESS;
        break;
    default:
        break;
    }

    return status;
}

static int morello_pcie_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    return morello_pcie_setup(event->target_id);
}

const struct fwk_module module_morello_pcie = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = (unsigned int)MORELLO_PCIE_API_COUNT,
    .init = morello_pcie_init,
    .element_init = morello_pcie_element_init,
    .bind = morello_pcie_bind,
    .start = morello_pcie_start,
    .process_bind_request = morello_pcie_process_bind_request,
    .process_notification = morello_pcie_process_notification,
};
