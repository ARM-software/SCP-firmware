/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP System Support.
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <fmw_cmsis.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <mod_clock.h>
#include <mod_n1sdp_flash.h>
#include <mod_n1sdp_system.h>
#include <mod_log.h>
#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
#include <mod_system_power.h>
#include <n1sdp_core.h>
#include <n1sdp_scp_pik.h>
#include <n1sdp_scp_irq.h>
#include <n1sdp_scp_mmap.h>
#include <n1sdp_ssc.h>
#include <config_clock.h>

/* Coresight counter register definitions */
struct cs_cnt_ctrl_reg {
    FWK_RW uint32_t CS_CNTCR;
    FWK_R  uint32_t CS_CNTSR;
    FWK_RW uint32_t CS_CNTCVLW;
    FWK_RW uint32_t CS_CNTCVUP;
};

#define CS_CNTCONTROL ((struct cs_cnt_ctrl_reg *)SCP_CS_CNTCONTROL_BASE)

/* Module context */
struct n1sdp_system_ctx {

    /* Log API pointer */
    const struct mod_log_api *log_api;

    /* Pointer to the Interrupt Service Routine API of the PPU_V1 module */
    const struct ppu_v1_isr_api *ppu_v1_isr_api;

    /* Power domain module restricted API pointer */
    struct mod_pd_restricted_api *mod_pd_restricted_api;

    /* Pointer to N1SDP Flash APIs */
    const struct mod_n1sdp_flash_api *flash_api;
};

struct n1sdp_system_isr {
    unsigned int interrupt;
    void (*handler)(void);
};

static struct n1sdp_system_ctx n1sdp_system_ctx;
const struct fwk_module_config config_n1sdp_system = { 0 };

/*
 *  PPU Interrupt Service Routines for cluster and core power domains
 */

static void ppu_cores_isr(void)
{
    uint32_t status = PIK_SCP->CPU_PPU_INT_STATUS[0];
    unsigned int core_idx;

    while (status != 0) {
        core_idx = __builtin_ctz(status);
        status &= ~(1 << core_idx);

        if (core_idx >= n1sdp_core_get_core_count())
            continue;

        n1sdp_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1, core_idx));
    }
}

static void ppu_clusters_isr(void)
{
    uint32_t status = PIK_SCP->CLUS_PPU_INT_STATUS;
    unsigned int cluster_idx;

    while (status != 0) {
        cluster_idx = __builtin_ctz(status);

        n1sdp_system_ctx.ppu_v1_isr_api->ppu_interrupt_handler(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1,
            n1sdp_core_get_core_count() + cluster_idx));

        status &= ~(1 << cluster_idx);
    }
}

/*
 *  PPU Interrupt Service Routine table
 */

static struct n1sdp_system_isr isrs[] = {
    [0] = { .interrupt = PPU_CORES0_IRQ,
            .handler = ppu_cores_isr },
    [1] = { .interrupt = PPU_CLUSTERS_IRQ,
            .handler = ppu_clusters_isr },
};

/*
 * System power module's driver API
 */

static int n1sdp_system_shutdown(
    enum mod_pd_system_shutdown system_shutdown)
{
    NVIC_SystemReset();

    return FWK_E_DEVICE;
}

static const struct mod_system_power_driver_api
    n1sdp_system_power_driver_api = {
    .system_shutdown = n1sdp_system_shutdown,
};

/*
 * AP memory 1MB windowed access driver API
 */

/*
 * Function to set the AP address for which the SCP 1MB window maps to.
 * dram_1mb_window_address should be 1MB aligned with no trailing zeros
 * which means that the 1MB aligned base address should be right shifted by
 * 20 bits before passing to this function. Bit 0 of ADDR_TRANS register
 * is enable bit so the dram_1mb_window_address will be shifted left by
 * one bit before programming by this function.
 */
static void n1sdp_system_enable_ap_memory_access(
    uint32_t dram_1mb_window_address)
{
    uint32_t addr_trans_reg = 0;

    addr_trans_reg = (dram_1mb_window_address << 1);
    addr_trans_reg |= ADDR_TRANS_EN;
    PIK_SCP->ADDR_TRANS = addr_trans_reg;
}

/*
 * Function to disable 1MB address translation window.
 */
static void n1sdp_system_disable_ap_memory_access(void)
{
    PIK_SCP->ADDR_TRANS &= ~ADDR_TRANS_EN;
}

struct mod_n1sdp_system_ap_memory_access_api
    n1sdp_system_ap_memory_access_api = {
        .enable_ap_memory_access = n1sdp_system_enable_ap_memory_access,
        .disable_ap_memory_access = n1sdp_system_disable_ap_memory_access,
};

/*
 * Function to copy into AP SRAM.
 */
static int n1sdp_system_copy_to_ap_sram(uint64_t sram_address,
                                        uint32_t spi_address,
                                        uint32_t size)
{
    uint32_t target_addr = (uint32_t)sram_address;

    memcpy((void *)target_addr, (void *)spi_address, size);

    if (memcmp((void *)target_addr, (void *)spi_address, size) != 0) {
        n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
            "[N1SDP SYSTEM] Copy failed at destination address: 0x%08x\n",
            target_addr);
            return FWK_E_DATA;
    }
    n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[N1SDP SYSTEM] Copied binary to SRAM address: 0x%08x\n",
        sram_address);
    return FWK_SUCCESS;
}

/*
 * Function to copy into DRAM location
 */
static int n1sdp_system_copy_to_ap_ddr(uint64_t dram_address,
                                       uint32_t spi_address,
                                       uint32_t size)
{
    uint32_t scp_address = 0;
    uint32_t copy_size = 0;
    uint32_t addr_offset = 0;
    int status = FWK_SUCCESS;

    while (size != 0) {
        addr_offset = (uint32_t)(dram_address & SCP_AP_1MB_WINDOW_ADDR_MASK);

        /* Map 1MB window to corresponding address in AP memory map. */
        n1sdp_system_enable_ap_memory_access(
            dram_address >> SCP_AP_1MB_WINDOW_ADDR_SHIFT);

        /* Get destination for this copy in SCP address space. */
        scp_address = SCP_AP_1MB_WINDOW_BASE | addr_offset;

        /* Get the size for this copy operation. */
        if (size > (SCP_AP_1MB_WINDOW_SIZE - addr_offset)) {
            /*
             * If the copy operation will wrap around the end of the
             * 1MB window we need to cut it off at the wrap around
             * point and change the window address.
             */
            copy_size = (uint32_t)(SCP_AP_1MB_WINDOW_SIZE - addr_offset);
        } else {
            /* All remaining data is within the current 1MB window. */
            copy_size = size;
        }

        /* Copy the data into the selected 1MB window. */
        memcpy((void *)scp_address, (void *)spi_address, copy_size);

        if (memcmp((void *)spi_address, (void *)scp_address, copy_size) != 0) {
            n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
                "[N1SDP SYSTEM] Copy failed at destination address: 0x%08x\n",
                scp_address);
            status = FWK_E_DATA;
            goto exit;
        }

        /* Update variables before starting over. */
        dram_address = dram_address + copy_size;
        spi_address = spi_address + copy_size;
        size = size - copy_size;
    }

exit:
    /* Disable the 1MB window. */
    n1sdp_system_disable_ap_memory_access();

    return status;
}

void cdbg_pwrupreq_handler(void)
{
    n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[N1SDP SYSTEM] Received debug power up request interrupt\n");

    n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[N1SDP SYSTEM] Power on Debug PIK\n");

    /* Clear interrupt */
    PIK_DEBUG->DEBUG_CTRL |= (0x1 << 1);
    fwk_interrupt_disable(CDBG_PWR_UP_REQ_IRQ);
}

void csys_pwrupreq_handler(void)
{
    n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
        "[N1SDP SYSTEM] Received system power up request interrupt\n");

    /* Clear interrupt */
    PIK_DEBUG->DEBUG_CTRL |= (0x1 << 2);
    fwk_interrupt_disable(CSYS_PWR_UP_REQ_IRQ);
}

/*
 * Functions fulfilling the framework's module interface
 */

static int n1sdp_system_mod_init(fwk_id_t module_id, unsigned int unused,
    const void *unused2)
{
    int status;
    unsigned int idx;
    struct n1sdp_system_isr *isr;

    for (idx = 0; idx < FWK_ARRAY_SIZE(isrs); idx++) {
        isr = &isrs[idx];
        status = fwk_interrupt_set_isr(isr->interrupt, isr->handler);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int n1sdp_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 0) {
        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_LOG),
            FWK_ID_API(FWK_MODULE_IDX_LOG, 0), &n1sdp_system_ctx.log_api);
        if (status != FWK_SUCCESS)
            return status;
    }

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_FLASH),
                             FWK_ID_API(FWK_MODULE_IDX_N1SDP_FLASH, 0),
                             &n1sdp_system_ctx.flash_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_POWER_DOMAIN),
        FWK_ID_API(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_RESTRICTED),
        &n1sdp_system_ctx.mod_pd_restricted_api);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_PPU_V1),
        FWK_ID_API(FWK_MODULE_IDX_PPU_V1, MOD_PPU_V1_API_IDX_ISR),
        &n1sdp_system_ctx.ppu_v1_isr_api);
}

static int n1sdp_system_process_bind_request(fwk_id_t requester_id,
    fwk_id_t pd_id, fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_N1SDP_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER:
        *api = &n1sdp_system_power_driver_api;
        break;
    case MOD_N1SDP_SYSTEM_API_IDX_AP_MEMORY_ACCESS:
        *api = &n1sdp_system_ap_memory_access_api;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int n1sdp_system_start(fwk_id_t id)
{
    int status;

    status = fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        FWK_ID_ELEMENT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_INTERCONNECT),
        id);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_interrupt_set_isr(CDBG_PWR_UP_REQ_IRQ, cdbg_pwrupreq_handler);
    if (status == FWK_SUCCESS) {
        fwk_interrupt_enable(CDBG_PWR_UP_REQ_IRQ);

        status = fwk_interrupt_set_isr(CSYS_PWR_UP_REQ_IRQ,
            csys_pwrupreq_handler);
        if (status == FWK_SUCCESS) {
            fwk_interrupt_enable(CSYS_PWR_UP_REQ_IRQ);

            PIK_CLUSTER(0)->CLKFORCE_SET = 0x00000004;
            PIK_CLUSTER(1)->CLKFORCE_SET = 0x00000004;

            /* Enable debugger access in SSC */
            SSC->SSC_DBGCFG_SET = 0x000000FF;

            /* Setup CoreSight counter */
            CS_CNTCONTROL->CS_CNTCR |= (1 << 0);
            CS_CNTCONTROL->CS_CNTCVLW = 0x00000000;
            CS_CNTCONTROL->CS_CNTCVUP = 0x0000FFFF;
        } else
            n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_DEBUG,
                "[N1SDP SYSTEM] CSYS PWR UP REQ IRQ register failed\n");
    } else
        n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_DEBUG,
            "[N1SDP SYSTEM] CDBG PWR UP REQ IRQ register failed\n");

    n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_DEBUG,
        "[N1SDP SYSTEM] Requesting SYSTOP initialization...\n");

    return n1sdp_system_ctx.mod_pd_restricted_api->set_composite_state_async(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0), false,
            MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON,
                                   MOD_PD_STATE_OFF, MOD_PD_STATE_OFF));
}

static int n1sdp_system_process_notification(const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params = NULL;
    struct mod_pd_restricted_api *mod_pd_restricted_api = NULL;
    struct mod_n1sdp_fip_descriptor *fip_desc_table = NULL;
    unsigned int fip_count;
    unsigned int i;
    unsigned int core_idx;
    unsigned int cluster_idx;
    unsigned int cluster_count;
    int fip_index_bl31 = -1;
    int fip_index_bl33 = -1;
    int status;

    assert(fwk_id_is_equal(event->id,
                           mod_clock_notification_id_state_changed));
    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));

    params = (struct clock_notification_params *)event->params;

    /*
     * Initialize primary core when the system is initialized for the
     * first time only.
     */
    if (params->new_state == MOD_CLOCK_STATE_RUNNING) {

        n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
            "[N1SDP SYSTEM] Looking for AP firmware in flash memory...\n");

        status = n1sdp_system_ctx.flash_api->get_n1sdp_fip_descriptor_count(
            FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_SYSTEM), &fip_count);
        if (status != FWK_SUCCESS)
            return status;

        status = n1sdp_system_ctx.flash_api->get_n1sdp_fip_descriptor_table(
            FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_SYSTEM), &fip_desc_table);
        if (status != FWK_SUCCESS)
            return status;

        for (i = 0; i < fip_count; i++) {
            if (fip_desc_table[i].type == MOD_N1SDP_FIP_TYPE_TF_BL31) {
                n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
                    "[N1SDP SYSTEM] Found BL31 at address: 0x%08x,"
                    " size: %u, flags: 0x%x\n",
                    fip_desc_table[i].address, fip_desc_table[i].size,
                    fip_desc_table[i].flags);
                fip_index_bl31 = i;
            } else if (fip_desc_table[i].type == MOD_N1SDP_FIP_TYPE_NS_BL33) {
                n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
                    "[N1SDP SYSTEM] Found BL33 at address: 0x%08x,"
                    " size: %u, flags: 0x%x\n",
                    fip_desc_table[i].address, fip_desc_table[i].size,
                    fip_desc_table[i].flags);
                fip_index_bl33 = i;
            }
        }

        if ((fip_index_bl31 < 0) || (fip_index_bl33 < 0)) {
            n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
                "[N1SDP SYSTEM] Error! "
                "FIP does not have all required binaries\n");
            return FWK_E_PANIC;
        }

        n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
            "[N1SDP SYSTEM] Copying AP BL31 to address 0x%x...\n",
            AP_CORE_RESET_ADDR);

        status = n1sdp_system_copy_to_ap_sram(AP_CORE_RESET_ADDR,
                     fip_desc_table[fip_index_bl31].address,
                     fip_desc_table[fip_index_bl31].size);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;

        n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
            "[N1SDP SYSTEM] Copying AP BL33 to address 0x%x...\n",
            AP_BL33_BASE_ADDR);

        status = n1sdp_system_copy_to_ap_ddr(AP_BL33_BASE_ADDR,
                     fip_desc_table[fip_index_bl33].address,
                     fip_desc_table[fip_index_bl33].size);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;

        mod_pd_restricted_api = n1sdp_system_ctx.mod_pd_restricted_api;

        n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_INFO,
            "[N1SDP SYSTEM] Setting AP Reset Address to 0x%x\n",
            AP_CORE_RESET_ADDR);

        cluster_count = n1sdp_core_get_cluster_count();
        for (cluster_idx = 0; cluster_idx < cluster_count; cluster_idx++) {
            for (core_idx = 0;
                core_idx < n1sdp_core_get_core_per_cluster_count(cluster_idx);
                core_idx++) {
                PIK_CLUSTER(cluster_idx)->STATIC_CONFIG[core_idx].RVBARADDR_LW
                    = (uint32_t)(AP_CORE_RESET_ADDR - AP_SCP_SRAM_OFFSET);
                PIK_CLUSTER(cluster_idx)->STATIC_CONFIG[core_idx].RVBARADDR_UP
                    = (uint32_t)
                    ((AP_CORE_RESET_ADDR - AP_SCP_SRAM_OFFSET) >> 32);
            }
        }

        n1sdp_system_ctx.log_api->log(MOD_LOG_GROUP_DEBUG,
            "[N1SDP SYSTEM] Switching ON primary core...\n");

        status = mod_pd_restricted_api->set_composite_state_async(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0),
            false,
            MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON,
                MOD_PD_STATE_ON, MOD_PD_STATE_ON));
        if (status != FWK_SUCCESS)
            return status;

        /*
         * Unsubscribe to interconnect clock state change notification as
         * it has to be processed only once during system startup.
         */
        return fwk_notification_unsubscribe(event->id, event->source_id,
                                            event->target_id);
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_n1sdp_system = {
    .name = "N1SDP_SYSTEM",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_N1SDP_SYSTEM_API_COUNT,
    .init = n1sdp_system_mod_init,
    .bind = n1sdp_system_bind,
    .process_bind_request = n1sdp_system_process_bind_request,
    .process_notification = n1sdp_system_process_notification,
    .start = n1sdp_system_start,
};
