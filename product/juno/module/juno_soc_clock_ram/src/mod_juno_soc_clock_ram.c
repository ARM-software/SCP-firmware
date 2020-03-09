/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_scc.h"
#include "juno_soc_clock_ram_pll.h"
#include "scp_config.h"
#include "system_clock.h"

#include <mod_clock.h>
#include <mod_juno_soc_clock_ram.h>
#include <mod_power_domain.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * When changing CLUSTER_MIN_FREQ, make sure it is a frequency we can easily
 * derive from SYSCLK (1600 * FWK_MHZ).
 */
#define CLUSTER_MIN_FREQ      (400 * FWK_MHZ)

/* HW Errata */
#define PCIEACLK_DEFAULT_FREQ (400 * FWK_MHZ)
#define HDLCDCLK_DEFAULT_FREQ (400 * FWK_MHZ)

#define JUNO_SOC_CLOCK_WAIT_TIMEOUT_US  10000

static const fwk_id_t juno_soc_clock_timer_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0);

/* Clock context */
struct juno_soc_clock_dev_ctx {
    const struct mod_juno_soc_clock_ram_dev_config *config;
    unsigned int current_rate_index;
    enum mod_clock_state state;
    unsigned int pd_state;

    /*
     * We don't rely on the bootloader leaving the clocks in any specific state.
     */
    bool rate_initialized;
};

struct juno_soc_clock_ctx {
    const struct mod_juno_soc_clock_ram_config *config;
    const struct mod_pd_restricted_api *pd_api;
    const struct mod_timer_api *timer_api;
    unsigned int debug_pd_state;
};

static struct juno_soc_clock_ctx module_ctx;
static struct juno_soc_clock_dev_ctx *ctx_table;

/*
 * Static helper functions
 */

struct check_clock_reg_data {
    FWK_RW uint32_t *reg;
    uint32_t mask;
    uint32_t val;
};

static bool check_clock_reg(void *data)
{
    struct check_clock_reg_data *params;

    fwk_assert(data != NULL);
    params = (struct check_clock_reg_data*)data;

    return ((*(params->reg) & params->mask) == params->val);
}


static int get_rate_entry(struct juno_soc_clock_dev_ctx *ctx,
                          uint64_t target_rate,
                          const struct juno_soc_clock_ram_rate **entry)
{
    unsigned int i;

    fwk_assert(ctx != NULL);
    fwk_assert(entry != NULL);

    /* Find the entry matching the requested rate */
    for (i = 0; i < ctx->config->rate_count; i++) {
        if (ctx->config->rate_table[i].rate == target_rate) {
            *entry = (struct juno_soc_clock_ram_rate *)
                    &ctx->config->rate_table[i];
            return FWK_SUCCESS;
        }
    }
    return FWK_E_PARAM;
}

void cluster_clock_divsys_set(FWK_RW uint32_t *clk,
                              uint32_t sys_divider,
                              bool wait)
{
    struct check_clock_reg_data params;
    uint32_t clkdivsys;
    uint32_t crntclkdivsys;
    int status;

    fwk_assert(clk != NULL);
    fwk_assert(sys_divider != 0);
    fwk_assert(sys_divider <= 16);

    clkdivsys = sys_divider - 1;

    *clk = (*clk & ~SCP_CONFIG_CLUSTERCLK_CONTROL_CLKDIVSYS_MASK) |
           (clkdivsys << SCP_CONFIG_CLUSTERCLK_CONTROL_CLKDIVSYS_POS);
    if (wait) {
        crntclkdivsys = clkdivsys <<
            SCP_CONFIG_CLUSTERCLK_CONTROL_CRNTCLKDIVSYS_POS;
        params.reg = clk;
        params.mask = SCP_CONFIG_CLUSTERCLK_CONTROL_CRNTCLKDIVSYS_MASK;
        params.val = crntclkdivsys;
        status = module_ctx.timer_api->wait(
            juno_soc_clock_timer_id,
            JUNO_SOC_CLOCK_WAIT_TIMEOUT_US,
            check_clock_reg, &params);
        fwk_assert(status == FWK_SUCCESS);
    }
}

void cluster_clock_divext_set(volatile uint32_t *clk,
                              uint32_t ext_divider,
                              bool wait)
{
    struct check_clock_reg_data params;
    uint32_t clkdivext;
    uint32_t crntclkdivext;
    int status;

    fwk_assert(clk != NULL);
    fwk_assert(ext_divider != 0);
    fwk_assert(ext_divider <= 16);

    clkdivext = ext_divider - 1;

    *clk = (*clk & ~SCP_CONFIG_CLUSTERCLK_CONTROL_CLKDIVEXT_MASK) |
           (clkdivext << SCP_CONFIG_CLUSTERCLK_CONTROL_CLKDIVEXT_POS);
    if (wait) {
        crntclkdivext = clkdivext <<
            SCP_CONFIG_CLUSTERCLK_CONTROL_CRNTCLKDIVEXT_POS;
        params.reg = clk;
        params.mask = SCP_CONFIG_CLUSTERCLK_CONTROL_CRNTCLKDIVEXT_MASK;
        params.val = crntclkdivext;
        status = module_ctx.timer_api->wait(
            juno_soc_clock_timer_id,
            JUNO_SOC_CLOCK_WAIT_TIMEOUT_US,
            check_clock_reg, &params);
        fwk_assert(status == FWK_SUCCESS);
    }
}

void cluster_clock_sel_set(volatile uint32_t *clk,
                           uint32_t source,
                           bool wait)
{
    struct check_clock_reg_data params;
    uint32_t crntclk;
    int status;

    fwk_assert(clk != NULL);

    *clk = (*clk & ~SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_MASK) |
           (source << SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_POS);
    if (wait) {
        crntclk = source << SCP_CONFIG_CLUSTERCLK_CONTROL_CRNTCLK_POS;
        params.reg = clk;
        params.mask = SCP_CONFIG_CLUSTERCLK_CONTROL_CRNTCLK_MASK;
        params.val = crntclk;
        status = module_ctx.timer_api->wait(
            juno_soc_clock_timer_id,
            JUNO_SOC_CLOCK_WAIT_TIMEOUT_US,
            check_clock_reg, &params);
        fwk_assert(status == FWK_SUCCESS);
    }
}

void clock_sel_set(volatile uint32_t *clk, uint32_t source)
{
    struct check_clock_reg_data params;
    int status;

    fwk_assert(clk != NULL);
    fwk_assert(source <= 2);

    *clk = (*clk & ~SCP_CONFIG_STDCLK_CONTROL_CLKSEL_MASK) |
           (source << SCP_CONFIG_STDCLK_CONTROL_CLKSEL_POS);
    params.reg = clk;
    params.mask = SCP_CONFIG_STDCLK_CONTROL_CRNTCLK_MASK;
    params.val = source << SCP_CONFIG_STDCLK_CONTROL_CRNTCLK_POS;
    status = module_ctx.timer_api->wait(
        juno_soc_clock_timer_id,
        JUNO_SOC_CLOCK_WAIT_TIMEOUT_US,
        check_clock_reg, &params);
    fwk_assert(status == FWK_SUCCESS);
}

void clock_div_set(volatile uint32_t *clk, uint32_t divider)
{
    struct check_clock_reg_data params;
    int status;

    fwk_assert(clk != NULL);
    fwk_assert(divider != 0);
    fwk_assert(divider <= 16);

    *clk = (*clk & ~SCP_CONFIG_STDCLK_CONTROL_CLKDIV_MASK) |
        ((divider - 1) << SCP_CONFIG_STDCLK_CONTROL_CLKDIV_POS);
    params.reg = clk;
    params.mask = SCP_CONFIG_STDCLK_CONTROL_CRNTCLKDIV_MASK;
    params.val = (divider - 1) << SCP_CONFIG_STDCLK_CONTROL_CRNTCLKDIV_POS;
    status = module_ctx.timer_api->wait(
        juno_soc_clock_timer_id,
        JUNO_SOC_CLOCK_WAIT_TIMEOUT_US,
        check_clock_reg, &params);
    fwk_assert(status == FWK_SUCCESS);
}

static void init_juno_soc_clock(void)
{
    /*
     * There should be no need to reset the clock dividers after waking since
     * these registers reside in the AON power domain. However the clock
     * sources may have been changed back to SYSREFCLK and therefore will
     * need to be updated.
     */

    /* CSS Internal Clocks */
    clock_sel_set(&SCP_CONFIG->CCICLK_CONTROL,
                  SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    clock_sel_set(&SCP_CONFIG->NICSCPCLK_CONTROL,
                  SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    clock_sel_set(&SCP_CONFIG->NICPERCLK_CONTROL,
                  SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    clock_sel_set(&SCP_CONFIG->SPCLK_CONTROL,
                  SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    clock_sel_set(&SCP_CONFIG->GICCLK_CONTROL,
                  SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);

    /* Debug Clocks */
    if (module_ctx.debug_pd_state == MOD_PD_STATE_ON) {
        clock_sel_set(&SCP_CONFIG->ATCLK_CONTROL,
                      SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
        clock_sel_set(&SCP_CONFIG->TRACECLKIN_CONTROL,
                      SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    }

    /* SoC Clocks */
    clock_sel_set(&SCC->FAXICLK, SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    clock_sel_set(&SCC->SAXICLK, SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    clock_sel_set(&SCC->HDLCDCLK, SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);

    if ((SCC->GPR0 & SCC_GPR0_SKIP_TLX_CLK_SETTING) == 0) {
        clock_sel_set(&SCC->TMIF2XCLK,
                      SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
        clock_sel_set(&SCC->TSIF2XCLK,
                      SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    }

    clock_sel_set(&SCC->USBHCLK, SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    clock_sel_set(&SCC->PCIEACLK, SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    clock_sel_set(&SCC->PCIETLCLK, SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    clock_sel_set(&SCC->PXLCLK, SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
}

static int set_rate(struct juno_soc_clock_dev_ctx *ctx,
                    const struct juno_soc_clock_ram_rate *rate_entry)
{
    int status;
    const struct mod_juno_soc_clock_ram_dev_config *config;
    unsigned int current_source;
    bool wait;

    fwk_assert(ctx != NULL);
    fwk_assert(rate_entry != NULL);

    config = ctx->config;
    current_source = *config->clock_control &
                   SCP_CONFIG_CLUSTERCLK_CONTROL_CRNTCLK_MASK;

    wait = (ctx->pd_state == MOD_PD_STATE_ON);

    switch (rate_entry->source) {

    /* Private PLL */
    case SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK:
        /*
         * If the current clock is already using the PRIVCLK, we need to change
         * the clock source to something else so we can change the private PLL.
         */
        if (current_source == SCP_CONFIG_CLUSTERCLK_CONTROL_CRNTCLK_PRIVCLK) {
            cluster_clock_divsys_set(config->clock_control,
                                     SYSINCLK / CLUSTER_MIN_FREQ,
                                     wait);
            cluster_clock_sel_set(config->clock_control,
                                  SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_SYSINCLK,
                                  wait);
        }

        cluster_clock_divext_set(config->clock_control,
                                 rate_entry->divider,
                                 wait);

        status = juno_soc_clock_ram_pll_set(config->pll,
                                            &rate_entry->pll_setting);
        fwk_assert(status == FWK_SUCCESS);

        cluster_clock_sel_set(config->clock_control,
                              SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_PRIVCLK,
                              wait);
        return FWK_SUCCESS;

    /* SYSPLL */
    case SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_SYSINCLK:

    /* SYSREFCLK */
    case SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_SYSREFCLK:
        cluster_clock_divsys_set(config->clock_control,
                                 rate_entry->divider,
                                 wait);
        cluster_clock_sel_set(config->clock_control, rate_entry->source, wait);

        return FWK_SUCCESS;

    default:
        return FWK_E_SUPPORT;
    }
}

/*
 * Clock driver functions
 */

static int juno_soc_clock_set_rate(fwk_id_t clock_id,
                                   uint64_t rate,
                                   enum mod_clock_round_mode round_mode)
{
    int status;
    struct juno_soc_clock_dev_ctx *ctx;
    const struct juno_soc_clock_ram_rate *rate_entry;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    if (ctx->state == MOD_CLOCK_STATE_STOPPED)
        return FWK_E_STATE;

    /*
     * Look up the divider and source settings. We do not perform any rounding
     * on the clock rate given as input which has to be precise in order not to
     * be refused with an FWK_E_PARAM error code.
     */
    status = get_rate_entry(ctx, rate, &rate_entry);
    if (status != FWK_SUCCESS)
        return FWK_E_PARAM;

    status = set_rate(ctx, rate_entry);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    ctx->current_rate_index = rate_entry - ctx->config->rate_table;

    ctx->rate_initialized = true;

    return FWK_SUCCESS;
}

static int juno_soc_clock_get_rate(fwk_id_t clock_id,
                                   uint64_t *rate)
{
    struct juno_soc_clock_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    if (!ctx->rate_initialized)
        *rate = 0;
    else
        *rate = ctx->config->rate_table[ctx->current_rate_index].rate;

    return FWK_SUCCESS;
}

static int juno_soc_clock_get_rate_from_index(fwk_id_t clock_id,
                                              unsigned int rate_index,
                                              uint64_t *rate)
{
    struct juno_soc_clock_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    if (rate_index >= ctx->config->rate_count)
        return FWK_E_PARAM;

    *rate = ctx->config->rate_table[rate_index].rate;

    return FWK_SUCCESS;
}

static int juno_soc_clock_set_state(fwk_id_t clock_id,
                                    enum mod_clock_state state)
{
    int status;
    struct juno_soc_clock_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    if (ctx->state == state)
        return FWK_SUCCESS;

    if (state == MOD_CLOCK_STATE_RUNNING) {
        /*
         * The state is changing to RUNNING, enable the clock by setting
         * it to the last known rate.
         */
        if (ctx->rate_initialized) {
            status = set_rate(ctx,
                 &ctx->config->rate_table[ctx->current_rate_index]);
            if (status != FWK_SUCCESS)
                return FWK_E_DEVICE;
        }
    } else {
        cluster_clock_sel_set(ctx->config->clock_control,
                              SCP_CONFIG_CLUSTERCLK_CONTROL_CLKSEL_GATED, true);
    }

    ctx->state = state;

    return FWK_SUCCESS;
}

static int juno_soc_clock_get_state(fwk_id_t clock_id,
                                    enum mod_clock_state *state)
{
    struct juno_soc_clock_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    *state = ctx->state;

    return FWK_SUCCESS;
}

static int juno_soc_clock_get_range(fwk_id_t clock_id,
                                    struct mod_clock_range *range)
{
    struct juno_soc_clock_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    range->rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE;
    range->min = ctx->config->rate_table[0].rate;
    range->max = ctx->config->rate_table[ctx->config->rate_count - 1].rate;
    range->rate_count = ctx->config->rate_count;

    return FWK_SUCCESS;
}

/*
 * Notification handler invoked after the state of a clock's power domain
 * has changed.
 */
static int juno_soc_clock_process_power_transition(fwk_id_t clock_id,
                                                   unsigned int state)
{
    struct juno_soc_clock_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(clock_id)];

    ctx->pd_state = state;

    return FWK_SUCCESS;
}

static const struct mod_clock_drv_api api_clock = {
    .set_rate = juno_soc_clock_set_rate,
    .get_rate = juno_soc_clock_get_rate,
    .get_rate_from_index = juno_soc_clock_get_rate_from_index,
    .set_state = juno_soc_clock_set_state,
    .get_state = juno_soc_clock_get_state,
    .get_range = juno_soc_clock_get_range,
    .process_power_transition = juno_soc_clock_process_power_transition,
};

/*
 * Framework handler functions
 */

static int juno_soc_clock_init(fwk_id_t module_id,
                               unsigned int element_count,
                               const void *data)
{
    fwk_expect(element_count > 0);

    ctx_table = fwk_mm_calloc(element_count,
        sizeof(struct juno_soc_clock_dev_ctx));

    module_ctx.config = data;

    return FWK_SUCCESS;
}

static int juno_soc_clock_element_init(fwk_id_t element_id,
                                       unsigned int sub_element_count,
                                       const void *data)
{
    unsigned int rate_index = 0;
    uint64_t rate;
    uint64_t last_rate = 0;
    struct juno_soc_clock_dev_ctx *ctx;
    const struct mod_juno_soc_clock_ram_dev_config *config = data;

    ctx = &ctx_table[fwk_id_get_element_idx(element_id)];

    /* Verify that the rate entries in the lookup table are ordered */
    for (rate_index = 0; rate_index < config->rate_count; rate_index++) {
        rate = config->rate_table[rate_index].rate;

        /* The rate entries must be in ascending order */
        if (rate < last_rate)
            return FWK_E_DATA;

        last_rate = rate;
    }

    ctx->config = config;
    ctx->state = MOD_CLOCK_STATE_RUNNING;

    return FWK_SUCCESS;
}

static int juno_soc_clock_bind(fwk_id_t id, unsigned int round)
{
    int status;

    /*
     * Only bind in first round of calls
     */
    if (round > 0)
        return FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    /* Bind to the timer component */
    status = fwk_module_bind(juno_soc_clock_timer_id,
        MOD_TIMER_API_ID_TIMER, &module_ctx.timer_api);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int juno_soc_clock_start(fwk_id_t id)
{
    int status;

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    /* HW Errata: Overwrite default clock */
    clock_div_set(&SCC->PCIEACLK, SYSINCLK / PCIEACLK_DEFAULT_FREQ);

    /* HW Errata: Disable dynamic clock gating */
    SCC->SYSTEM_CLK_FORCE |= SCC_SYSTEM_CLK_FORCE_FAXICLK;

    /* Switch pixel clock source to PLL */
    clock_sel_set(&SCC->PXLCLK, SCC_PXLCLK_CLKSEL_PLL);

    /* HW Errata: Disable dynamic clock gating */
    SCC->SYSTEM_CLK_FORCE |= SCC_SYSTEM_CLK_FORCE_HDLCDCLK;

    /* HW Errata: Overwrite default clock */
    clock_div_set(&SCC->HDLCDCLK, SYSINCLK / HDLCDCLK_DEFAULT_FREQ);

    /* Subscribe to the systop power domain transition */
    status = fwk_notification_subscribe(
        mod_pd_notification_id_power_state_transition,
        module_ctx.config->systop_pd_id,
        id);

    if (status != FWK_SUCCESS)
        return status;

    /* Subscribe to the debug power domain transition */
    return fwk_notification_subscribe(
        mod_pd_notification_id_power_state_transition,
        module_ctx.config->debug_pd_id,
        id);
}

static int juno_soc_clock_process_bind_request(fwk_id_t source_id,
                                               fwk_id_t target_id,
                                               fwk_id_t api_id,
                                               const void **api)
{
    if (fwk_id_is_equal(api_id, mod_juno_soc_clock_ram_api_id_driver)) {
        *api = &api_clock;

        return FWK_SUCCESS;
    }

    return FWK_E_PARAM;
}

static int process_debug_notification(const struct fwk_event *event,
                                               struct fwk_event *resp_event) {
    struct mod_pd_power_state_transition_notification_params *params =
               (struct mod_pd_power_state_transition_notification_params *)
               event->params;

    /*
     * On Juno, for Debug Power Domain, only MOD_PD_STATE_ON is accepted as
     * argument for the set_state command, so we will never receive
     * notifications for MOD_PD_STATE_OFF.
     */
    fwk_assert(params->state != MOD_PD_STATE_OFF);

    module_ctx.debug_pd_state = MOD_PD_STATE_ON;

    return FWK_SUCCESS;
}

static int process_systop_notification(const struct fwk_event *event,
                                               struct fwk_event *resp_event) {
    struct mod_pd_power_state_transition_notification_params *params =
               (struct mod_pd_power_state_transition_notification_params *)
               event->params;

    if (params->state == MOD_PD_STATE_ON) {
        juno_soc_clock_ram_pll_init();
        init_juno_soc_clock();
    }

    return FWK_SUCCESS;

}

static int juno_soc_clock_process_notification(const struct fwk_event *event,
                                               struct fwk_event *resp_event)
{
    if (fwk_id_is_equal(event->source_id, module_ctx.config->debug_pd_id))
        return process_debug_notification(event, resp_event);
    else
        return process_systop_notification(event, resp_event);
}

const struct fwk_module module_juno_soc_clock_ram = {
    .name = "Juno SoC Clock Driver",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = juno_soc_clock_init,
    .element_init = juno_soc_clock_element_init,
    .start = juno_soc_clock_start,
    .bind = juno_soc_clock_bind,
    .process_bind_request = juno_soc_clock_process_bind_request,
    .process_notification = juno_soc_clock_process_notification,
};
