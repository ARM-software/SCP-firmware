/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "juno_id.h"
#include "juno_pcie.h"
#include "juno_scc.h"
#include "juno_utils.h"
#include "juno_wdog_ram.h"
#include "nic400_gpv.h"
#include "pl35x.h"
#include "xpressrich3.h"

#include <mod_juno_ram.h>
#include <mod_power_domain.h>
#include <mod_system_power.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_math.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TIMEOUT_RESET_PCIE_US       (1000 * 1000)
#define SMC_INIT_CMD_SET_CS5        UINT32_C(5 << 23)
#define SMC_INIT_CMD_UPDATE_REG     UINT32_C(2 << 21)
#define PCSM_TRICKLE_DELAY          24
#define PCSM_HAMMER_DELAY           9
#define PCSM_RAM_DELAY              37
#define PCIE_ATR_IMPL               UINT32_C(0x00000001)

static struct {
    /* Module configuration */
    const struct mod_juno_ram_config *config;

    /* Timer module API */
    const struct mod_timer_api *timer_api;

    /* Running platform */
    enum juno_idx_platform platform;

    /* Whether power domain-sensitive peripherals are initialized */
    bool periph_initialized;
} ctx;

static const fwk_id_t pd_source_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, POWER_DOMAIN_IDX_SYSTOP);

/*
 * Utility Functions
 */
static int pcie_add_translation_table_entry(
    struct xpressrich3_addr_trsl_reg *entry,
    uint64_t source_addr,
    uint64_t translated_addr,
    uint64_t size,
    uint64_t translation_parameter)
{
    uint32_t log2_upper_bound;

    fwk_assert(size != 0);
    log2_upper_bound = fwk_math_log2(size);

    /*
     * If 'size' is not a power of two, get the smallest power of two greater
     * than 'size'.
     */
    if ((size & (size - 1)) != 0)
        log2_upper_bound++;

    if (!fwk_expect((log2_upper_bound >= 12) && (log2_upper_bound <= 64)))
        return FWK_E_PANIC;

    entry->SRC_ADDR_LO = (uint32_t)source_addr |
                         (((log2_upper_bound - 1) << 1) | PCIE_ATR_IMPL);
    entry->SRC_ADDR_UP = source_addr >> 32;

    entry->TRSL_ADDR_LO = (uint32_t)translated_addr;
    entry->TRSL_ADDR_UP = translated_addr >> 32;

    entry->TRSL_PARAM = translation_parameter;

    return FWK_SUCCESS;
}

static bool pcie_check_reset_completed(void *data)
{
    return ((PCIE->RESET_STATUS & PCIE_CONTROL_RESET_STATUS_RCPHYPLL_OUT) ==
            PCIE_CONTROL_RESET_STATUS_RCPHYPLL_OUT);
}

static bool pcie_check_link_up(void *data)
{
    return ((PCIE_ROOT->CS.PCIE_BASIC_STATUS &
             PCIE_BASIC_STATUS_NEG_LINK_WIDTH_MASK) != 0);
}

static void smc_configure(void)
{
    uint32_t smc_cycles = 0;
    uint32_t smc_opmode = 0;

    /* SMC: Switch CS5 from 32 bits to 16 bits mode. */
    smc_cycles = SMC->CS[5].CYCLES;
    smc_opmode = SMC->CS[5].OPMODE;
    smc_opmode = (smc_opmode & 0xFFFC) | 0x01;
    SMC->SET_CYCLES = smc_cycles;
    SMC->SET_OPMODE = smc_opmode;
    SMC->DIRECT_CMD = SMC_INIT_CMD_SET_CS5 | SMC_INIT_CMD_UPDATE_REG;

    juno_utils_smc_init();
}

static void nic400_configure(void)
{
    /* Fabric QoS settings */
    NIC400_GPV_HDLCD0->QOS_READ_PRIORITY = 0xC;
    NIC400_GPV_HDLCD1->QOS_READ_PRIORITY = 0xC;
    NIC400_GPV_CCI_PCIE->QOS_READ_PRIORITY = 0x4;
    NIC400_GPV_CCI_PCIE->QOS_WRITE_PRIORITY = 0x4;
    NIC400_GPV_CCI_GPU->QOS_READ_PRIORITY = 0x8;
    NIC400_GPV_CCI_GPU->QOS_WRITE_PRIORITY = 0x8;
    NIC400_GPV_CCI_LITTLE->QOS_READ_PRIORITY = 0xB;
    NIC400_GPV_CCI_LITTLE->QOS_WRITE_PRIORITY = 0xB;
    NIC400_GPV_CCI_BIG->QOS_READ_PRIORITY = 0xB;
    NIC400_GPV_CCI_BIG->QOS_WRITE_PRIORITY = 0xB;
}

static void pcsm_configure(void)
{
    unsigned int i;

    /*
     * PCSM delay timings
     * Expressed as a number of 50MHz REFCLK ticks.
     * Note: The PCSM waits for the programmed number of ticks + 1.
     */
    for (i = 0; i < PCSM_IDX_COUNT; i++) {
        SCC->PCSM[i].TRICKLE_DELAY = PCSM_TRICKLE_DELAY;
        SCC->PCSM[i].HAMMER_DELAY = PCSM_HAMMER_DELAY;
        SCC->PCSM[i].RAM_DELAY = PCSM_RAM_DELAY;
    }
}

static void scc_configure(void)
{
    /* Ensure all SoC devices are released from reset */
    SCC->VSYS_MANUAL_RESET = 0;
    /* Configure DMA as non-secure */
    SCC->DMA_CONTROL0 = 0x0007FFFE;
    /* Configure SMC Masks: CS6/CS7 are not used */
    SCC->SMC_MASK[3] = 0xFF00FF00;
}

static int pcie_configure(void)
{
    int status;
    enum juno_idx_revision revision_id = JUNO_IDX_REVISION_COUNT;

    status = juno_id_get_revision(&revision_id);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    if ((revision_id != JUNO_IDX_REVISION_R0) &&
        ((SCC->GPR0 & SCC_GPR0_PCIE_AP_MANAGED) == 0)) {

        /*
         * Enable IO window, prefetchable memory window and 64-bit
         * addressing support for the prefetchable memory window.
         */
        PCIE_ROOT->CS.PCIE_BAR_WIN = (PCIE_BAR_WIN_IO_WIN_ENABLE |
                                      PCIE_BAR_WIN_PREFETCH_WIN_ENABLE |
                                      PCIE_BAR_WIN_PREFETCH_WIN64_ENABLE);

        /* Set up PCI-PCI bridge configuration register */
        PCIE_ROOT->CS.PCIE_PCI_IDS_1 =
            ((PCI_BRIDGE_CLASS << 8) | PCI_BRIDGE_REVISION_ID);

        /* Assert values before translation */
        fwk_assert(GICV2M_MSI_SIZE > 0);
        fwk_assert(JUNO_SYSTEM_MEMORY_SIZE > 0);
        fwk_assert(JUNO_EXTRA_SYSTEM_MEMORY_SIZE > 0);
        fwk_assert(PCI_ECAM_SIZE > 0);
        fwk_assert(PCI_IO_SIZE > 0);
        fwk_assert(PCI_MEM32_SIZE > 0);
        fwk_assert(PCI_MEM64_SIZE > 0);

        /* PCIE Window 0 - Translation: MSI support */
        status = pcie_add_translation_table_entry(
            &PCIE_ROOT->ATR_PCIE_WIN0[0],
            GICV2M_MSI_BASE,
            GICV2M_MSI_BASE,
            GICV2M_MSI_SIZE,
            PCIE_TRSL_PARAM_AXIDEVICE);
        if (status != FWK_SUCCESS)
            return status;

        /* PCIE Window 0 - Translation: System memory support */
        status = pcie_add_translation_table_entry(
            &PCIE_ROOT->ATR_PCIE_WIN0[1],
            JUNO_SYSTEM_MEMORY_BASE,
            JUNO_SYSTEM_MEMORY_BASE,
            JUNO_SYSTEM_MEMORY_SIZE,
            PCIE_TRSL_PARAM_AXIMEMORY);
        if (status != FWK_SUCCESS)
            return status;

        status = pcie_add_translation_table_entry(
            &PCIE_ROOT->ATR_PCIE_WIN0[2],
            JUNO_EXTRA_SYSTEM_MEMORY_BASE,
            JUNO_EXTRA_SYSTEM_MEMORY_BASE,
            JUNO_EXTRA_SYSTEM_MEMORY_SIZE,
            PCIE_TRSL_PARAM_AXIMEMORY);
        if (status != FWK_SUCCESS)
            return status;

        /* AXI4 Slave - Translation: PCI ECAM support */
        status = pcie_add_translation_table_entry(
            &PCIE_ROOT->ATR_AXI4_SLV0[1],
            PCI_ECAM_BASE,
            PCI_ECAM_BASE,
            PCI_ECAM_SIZE,
            PCIE_TRSL_PARAM_PCIE_CONF);
        if (status != FWK_SUCCESS)
            return status;

        /* AXI4 Slave - Translation: PCI IO support */
        status = pcie_add_translation_table_entry(
            &PCIE_ROOT->ATR_AXI4_SLV0[2],
            PCI_IO_BASE,
            PCI_IO_BASE,
            PCI_IO_SIZE,
            PCIE_TRSL_PARAM_PCIE_IO);
        if (status != FWK_SUCCESS)
            return status;

        /* AXI4 Slave - Translation: PCI MEM32 support */
        status = pcie_add_translation_table_entry(
            &PCIE_ROOT->ATR_AXI4_SLV0[3],
            PCI_MEM32_BASE,
            PCI_MEM32_BASE,
            PCI_MEM32_SIZE,
            PCIE_TRSL_PARAM_PCIE_MEMORY);
        if (status != FWK_SUCCESS)
            return status;

        /* AXI4 Slave - Translation: PCI MEM64 support */
        status = pcie_add_translation_table_entry(
            &PCIE_ROOT->ATR_AXI4_SLV0[4],
            PCI_MEM64_BASE,
            PCI_MEM64_BASE,
            PCI_MEM64_SIZE,
            PCIE_TRSL_PARAM_PCIE_MEMORY);
        if (status != FWK_SUCCESS)
            return status;

        /* Add credits */
        PCIE_ROOT->CS.PCIE_VC_CRED_0 = 0x00F0B818;
        PCIE_ROOT->CS.PCIE_VC_CRED_1 = 0x00000001;

        /* Enable ECRC support */
        PCIE_ROOT->CS.PCIE_PEX_SPC2 = 0x00006006;

        /* Request and wait for reset completion */
        PCIE->RESET_CONTROL = PCIE_CONTROL_RESET_CONTROL_RCPHY_REL;
        status = ctx.timer_api->wait(
            ctx.config->timer_id,
            TIMEOUT_RESET_PCIE_US,
            pcie_check_reset_completed,
            NULL);
        if (!fwk_expect(status == FWK_SUCCESS))
            return FWK_E_PANIC;

        /* Wait for the link to be up */
        status = ctx.timer_api->wait(
            ctx.config->timer_id,
            TIMEOUT_RESET_PCIE_US,
            pcie_check_link_up,
            NULL);
        if (!fwk_expect(status == FWK_SUCCESS))
            return FWK_E_PANIC;

        /* Set up interrupt mask - Application Processor */
        PCIE_ROOT->IE.IMASK_LOCAL = (PCIE_INT_LINES | PCIE_INT_MSI);
    }

    return FWK_SUCCESS;
}

static int initialize_peripherals(void)
{
    int status = FWK_SUCCESS;

    if (ctx.periph_initialized)
        return status;

    if (ctx.platform == JUNO_IDX_PLATFORM_RTL)
        status = pcie_configure();

    smc_configure();

    ctx.periph_initialized = true;

    return status;
}

/*
 * Framework API
 */
static int juno_ram_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    fwk_assert(element_count == 0);

    ctx.config = data;

    return FWK_SUCCESS;
}

static int juno_ram_bind(fwk_id_t id, unsigned int round)
{
    if (round != 0)
        return FWK_SUCCESS;

    return fwk_module_bind(ctx.config->timer_id, MOD_TIMER_API_ID_TIMER,
        &ctx.timer_api);
}

static int juno_ram_start(fwk_id_t id)
{
    int status;

    /*
     * The watchdog should not be enabled when the RAM firmware is executing.
     * If the ROM firmware ran in release mode then the watchdog will have been
     * left running and it must be disabled.
     */
    #ifndef BUILD_MODE_DEBUG
    juno_wdog_ram_disable();
    #endif

    fwk_assert(fwk_module_is_valid_module_id(id));

    status = juno_id_get_platform(&ctx.platform);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_PANIC;

    if (ctx.platform == JUNO_IDX_PLATFORM_RTL) {
        nic400_configure();

        pcsm_configure();
    }

    scc_configure();

    /* Register for Power Domain state transition notifications */
    status = fwk_notification_subscribe(
        mod_pd_notification_id_power_state_transition,
        pd_source_id,
        id);
    if (status != FWK_SUCCESS)
        return status;

    return initialize_peripherals();
}

static int juno_ram_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct mod_pd_power_state_transition_notification_params *pd_params;
    int status = FWK_SUCCESS;

    if (fwk_id_is_equal(event->id,
                        mod_pd_notification_id_power_state_transition)) {
        pd_params =
            (struct mod_pd_power_state_transition_notification_params *)
                event->params;

        if ((pd_params->state == MOD_PD_STATE_OFF) ||
            (pd_params->state == MOD_SYSTEM_POWER_POWER_STATE_SLEEP0)) {
            /* PD-sensitive peripherals will require re-init */
            ctx.periph_initialized = false;
        } else if (pd_params->state == MOD_PD_STATE_ON)
            status = initialize_peripherals();

    } else
        status = FWK_E_PARAM;

    return status;
}

const struct fwk_module module_juno_ram = {
    .name = "Juno RAM",
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = juno_ram_init,
    .bind = juno_ram_bind,
    .start = juno_ram_start,
    .process_notification = juno_ram_process_notification,
};
