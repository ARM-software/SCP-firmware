/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_clock.h>
#include <mod_sp805.h>

#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_notification.h>

#define MOD_NAME "[SP805] "

/* SP805 Watchdog Timer device context */
struct mod_sp805_dev_ctx {
    const struct mod_sp805_config *config;
    struct sp805_reg *reg_base;
};

static struct mod_sp805_dev_ctx ctx;

static void sp805_isr(uintptr_t unused)
{
    FWK_LOG_DEBUG(MOD_NAME "SP805 watchdog timer interrupt generated.");

    /* Clear the watchdog Interrupt */
    ctx.reg_base->LOCK = ENABLE_WR_ACCESS;
    ctx.reg_base->INTCLR = 0;
    ctx.reg_base->LOCK = DISABLE_WR_ACCESS;

    fwk_interrupt_clear_pending(ctx.config->sp805_irq);
}

static void enable_sp805_interrupt(void)
{
    /*
     * Ensure that a valid interrupt number has been specified in the
     * configuration data. If not specified, the SP805 WDT is not enabled
     * because there is no other mechanism to write to the WdogIntClr register
     * when the counter value reaches zero.
     */
    if (ctx.config->sp805_irq == FWK_INTERRUPT_NONE) {
        FWK_LOG_ERR(MOD_NAME
                    "Interrupt number not specified in config data, failed to "
                    "enable watchdog!");
        return;
    }

    ctx.reg_base->LOCK = ENABLE_WR_ACCESS;
    ctx.reg_base->LOAD = ctx.config->wdt_load_value;

    /* Enable interrupt event and reset output */
    ctx.reg_base->CONTROL = RESET_EN | INT_EN;
    ctx.reg_base->LOCK = DISABLE_WR_ACCESS;
    fwk_interrupt_set_isr_param(
        ctx.config->sp805_irq, sp805_isr, (uintptr_t)NULL);

    fwk_interrupt_clear_pending(ctx.config->sp805_irq);
    fwk_interrupt_enable(ctx.config->sp805_irq);

    FWK_LOG_INFO(
        MOD_NAME "WatchDog Interrupt enabled, WdogLoad value: %x",
        (unsigned int)ctx.reg_base->LOAD);
}

/*
 * Framework handlers
 */
static int mod_sp805_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *data)
{
    ctx.config = (struct mod_sp805_config *)data;
    fwk_assert(ctx.config != NULL);

    ctx.reg_base = (struct sp805_reg *)ctx.config->reg_base;
    if (ctx.reg_base == NULL) {
        FWK_LOG_ERR(MOD_NAME "Register base address missing in config data");
        return FWK_E_DATA;
    }

    return FWK_SUCCESS;
}

static int mod_sp805_start(fwk_id_t id)
{
    if (!fwk_id_is_type(ctx.config->driver_id, FWK_ID_TYPE_NONE)) {
        /* Register for clock state notifications */
        return fwk_notification_subscribe(
            mod_clock_notification_id_state_changed, ctx.config->driver_id, id);
    } else {
        enable_sp805_interrupt();
    }

    return FWK_SUCCESS;
}

static int mod_sp805_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;

    fwk_assert(
        fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));

    params = (struct clock_notification_params *)event->params;
    if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
        enable_sp805_interrupt();
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_sp805 = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mod_sp805_init,
    .start = mod_sp805_start,
    .process_notification = mod_sp805_process_notification,
};
