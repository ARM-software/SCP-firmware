/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_thread.h>
#include <mod_juno_ppu.h>
#include <mod_juno_rom.h>
#include <mod_log.h>
#include <mod_power_domain.h>
#include <juno_nic400.h>
#include <juno_ppu_idx.h>
#include <juno_scc.h>
#include <juno_utils.h>
#include <scp_config.h>

static struct {
    const struct mod_juno_rom_config *config;
    const struct mod_juno_ppu_rom_api *ppu_api;
    struct mod_log_api *log_api;
    unsigned int notification_count;
} ctx;

/*
 * Static helpers
 */

static int deferred_setup(void)
{
    return FWK_SUCCESS;
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

    status = fwk_module_bind(fwk_module_id_log, MOD_LOG_API_ID, &ctx.log_api);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int juno_rom_start(fwk_id_t id)
{
    struct fwk_event event = {
        .source_id = fwk_module_id_juno_rom,
        .target_id = fwk_module_id_juno_rom,
        .id = mod_juno_rom_event_id_run,
    };

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

    fwk_assert((SCC->GPR1 & SCC_GPR1_BOOT_MAP_ENABLE) == 0);

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
            ctx.log_api->log(
                MOD_LOG_GROUP_ERROR,
                "[ROM] ERROR: Alternative AP ROM address does not have 4 byte "
                "alignment.\n");
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
