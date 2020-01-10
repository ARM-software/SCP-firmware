/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_debug_rom.h"
#include "juno_nic400.h"
#include "juno_ppu_idx.h"
#include "juno_scc.h"
#include "juno_utils.h"
#include "juno_wdog_rom.h"
#include "scp_config.h"

#include <mod_bootloader.h>
#include <mod_juno_ppu.h>
#include <mod_juno_rom.h>
#include <mod_power_domain.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <string.h>

/* Values for cluster configuration */
#define CLUSTERCLK_CONTROL_CLKDIVSYS        UINT32_C(0x000000F0)
#define CLUSTERCLK_CONTROL_CLKDIVEXT        UINT32_C(0x00000F00)
#define CLUSTERCLK_CONTROL_CRNTCLKDIVSYS    UINT32_C(0x000F0000)
#define CLUSTERCLK_CONTROL_CRNTCLKDIVEXT    UINT32_C(0x00F00000)
#define CLUSTERCLK_CONTROL_CLKSEL           UINT32_C(0x0000000F)
#define CLUSTERCLK_CONTROL_CRNTCLK          UINT32_C(0x0000F000)
#define CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK   UINT32_C(0x00000004)

/* Cluster clock fields */
#define CLK_DIVIDER_SYS_POS         4
#define CLK_DIVIDER_EXT_POS         8
#define CRNT_CLK_DIV_POS            12
#define CRNT_CLK_DIV_SYS_POS        16
#define CRNT_CLK_DIV_EXT_POS        20

static struct {
    const struct mod_juno_rom_config *config;
    const struct mod_juno_ppu_rom_api *ppu_api;
    struct mod_bootloader_api *bootloader_api;
    unsigned int notification_count;
    unsigned int boot_map_little;
    unsigned int boot_map_big;
} ctx;

static const fwk_id_t little_cluster_ppu =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU, JUNO_PPU_DEV_IDX_LITTLE_SSTOP);

static const fwk_id_t big_cluster_ppu =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU, JUNO_PPU_DEV_IDX_BIG_SSTOP);

static const fwk_id_t core_ppu_table_little[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU, JUNO_PPU_DEV_IDX_LITTLE_CPU0),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU, JUNO_PPU_DEV_IDX_LITTLE_CPU1),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU, JUNO_PPU_DEV_IDX_LITTLE_CPU2),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU, JUNO_PPU_DEV_IDX_LITTLE_CPU3),
};

static const fwk_id_t core_ppu_table_big[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU, JUNO_PPU_DEV_IDX_BIG_CPU0),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU, JUNO_PPU_DEV_IDX_BIG_CPU1),
};

/*
 * Static helpers
 */

static int power_cluster_and_cores_from_boot_map(
    unsigned int boot_map,
    fwk_id_t cluster_ppu,
    const fwk_id_t *core_ppu_table,
    unsigned int core_ppu_table_size)
{
    int status;
    unsigned int i;
    volatile uint32_t *snoop_ctrl;

    fwk_assert(core_ppu_table != NULL);

    if (boot_map == 0)
        return FWK_SUCCESS;

    /* Turn on the cluster */
    status = ctx.ppu_api->set_state_and_wait(cluster_ppu, MOD_PD_STATE_ON);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    /* Assign the specified snoop for this cluster */
    if (fwk_id_get_element_idx(cluster_ppu) ==
        fwk_id_get_element_idx(little_cluster_ppu))
        snoop_ctrl = &SCP_CONFIG->LITTLE_SNOOP_CONTROL;
    else if (fwk_id_get_element_idx(cluster_ppu) ==
        fwk_id_get_element_idx(big_cluster_ppu))
        snoop_ctrl = &SCP_CONFIG->BIG_SNOOP_CONTROL;
    else
        return FWK_E_PANIC;

    juno_utils_open_snoop_gate_and_wait(snoop_ctrl);

    /*
     * Turn on the cores using its boot map. (If bit N is set in the boot map,
     * core N must be powered on)
     */
    for (i = 0; i < core_ppu_table_size; ++i) {
        if (boot_map & (1 << i)) {
            status = ctx.ppu_api->set_state_and_wait(
                core_ppu_table[i],
                MOD_PD_STATE_ON);

            if (status != FWK_SUCCESS)
                return FWK_E_DEVICE;
        }
    }

    return FWK_SUCCESS;
}

static void css_clock_cluster_div_set(volatile uint32_t *clk,
                                      uint32_t sys_divider,
                                      uint32_t ext_divider,
                                      bool wait)
{
    uint32_t div_set;
    uint32_t div_set_mask;
    uint32_t div_check;
    uint32_t div_check_mask;

    div_set = ((sys_divider - 1) << CLK_DIVIDER_SYS_POS) |
              ((ext_divider - 1) << CLK_DIVIDER_EXT_POS);
    div_set_mask =
        (CLUSTERCLK_CONTROL_CLKDIVSYS | CLUSTERCLK_CONTROL_CLKDIVEXT);

    div_check = ((sys_divider - 1) << CRNT_CLK_DIV_SYS_POS) |
                ((ext_divider - 1) << CRNT_CLK_DIV_EXT_POS);
    div_check_mask =
        (CLUSTERCLK_CONTROL_CRNTCLKDIVSYS | CLUSTERCLK_CONTROL_CRNTCLKDIVEXT);

    *clk = (*clk & ~(div_set_mask)) | div_set;

    if (wait) {
        while ((*clk & div_check_mask) != div_check)
            continue;
    }
}

static void css_clock_cluster_sel_set(volatile uint32_t *clk,
                                      uint32_t source,
                                      bool wait)
{
    *clk = (*clk & ~CLUSTERCLK_CONTROL_CLKSEL) | source;

    if (wait) {
        while ((*clk & CLUSTERCLK_CONTROL_CRNTCLK) !=
            (source << CRNT_CLK_DIV_POS))
            continue;
    }
}

static int deferred_setup(void)
{
    int status;

    /* Turn on the required cluster(s) and core(s) using the boot maps */
    status = power_cluster_and_cores_from_boot_map(
        ctx.boot_map_little,
        little_cluster_ppu,
        core_ppu_table_little,
        FWK_ARRAY_SIZE(core_ppu_table_little));

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[ROM] ERROR: Failed to turn on LITTLE cluster.");
        return FWK_E_DEVICE;
    }

    status = power_cluster_and_cores_from_boot_map(
        ctx.boot_map_big,
        big_cluster_ppu,
        core_ppu_table_big,
        FWK_ARRAY_SIZE(core_ppu_table_big));

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[ROM] ERROR: Failed to turn on big cluster.");
        return FWK_E_DEVICE;
    }

    if (ctx.boot_map_little) {
        /* Switch clock source to the private PLL */
        css_clock_cluster_div_set(&SCP_CONFIG->LITTLECLK_CONTROL, 1, 1, true);
        css_clock_cluster_sel_set(&SCP_CONFIG->LITTLECLK_CONTROL,
                                  CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK, true);
    }

    if (ctx.boot_map_big) {
        /* Switch clock source to the private PLL */
        css_clock_cluster_div_set(&SCP_CONFIG->BIGCLK_CONTROL, 1, 1, true);
        css_clock_cluster_sel_set(&SCP_CONFIG->BIGCLK_CONTROL,
                                  CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK, true);
    }

    #ifndef BUILD_MODE_DEBUG
    juno_wdog_rom_reload();
    #endif

    status = ctx.bootloader_api->load_image();

    FWK_LOG_ERR("[ROM] ERROR: Failed to load RAM firmware image: %d", status);

    return FWK_E_DATA;
}

/*
 * Framework API
 */

static int juno_rom_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (!fwk_expect(data != NULL))
        return FWK_E_PARAM;

    /* Enable all clocks */
    SCP_CONFIG->CLOCK_ENABLE_SET = SCP_CONFIG_CLOCK_ENABLE_ALL;

    juno_utils_smc_init();

    ctx.config = data;

    return FWK_SUCCESS;
}

static int juno_rom_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round != 0)
        return FWK_SUCCESS;

    status = fwk_module_bind(fwk_module_id_juno_ppu,
        mod_juno_ppu_api_id_rom, &ctx.ppu_api);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_PANIC;

    status = fwk_module_bind(
        fwk_module_id_bootloader,
        FWK_ID_API(FWK_MODULE_IDX_BOOTLOADER, 0),
        &ctx.bootloader_api);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int juno_rom_start(fwk_id_t id)
{
    int status;

    struct fwk_event event = {
        .source_id = fwk_module_id_juno_rom,
        .target_id = fwk_module_id_juno_rom,
        .id = mod_juno_rom_event_id_run,
    };

    #ifndef BUILD_MODE_DEBUG
    juno_wdog_rom_halt_on_debug_config();
    juno_wdog_rom_enable();
    #endif

    status = juno_debug_rom_init(ctx.ppu_api);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_thread_put_event(&event);
}

static int juno_rom_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    int status;
    struct fwk_event systop_on_event;
    struct mod_pd_power_state_transition_notification_params
        *notification_params;

    /* Configure the boot maps to power on LITTLE_CPU0 by default */
    ctx.boot_map_little = 1;
    ctx.boot_map_big = 0;

    /* If the SCC provides boot maps, use them instead */
    if (SCC->GPR1 & SCC_GPR1_BOOT_MAP_ENABLE) {
        ctx.boot_map_little = (SCC->GPR1 & SCC_GPR1_BOOT_MAP_LITTLE) >>
            SCC_GPR1_BOOT_MAP_LITTLE_POS;
        ctx.boot_map_big = (SCC->GPR1 & SCC_GPR1_BOOT_MAP_BIG) >>
            SCC_GPR1_BOOT_MAP_BIG_POS;
    }

    /* Set cryptographic extensions state */
    if (SCC->GPR1 & SCC_GPR1_CRYPTO_DISABLE) {
        SCP_CONFIG->BIG_STATIC_CONFIG |=
            SCP_CONFIG_BIG_STATIC_CONFIG_CRYPTODISABLE;
        SCP_CONFIG->LITTLE_STATIC_CONFIG |=
            SCP_CONFIG_LITTLE_STATIC_CONFIG_CRYPTODISABLE;
    }

    /* Set endianness configuration */
    if (SCC->GPR1 & SCC_GPR1_CFGEE) {
        SCP_CONFIG->BIG_STATIC_CONFIG |= SCP_CONFIG_BIG_STATIC_CONFIG_CFGEND;
        SCP_CONFIG->LITTLE_STATIC_CONFIG |=
            SCP_CONFIG_LITTLE_STATIC_CONFIG_CFGEND;
    }

    /* Set default exception handling state */
    if (SCC->GPR1 & SCC_GPR1_CFGTE) {
        SCP_CONFIG->BIG_STATIC_CONFIG |= SCP_CONFIG_BIG_STATIC_CONFIG_CFGTE;
        SCP_CONFIG->LITTLE_STATIC_CONFIG |=
            SCP_CONFIG_LITTLE_STATIC_CONFIG_CFGTE;
    }

    /* Set NIC400 ThinLinks interface state */
    if (SCC->NIC400_TLX & SCC_TLX_MST_ENABLE)
        NIC400->REMAP |= NIC400_REMAP_TLX_EN;

    /* Set alternative AP ROM address (if applicable) */
    if (SCC->APP_ALT_BOOT != 0) {
        if ((SCC->APP_ALT_BOOT & 0x3) != 0) {
            FWK_LOG_ERR(
                "[ROM] ERROR: Alternative AP ROM address does not have 4 byte "
                "alignment");
            return FWK_E_ALIGN;
        }

        SCP_CONFIG->RESET_VEC_BASE_ADDR =
            (SCC->APP_ALT_BOOT & SCC_APP_ALT_BOOT_ADDR);
    }

    /* Disable barrier requests on the ACE IF */
    SCP_CONFIG->BIG_STATIC_CONFIG |=
        SCP_CONFIG_BIG_STATIC_CONFIG_BARRIERDISABLE;
    SCP_CONFIG->LITTLE_STATIC_CONFIG |=
        SCP_CONFIG_LITTLE_STATIC_CONFIG_BARRIERDISABLE;

    /* Zero the AP context area */
    memset((void *)ctx.config->ap_context_base, 0, ctx.config->ap_context_size);

    /* Send SYSTOP ON notification */
    systop_on_event = (struct fwk_event) {
        .response_requested = true,
        .id = mod_juno_rom_notification_id_systop,
        .source_id = FWK_ID_NONE
    };

    notification_params = (void *)systop_on_event.params;
    notification_params->state = MOD_PD_STATE_ON;

    status = fwk_notification_notify(&systop_on_event, &ctx.notification_count);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_PANIC;

    if (ctx.notification_count == 0)
        return deferred_setup();

    return FWK_SUCCESS;
}

static int juno_rom_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    fwk_assert(fwk_id_is_equal(event->id, mod_juno_rom_notification_id_systop));
    fwk_assert(event->is_response == true);

    /* At least one notification response must be outstanding */
    if (!fwk_expect(ctx.notification_count > 0))
        return FWK_E_PANIC;

    /* Complete remaining setup now that all subscribers have responded */
    if ((--ctx.notification_count) == 0)
        return deferred_setup();

    return FWK_SUCCESS;
}

const struct fwk_module module_juno_rom = {
    .name = "Juno ROM",
    .type = FWK_MODULE_TYPE_SERVICE,
    .event_count = MOD_JUNO_ROM_EVENT_COUNT,
    .notification_count = MOD_JUNO_ROM_NOTIFICATION_COUNT,
    .init = juno_rom_init,
    .bind = juno_rom_bind,
    .start = juno_rom_start,
    .process_event = juno_rom_process_event,
    .process_notification = juno_rom_process_notification,
};
