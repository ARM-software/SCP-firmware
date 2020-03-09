/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno Debug driver
 */

#include <coresight_soc400.h>
#include <juno_irq.h>
#include <juno_ssc.h>
#include <juno_utils.h>
#include <scp_config.h>
#include <system_clock.h>
#include <system_mmap.h>

#include <mod_debug.h>
#include <mod_juno_debug.h>
#include <mod_power_domain.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_interrupt.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_multi_thread.h>
#include <fwk_status.h>

#define CS_CNTCONTROL       ((struct cs_cnt_ctrl_reg *) CS_CNTCONTROL_BASE)
#define CNTCONTROL_CR_HDBG  UINT32_C(0x00000002)
#define CNTCONTROL_CR_EN    UINT32_C(0x00000001)

#define TIME_OUT_ATCLK_DIV_SET      (100 * 1000)
#define TIME_OUT_ATCLK_SEL_SET      (100 * 1000)
#define TIME_OUT_TRACECLK_DIV_SET   (100 * 1000)
#define TIME_OUT_TRACECLK_SEL_SET   (100 * 1000)
#define TIME_OUT_PCLKDBG_DIV_SET    (100 * 1000)
#define TIME_OUT_CORESIGHT_DE_ASSERT_REQ (1000 * 1000)

static const fwk_id_t mod_juno_debug_api_id_driver =
    FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_DEBUG, MOD_JUNO_DEBUG_API_IDX_DRIVER);

/* Coresight counter register definitions */
struct cs_cnt_ctrl_reg {
    FWK_RW uint32_t CS_CNTCR;
};

static struct mod_timer_api *timer_api;

enum juno_debug_state {
    IDLE,
    GET_STATE,
    SET_STATE_DBGSYS,
    SET_STATE_BIG_SSTOP,
    SET_STATE_LITTLE_SSTOP,
};

enum juno_debug_event_idx {
    JUNO_DEBUG_EVENT_IDX_SUBMIT_REQUEST,
    JUNO_DEBUG_EVENT_IDX_COUNT,
};

static const fwk_id_t juno_debug_event_submit_request =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_JUNO_DEBUG,
                      JUNO_DEBUG_EVENT_IDX_SUBMIT_REQUEST);

struct juno_debug_dev_ctx {
    const struct mod_pd_restricted_api *pd_api;
    const struct mod_debug_driver_input_api *driver_input_api;
    fwk_id_t debug_hal_id;
    uint32_t cookie;
    enum juno_debug_state state;
    /* Keep track of the state of the DEBUG power domain */
    bool pd_dbg_on;
    enum scp_debug_user user_id;
};

static struct juno_debug_dev_ctx dev_ctx;

static const uint32_t debug_clk_mask = (SCP_CONFIG_CLOCK_ENABLE_PCLKDBGEN |
                                        SCP_CONFIG_CLOCK_ENABLE_TRACECLKINEN |
                                        SCP_CONFIG_CLOCK_ENABLE_ATCLKEN);

/*
 * Helpers functions
 */

static bool atclk_clock_div_set_check(void *data)
{
    return juno_utils_atclk_clock_div_set_check(*(uint32_t *) data);
}

static bool atclk_clock_sel_set_check(void *data)
{
    return juno_utils_atclk_clock_sel_set_check(*(uint32_t *)data);
}

static bool traceclk_clock_div_set_check(void *data)
{
    return juno_utils_traceclk_clock_div_set_check(*(uint32_t *)data);
}

static bool traceclk_clock_sel_set_check(void *data)
{
    return juno_utils_traceclk_clock_sel_set_check(*(uint32_t *)data);
}

static bool pclkdbg_clock_div_set_check(void *data)
{
    return juno_utils_pclkdbg_clock_div_set_check(*(uint32_t *)data);
}

static void set_ack_debug_pwrup_req(void)
{
    SCP_CONFIG->DEBUG_CONTROL |= SCP_CONFIG_DEBUG_CONTROL_CDBGPWRUPACK;

    fwk_interrupt_enable(CDBG_PWR_UP_REQ_IRQ);
}

static void clear_ack_debug_pwrup_req(void)
{
    SCP_CONFIG->DEBUG_CONTROL &= ~SCP_CONFIG_DEBUG_CONTROL_CDBGPWRUPACK;

    fwk_interrupt_enable(CDBG_PWR_UP_REQ_IRQ);
}

/*
 * Interrupt handlers
 */

static void juno_debug_cdbg_pwr_up_req_isr(void)
{
    int status;
    const struct mod_debug_driver_input_api *driver_input_api;
    bool enable;

    driver_input_api = dev_ctx.driver_input_api;

    fwk_interrupt_disable(CDBG_PWR_UP_REQ_IRQ);

    enable = SCP_CONFIG->DEBUG_STATUS & SCP_CONFIG_DEBUG_STATUS_CDBGPWRUPREQ;
    status = driver_input_api->enable(dev_ctx.debug_hal_id,
                                      enable, SCP_DEBUG_USER_DAP);
    fwk_assert(status == FWK_SUCCESS);
}

static void juno_debug_csys_pwr_up_req_isr(void)
{
    if (SCP_CONFIG->DEBUG_STATUS & SCP_CONFIG_DEBUG_STATUS_CSYSPWRUPREQ)
        SCP_CONFIG->DEBUG_CONTROL |= SCP_CONFIG_DEBUG_CONTROL_CSYSPWRUPACK;
    else
        SCP_CONFIG->DEBUG_CONTROL &= ~SCP_CONFIG_DEBUG_CONTROL_CSYSPWRUPACK;
}

static bool coresight_de_assert_req_check(void *data)
{
    return ((SCP_CONFIG->DEBUG_STATUS & SCP_CONFIG_DEBUG_STATUS_CDBGRSTREQ) ==
        SCP_CONFIG_DEBUG_STATUS_CDBGRSTREQ);
}

static void juno_debug_cdbg_rst_req_isr(void)
{
    int status;
    const struct mod_juno_debug_config *module_config =
        fwk_module_get_data(fwk_module_id_juno_debug);

    /* Reset debug system */
    SCP_CONFIG->SYS_MANUAL_RESET.SET = SCP_CONFIG_SYS_MANUAL_RESET_DBGSYSRESET;

    /* Acknowledge request */
    SCP_CONFIG->DEBUG_CONTROL |= SCP_CONFIG_DEBUG_CONTROL_CDBGRSTACK;

    /* Wait for CoreSight to de-assert request */
    status = timer_api->wait(module_config->timer_id,
                             TIME_OUT_CORESIGHT_DE_ASSERT_REQ,
                             coresight_de_assert_req_check,
                             NULL);

    /* The condition wasn't met before the end of the timeout */
    if (status != FWK_SUCCESS)
        return;

    /* De-assert reset */
    SCP_CONFIG->SYS_MANUAL_RESET.SET = SCP_CONFIG_SYS_MANUAL_RESET_DBGSYSRESET;

    /* De-assert ACK */
    SCP_CONFIG->DEBUG_CONTROL &= ~SCP_CONFIG_DEBUG_CONTROL_CDBGRSTACK;
}

static int apply_clock_settings(struct juno_css_debug_dev *juno_css_debug_dev)
{
    int status;
    uint32_t source = SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK;
    const struct mod_juno_debug_config *module_config =
        fwk_module_get_data(fwk_module_id_juno_debug);

    juno_utils_atclk_clock_div_set(juno_css_debug_dev->div_atclk);
    status = timer_api->wait(module_config->timer_id,
                             TIME_OUT_ATCLK_DIV_SET,
                             atclk_clock_div_set_check,
                             (void *)(&juno_css_debug_dev->div_atclk));
    if (status != FWK_SUCCESS)
        return status;

    juno_utils_atclk_clock_sel_set(source);
    status = timer_api->wait(module_config->timer_id,
                             TIME_OUT_ATCLK_SEL_SET,
                             atclk_clock_sel_set_check,
                             (void *)&source);
    if (status != FWK_SUCCESS)
        return status;

    juno_utils_traceclk_clock_div_set(juno_css_debug_dev->div_traceclk);
    status = timer_api->wait(module_config->timer_id,
                             TIME_OUT_TRACECLK_DIV_SET,
                             traceclk_clock_div_set_check,
                             (void *)&juno_css_debug_dev->div_traceclk);
    if (status != FWK_SUCCESS)
        return status;

    juno_utils_traceclk_clock_sel_set(source);
    status = timer_api->wait(module_config->timer_id,
                             TIME_OUT_TRACECLK_SEL_SET,
                             traceclk_clock_sel_set_check,
                             (void *)&source);
    if (status != FWK_SUCCESS)
        return status;

    juno_utils_pclkdbg_clock_div_set(juno_css_debug_dev->div_pclk);
    return timer_api->wait(module_config->timer_id,
                           TIME_OUT_PCLKDBG_DIV_SET,
                           pclkdbg_clock_div_set_check,
                           (void *)&juno_css_debug_dev->div_pclk);
}

static int turn_on_pd(fwk_id_t pd_id, enum juno_debug_state next_state)
{
    int status;

    status = dev_ctx.pd_api->set_state(pd_id, MOD_PD_STATE_ON);
    if (status == FWK_PENDING)
        dev_ctx.state = next_state;

    return status;
}

static int enable_debug(fwk_id_t id, enum scp_debug_user user_id)
{
    int status;
    struct fwk_event req;

    if (dev_ctx.state != IDLE)
        return FWK_E_BUSY;

    dev_ctx.user_id = user_id;

    if (!dev_ctx.pd_dbg_on) {
        req = (struct fwk_event) {
            .target_id = id,
            .id = juno_debug_event_submit_request,
        };

        status = fwk_thread_put_event(&req);
        if (status != FWK_SUCCESS)
            return status;

        return FWK_PENDING;
    }

    /*
     * This request originated from an IRQ, therefore acknowledgment is
     * required and the ISR needs to be re-enabled.
     */
    if (user_id == SCP_DEBUG_USER_DAP)
        set_ack_debug_pwrup_req();

    return FWK_SUCCESS;
}

static int disable_debug(fwk_id_t id, enum scp_debug_user user_id)
{
    /*
     * This request originated from an IRQ, therefore acknowledgment is
     * required and the ISR needs to be re-enabled.
     */
    if (user_id == SCP_DEBUG_USER_DAP)
        clear_ack_debug_pwrup_req();

    return FWK_SUCCESS;
}

/*
 * Module APIs
 */

static int set_enabled(fwk_id_t id, bool enable, enum scp_debug_user user_id)
{
    if (dev_ctx.state != IDLE)
        return FWK_E_BUSY;

    return enable ? enable_debug(id, user_id) : disable_debug(id, user_id);
}

static int get_enabled(fwk_id_t id, bool *enabled, enum scp_debug_user user_id)
{
    if (dev_ctx.state != IDLE)
        return FWK_E_BUSY;

    *enabled = dev_ctx.pd_dbg_on;

    return FWK_SUCCESS;
}

static struct mod_debug_driver_api juno_debug_drv_api = {
    .set_enabled = set_enabled,
    .get_enabled = get_enabled,
};

static int process_enable_request(fwk_id_t id)
{
    int status;
    unsigned int power_state;
    struct juno_css_debug_dev *juno_css_debug_dev;
    const struct mod_juno_debug_dev_config *config;

    if (dev_ctx.state != IDLE)
        return FWK_E_BUSY;

    config = fwk_module_get_data(id);
    juno_css_debug_dev = config->clk_settings;

    /* Check if DBGSYS has already been powered-on */
    status = dev_ctx.pd_api->get_state(config->pd_dbgsys_id, &power_state);
    if (status != FWK_SUCCESS) {
        if (status == FWK_PENDING)
            dev_ctx.state = GET_STATE;
        return status;
    }

    if (power_state != MOD_PD_STATE_ON) {
        juno_utils_system_clock_enable(debug_clk_mask);

        /* Turn on DBGSYS */
        status = turn_on_pd(config->pd_dbgsys_id, SET_STATE_DBGSYS);
        if (status != FWK_SUCCESS)
            return status;

        /* Turn on BIG_SSTOP */
        status = turn_on_pd(config->pd_big_sstop_id, SET_STATE_BIG_SSTOP);
        if (status != FWK_SUCCESS)
            return status;

        /* Turn on LITTLE_SSTOP */
        status = turn_on_pd(config->pd_little_sstop_id,
                            SET_STATE_LITTLE_SSTOP);
        if (status != FWK_SUCCESS)
            return status;

        /*
         * Apply clock settings
         */
        status = apply_clock_settings(juno_css_debug_dev);
        if (status != FWK_SUCCESS)
            return status;

        if (juno_css_debug_dev->manual_reset_required) {
            SCP_CONFIG->SYS_MANUAL_RESET.SET =
                SCP_CONFIG_SYS_MANUAL_RESET_DBGSYSRESET;
            SCP_CONFIG->SYS_MANUAL_RESET.CLEAR =
                SCP_CONFIG_SYS_MANUAL_RESET_DBGSYSRESET;
        }

        /* Enable CoreSight timestamp generator (with halt-on-debug) */
        CS_CNTCONTROL->CS_CNTCR |= (CNTCONTROL_CR_EN | CNTCONTROL_CR_HDBG);
    }

    dev_ctx.pd_dbg_on = true;

    /*
     * This request originated from an IRQ, therefore acknowledge is
     * required and the ISR needs to be re-enabled.
     */
    if (dev_ctx.user_id == SCP_DEBUG_USER_DAP)
        set_ack_debug_pwrup_req();

    return FWK_SUCCESS;
}

/*
 * Framework handlers
 */

static int juno_debug_init(fwk_id_t module_id,
                           unsigned int element_count,
                           const void *data)
{
    fwk_assert(data != NULL);
    fwk_assert(element_count == 1);

    return FWK_SUCCESS;
}

static int juno_debug_element_init(fwk_id_t element_id,
                                   unsigned int sub_element_count,
                                   const void *data)
{
    return fwk_thread_create(element_id);
}

static int juno_debug_bind(fwk_id_t id, unsigned int round)
{
    int status;
    const struct mod_juno_debug_config *module_config;

    /* Bind in the second round */
    if (round == 0)
        return FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        module_config = fwk_module_get_data(fwk_module_id_juno_debug);

        /* Bind to Timer */
        return fwk_module_bind(module_config->timer_id,
                               MOD_TIMER_API_ID_TIMER,
                               &timer_api);
    }

    status = fwk_module_bind(fwk_module_id_power_domain,
                             mod_pd_api_id_restricted,
                             &dev_ctx.pd_api);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_module_bind(dev_ctx.debug_hal_id,
                           FWK_ID_API(FWK_MODULE_IDX_DEBUG,
                                      MOD_DEBUG_API_IDX_DRIVER_INPUT),
                           &dev_ctx.driver_input_api);
}

static int juno_debug_process_bind_request(fwk_id_t source_id,
                                           fwk_id_t target_id,
                                           fwk_id_t api_id,
                                           const void **api)
{
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT) ||
        !fwk_id_is_equal(api_id, mod_juno_debug_api_id_driver) ||
        api == NULL)
        return FWK_E_PARAM;

    *api = &juno_debug_drv_api;

    dev_ctx.debug_hal_id = source_id;

    return FWK_SUCCESS;
}

static int juno_debug_start(fwk_id_t id)
{
    int status;

    status = fwk_interrupt_set_isr(CDBG_PWR_UP_REQ_IRQ,
                                   juno_debug_cdbg_pwr_up_req_isr);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_interrupt_set_isr(CSYS_PWR_UP_REQ_IRQ,
                                   juno_debug_csys_pwr_up_req_isr);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_interrupt_set_isr(CDBG_RST_REQ_IRQ,
                                   juno_debug_cdbg_rst_req_isr);
    if (status != FWK_SUCCESS)
        return status;

    fwk_interrupt_clear_pending(CDBG_PWR_UP_REQ_IRQ);
    fwk_interrupt_clear_pending(CDBG_RST_REQ_IRQ);
    fwk_interrupt_clear_pending(CSYS_PWR_UP_REQ_IRQ);

    fwk_interrupt_enable(CDBG_PWR_UP_REQ_IRQ);
    fwk_interrupt_enable(CDBG_RST_REQ_IRQ);
    fwk_interrupt_enable(CSYS_PWR_UP_REQ_IRQ);

    dev_ctx.state = IDLE;

    return FWK_SUCCESS;
}

static int juno_debug_process_event(const struct fwk_event *event,
                                    struct fwk_event *resp_event)
{
    int status;
    struct mod_debug_response_params dbg_resp_params;
    const struct mod_juno_debug_dev_config *config =
        fwk_module_get_data(event->target_id);
    struct pd_get_state_response *pd_get_resp_params;
    struct pd_set_state_response *pd_set_resp_params;
    struct juno_css_debug_dev *juno_css_debug_dev = config->clk_settings;

    if (fwk_id_is_equal(event->id,
                        juno_debug_event_submit_request)) {
        status = process_enable_request(event->target_id);
        if (status == FWK_PENDING)
            return FWK_SUCCESS;

    } else {

        switch (fwk_id_get_event_idx(event->id)) {
        /* Response event from PD */
        case MOD_PD_PUBLIC_EVENT_IDX_GET_STATE:
        case MOD_PD_PUBLIC_EVENT_IDX_SET_STATE:
            pd_set_resp_params = (struct pd_set_state_response *)event->params;

            if (dev_ctx.state != GET_STATE &&
                pd_set_resp_params->status != FWK_SUCCESS) {
                status = pd_set_resp_params->status;
                break;
            }

            switch (dev_ctx.state) {
            case GET_STATE:
                pd_get_resp_params =
                    (struct pd_get_state_response *)event->params;

                if (pd_get_resp_params->status != FWK_SUCCESS) {
                    status = pd_get_resp_params->status;
                    break;
                }

                if (pd_get_resp_params->state == MOD_PD_STATE_ON) {
                    /*
                     * This request originated from an IRQ, therefore
                     * acknowledgment is required and the ISR needs to be
                     * re-enabled.
                     */
                    if (dev_ctx.user_id == SCP_DEBUG_USER_DAP)
                        set_ack_debug_pwrup_req();

                    status = FWK_SUCCESS;
                    break;

                }

                juno_utils_system_clock_enable(debug_clk_mask);

                /* Turn on DBGSYS */
                status = turn_on_pd(config->pd_dbgsys_id, SET_STATE_DBGSYS);
                if (status == FWK_PENDING)
                    return FWK_SUCCESS;

                if (status != FWK_SUCCESS)
                    break;

                /* FALLTHRU */
            case SET_STATE_DBGSYS:
                /* Turn on BIG_SSTOP */
                status = turn_on_pd(config->pd_big_sstop_id,
                                    SET_STATE_BIG_SSTOP);
                if (status == FWK_PENDING)
                    return FWK_SUCCESS;

                if (status != FWK_SUCCESS)
                    break;

                /* FALLTHRU */
            case SET_STATE_BIG_SSTOP:
                /* Turn on LITTLE_SSTOP */
                status = turn_on_pd(config->pd_little_sstop_id,
                                    SET_STATE_LITTLE_SSTOP);
                if (status == FWK_PENDING)
                    return FWK_SUCCESS;

                if (status != FWK_SUCCESS)
                    break;

                /* FALLTHRU */
            case SET_STATE_LITTLE_SSTOP:
                /*
                * Apply clock settings
                */
                status = apply_clock_settings(juno_css_debug_dev);
                if (status != FWK_SUCCESS)
                    break;

                if (juno_css_debug_dev->manual_reset_required) {
                    SCP_CONFIG->SYS_MANUAL_RESET.SET =
                        SCP_CONFIG_SYS_MANUAL_RESET_DBGSYSRESET;
                    SCP_CONFIG->SYS_MANUAL_RESET.CLEAR =
                        SCP_CONFIG_SYS_MANUAL_RESET_DBGSYSRESET;
                    }

                /* Enable CoreSight timestamp generator (with halt-on-debug) */
                CS_CNTCONTROL->CS_CNTCR |=
                    (CNTCONTROL_CR_EN | CNTCONTROL_CR_HDBG);

                dev_ctx.pd_dbg_on = true;

                /*
                 * This request originated from an IRQ, therefore
                 * acknowledgment is required and the ISR needs to be
                 * re-enabled.
                 */
                if (dev_ctx.user_id == SCP_DEBUG_USER_DAP)
                    set_ack_debug_pwrup_req();

                status = FWK_SUCCESS;

                break;

            default:
                return FWK_E_PANIC;
            }

            break;

        default:
            return FWK_E_PARAM;
        }
    }

    dbg_resp_params.status = status;
    dbg_resp_params.enabled = dev_ctx.pd_dbg_on;

    dev_ctx.state = IDLE;

    dev_ctx.driver_input_api->request_complete(dev_ctx.debug_hal_id,
                                               &dbg_resp_params);

    return FWK_SUCCESS;
}

struct fwk_module module_juno_debug = {
    .name = "JUNO DEBUG",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_JUNO_DEBUG_API_IDX_COUNT,
    .event_count = JUNO_DEBUG_EVENT_IDX_COUNT,
    .init = juno_debug_init,
    .element_init = juno_debug_element_init,
    .bind = juno_debug_bind,
    .process_bind_request = juno_debug_process_bind_request,
    .start = juno_debug_start,
    .process_event = juno_debug_process_event,
};
