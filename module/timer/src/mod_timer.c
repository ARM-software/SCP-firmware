/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *    Implementation of Timer module
 */

#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_dlist.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_list.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Timer device context (element) */
struct dev_ctx {
    /* Pointer to the device's configuration */
    const struct mod_timer_dev_config *config;
    /* Pointer to an API provided by the driver that controls the device */
    struct mod_timer_driver_api *driver;
    /* Identifier of the driver that controls the device */
    fwk_id_t driver_dev_id;
    /* Storage for all alarms */
    struct alarm_ctx *alarm_pool;
    /* Queue of active alarms */
    struct fwk_dlist alarms_active;
};

/* Alarm item context (sub-element) */
struct alarm_ctx {
    /* List node */
    struct fwk_dlist_node node;
    /* Time between starting this alarm and it triggering */
    uint32_t microseconds;
    /* Timestamp of the time this alarm will trigger */
    uint64_t timestamp;
    /* Pointer to the callback function */
    void (*callback)(uintptr_t param);
    /* Parameter of the callback function */
    uintptr_t param;
    /* Flag indicating if this alarm if periodic */
    bool periodic;
    /* Flag indicating if this alarm is in the active queue */
    bool activated;
    /* Flag indicating if this alarm has been bound to */
    bool bound;
    /* Flag indicating if this alarm is started */
    bool started;
};

/* Table of timer device context structures */
static struct dev_ctx *ctx_table;

/*
 * Forward declarations
 */

static void timer_isr(uintptr_t ctx_ptr);

/*
 * Internal functions
 */

static int _time_to_timestamp(struct dev_ctx *ctx,
                              uint32_t microseconds,
                              uint64_t *timestamp)
{
    int status;
    uint32_t frequency;

    assert(ctx != NULL);
    assert(timestamp != NULL);

    status = ctx->driver->get_frequency(ctx->driver_dev_id, &frequency);
    if (status != FWK_SUCCESS)
        return status;

    *timestamp = ((uint64_t)frequency * microseconds) / 1000000;

    return FWK_SUCCESS;
}

static int _timestamp_from_now(struct dev_ctx *ctx,
                               uint32_t microseconds,
                               uint64_t *timestamp)
{
    int status;
    uint64_t counter;

    assert(ctx != NULL);
    assert(timestamp != NULL);

    status = _time_to_timestamp(ctx, microseconds, timestamp);
    if (status != FWK_SUCCESS)
        return status;

    status = ctx->driver->get_counter(ctx->driver_dev_id, &counter);
    if (status != FWK_SUCCESS)
        return status;

    *timestamp += counter;

    return FWK_SUCCESS;
}

static int _remaining(const struct dev_ctx *ctx,
                      uint64_t timestamp,
                      uint64_t *remaining_ticks)
{
    int status;
    uint64_t counter;

    fwk_assert(ctx != NULL);
    fwk_assert(remaining_ticks != NULL);

    status = ctx->driver->get_counter(ctx->driver_dev_id, &counter);
    if (!fwk_expect(status == FWK_SUCCESS))
        return status;

    /* If timestamp is in the past, remaining_ticks is set to zero. */
    if (timestamp < counter)
        *remaining_ticks = 0;
    else
        *remaining_ticks = timestamp - counter;

    return FWK_SUCCESS;
}

static void _configure_timer_with_next_alarm(struct dev_ctx *ctx)
{
    struct alarm_ctx *alarm_head;

    assert(ctx != NULL);

    alarm_head = (struct alarm_ctx *)fwk_list_head(&ctx->alarms_active);
    if (alarm_head != NULL) {
        /* Configure timer device */
        ctx->driver->set_timer(ctx->driver_dev_id, alarm_head->timestamp);
        ctx->driver->enable(ctx->driver_dev_id);
    }
}

static void _insert_alarm_ctx_into_active_queue(struct dev_ctx *ctx,
                                                struct alarm_ctx *alarm_new)
{
    struct fwk_dlist_node *alarm_node;
    struct alarm_ctx *alarm;

    assert(ctx != NULL);
    assert(alarm_new != NULL);

    /*
     * Search though the active queue to find the correct place to insert the
     * new alarm item
     */
    alarm_node = fwk_list_head(&ctx->alarms_active);
    alarm = FWK_LIST_GET(alarm_node, struct alarm_ctx, node);

    while ((alarm_node != NULL) && (alarm_new->timestamp > alarm->timestamp)) {
        alarm_node = fwk_list_next(&ctx->alarms_active, alarm_node);
        alarm = FWK_LIST_GET(alarm_node, struct alarm_ctx, node);
    }

    /* Insert alarm_new just BEFORE the alarm that was found */
    fwk_list_insert(&ctx->alarms_active,
                    &(alarm_new->node),
                    alarm_node);

    alarm_new->activated = true;
}


/*
 * Functions fulfilling the timer API
 */

static int get_frequency(fwk_id_t dev_id, uint32_t *frequency)
{
    struct dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(dev_id)];

    if (frequency == NULL)
        return FWK_E_PARAM;

    return ctx->driver->get_frequency(ctx->driver_dev_id, frequency);
}

static int time_to_timestamp(fwk_id_t dev_id,
                             uint32_t microseconds,
                             uint64_t *timestamp)
{
    struct dev_ctx *ctx;

    if (timestamp == NULL)
        return FWK_E_PARAM;

    ctx = &ctx_table[fwk_id_get_element_idx(dev_id)];

    return _time_to_timestamp(ctx, microseconds, timestamp);
}

static int get_counter(fwk_id_t dev_id, uint64_t *counter)
{
    struct dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(dev_id)];

    if (counter == NULL)
        return FWK_E_PARAM;

    /* Read counter */
    return ctx->driver->get_counter(ctx->driver_dev_id, counter);
}

static int delay(fwk_id_t dev_id, uint32_t microseconds)
{
    int status;
    struct dev_ctx *ctx;
    uint64_t counter, counter_limit;

    ctx = &ctx_table[fwk_id_get_element_idx(dev_id)];

    status = _timestamp_from_now(ctx, microseconds, &counter_limit);
    if (status != FWK_SUCCESS)
        return status;

    do {
        status = ctx->driver->get_counter(ctx->driver_dev_id, &counter);
        if (status != FWK_SUCCESS)
            return status;
    } while (counter < counter_limit);

    return FWK_SUCCESS;
}

static int wait(fwk_id_t dev_id,
                uint32_t microseconds,
                bool (*cond)(void*),
                void *data)
{
    struct dev_ctx *ctx;
    int status;
    uint64_t counter, counter_limit;

    ctx = &ctx_table[fwk_id_get_element_idx(dev_id)];

    status = _timestamp_from_now(ctx, microseconds, &counter_limit);
    if (status != FWK_SUCCESS)
        return status;

    while (true) {

        if (cond(data))
            return FWK_SUCCESS;

        status = ctx->driver->get_counter(ctx->driver_dev_id, &counter);
        if (status != FWK_SUCCESS)
            return FWK_E_DEVICE;

        /*
         * If the time to wait is over, check condition one last time.
         */
        if (counter > counter_limit) {
            if (cond(data))
                return FWK_SUCCESS;
            else
                return FWK_E_TIMEOUT;
        }
    }
}

static int remaining(fwk_id_t dev_id,
                     uint64_t timestamp,
                     uint64_t *remaining_ticks)
{
    struct dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(dev_id)];

    if (remaining_ticks == NULL)
        return FWK_E_PARAM;

    return _remaining(ctx, timestamp, remaining_ticks);
}

static int get_next_alarm_remaining(fwk_id_t dev_id,
                                    bool *has_alarm,
                                    uint64_t *remaining_ticks)
{
    int status = FWK_E_PARAM;
    const struct dev_ctx *ctx;
    const struct alarm_ctx *alarm_ctx;
    const struct fwk_dlist_node *alarm_ctx_node;

    if (has_alarm == NULL)
        return FWK_E_PARAM;

    if (remaining_ticks == NULL)
        return FWK_E_PARAM;

    ctx = &ctx_table[fwk_id_get_element_idx(dev_id)];

    /*
     * The timer interrupt is disabled to ensure that the alarm list is not
     * modified while we are trying to read it below.
     */
    ctx->driver->disable(ctx->driver_dev_id);

    *has_alarm = !fwk_list_is_empty(&ctx->alarms_active);

    if (*has_alarm) {
        alarm_ctx_node = fwk_list_head(&ctx->alarms_active);
        alarm_ctx = FWK_LIST_GET(alarm_ctx_node, struct alarm_ctx, node);

        status = _remaining(ctx, alarm_ctx->timestamp, remaining_ticks);
    }

    ctx->driver->enable(ctx->driver_dev_id);

    return status;
}

static const struct mod_timer_api timer_api = {
    .get_frequency = get_frequency,
    .time_to_timestamp = time_to_timestamp,
    .get_counter = get_counter,
    .delay = delay,
    .wait = wait,
    .remaining = remaining,
    .get_next_alarm_remaining = get_next_alarm_remaining,
};

/*
 * Functions fulfilling the alarm API
 */

static int alarm_stop(fwk_id_t alarm_id)
{
    int status;
    struct dev_ctx *ctx;
    struct alarm_ctx *alarm;
    unsigned int interrupt;

    assert(fwk_module_is_valid_sub_element_id(alarm_id));

    ctx = &ctx_table[fwk_id_get_element_idx(alarm_id)];

    status = fwk_interrupt_get_current(&interrupt);
    switch (status) {
    case FWK_E_STATE:
        /* Not within an ISR */
        break;

    case FWK_SUCCESS:
        /* Within an ISR */

        if (interrupt == ctx->config->timer_irq) {
            /*
             * The interrupt handler is the interrupt handler for the alarm's
             * timer
             */
            break;
        }
        /* Fall-through */

    default:
        return FWK_E_ACCESS;
    }

    alarm = &ctx->alarm_pool[fwk_id_get_sub_element_idx(alarm_id)];

    /* Prevent possible data races with the timer interrupt */
    ctx->driver->disable(ctx->driver_dev_id);

    if (!alarm->started) {
        ctx->driver->enable(ctx->driver_dev_id);
        return FWK_E_STATE;
    }

    alarm->started = false;

    if (!alarm->activated)
        return FWK_SUCCESS;

    /*
     * If the alarm is stopped while the interrupts are globally disabled, an
     * interrupt may be pending because the alarm being stopped here has
     * triggered. If the interrupt is not cleared then when the interrupts are
     * re-enabled, the timer ISR will be executed but the alarm, cause of the
     * interrupt, will have disappeared. To avoid that, the timer interrupt is
     * cleared here. If the interrupt was triggered by another alarm, it will be
     * re-triggered when the timer interrupt is re-enabled.
     */
    fwk_interrupt_clear_pending(ctx->config->timer_irq);

    fwk_list_remove(&ctx->alarms_active, (struct fwk_dlist_node *)alarm);
    alarm->activated = false;

    _configure_timer_with_next_alarm(ctx);

    return FWK_SUCCESS;
}

static int alarm_start(fwk_id_t alarm_id,
                       unsigned int milliseconds,
                       enum mod_timer_alarm_type type,
                       void (*callback)(uintptr_t param),
                       uintptr_t param)
{
    int status;
    struct dev_ctx *ctx;
    struct alarm_ctx *alarm;
    unsigned int interrupt;

    assert(fwk_module_is_valid_sub_element_id(alarm_id));

    status = fwk_interrupt_get_current(&interrupt);
    if (status != FWK_E_STATE) {
        /*
         * Could not attain call context OR this function is called from an
         * interrupt handler.
         */
        return FWK_E_ACCESS;
    }

    ctx = ctx_table + fwk_id_get_element_idx(alarm_id);
    alarm = &ctx->alarm_pool[fwk_id_get_sub_element_idx(alarm_id)];

    if (alarm->started)
        alarm_stop(alarm_id);

    alarm->started = true;

    /* Cap to ensure value will not overflow when stored as microseconds */
    milliseconds = FWK_MIN(milliseconds, UINT32_MAX / 1000);

    /* Populate alarm item */
    alarm->callback = callback;
    alarm->param = param;
    alarm->periodic =
        (type == MOD_TIMER_ALARM_TYPE_PERIODIC ? true : false);
    alarm->microseconds = milliseconds * 1000;
    status = _timestamp_from_now(ctx,
                                 alarm->microseconds,
                                 &alarm->timestamp);
    if (status != FWK_SUCCESS)
        return status;

    /* Disable timer interrupts to work with the active queue */
    ctx->driver->disable(ctx->driver_dev_id);

    _insert_alarm_ctx_into_active_queue(ctx, alarm);

    _configure_timer_with_next_alarm(ctx);

    return FWK_SUCCESS;
}

static const struct mod_timer_alarm_api alarm_api = {
    .start = alarm_start,
    .stop = alarm_stop,
};

static void timer_isr(uintptr_t ctx_ptr)
{
    int status;
    struct alarm_ctx *alarm;
    struct dev_ctx *ctx = (struct dev_ctx *)ctx_ptr;
    uint64_t timestamp = 0;

    assert(ctx != NULL);

    /* Disable timer interrupts to work with the active queue */
    ctx->driver->disable(ctx->driver_dev_id);
    fwk_interrupt_clear_pending(ctx->config->timer_irq);

    alarm = (struct alarm_ctx *)fwk_list_pop_head(&ctx->alarms_active);

    if (alarm == NULL) {
        /* Timer interrupt triggered without any alarm in the active queue */
        assert(false);
        return;
    }

    alarm->activated = false;

    /* Execute the callback function */
    alarm->callback(alarm->param);

    if (alarm->periodic && alarm->started) {
        /* Put this alarm back into the active queue */
        status = _time_to_timestamp(ctx, alarm->microseconds, &timestamp);

        if (status == FWK_SUCCESS) {
            alarm->timestamp += timestamp;
            _insert_alarm_ctx_into_active_queue(ctx, alarm);
        } else {
            FWK_LOG_ERR(
                "[Timer] Error: Periodic alarm could not be added "
                "back into queue.");
        }
    }

    _configure_timer_with_next_alarm(ctx);
}

/*
 * Functions fulfilling the framework's module interface
 */

static int timer_init(fwk_id_t module_id,
                      unsigned int element_count,
                      const void *data)
{
    ctx_table = fwk_mm_calloc(element_count, sizeof(struct dev_ctx));

    return FWK_SUCCESS;
}

static int timer_device_init(fwk_id_t element_id, unsigned int alarm_count,
                             const void *data)
{
    struct dev_ctx *ctx;

    assert(data != NULL);

    ctx = ctx_table + fwk_id_get_element_idx(element_id);
    ctx->config = data;

    if (alarm_count > 0)
        ctx->alarm_pool = fwk_mm_calloc(alarm_count, sizeof(struct alarm_ctx));

    return FWK_SUCCESS;
}

static int timer_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct dev_ctx *ctx;
    struct mod_timer_driver_api *driver = NULL;
    unsigned int driver_module_idx;

    /* Nothing to do after the initial round. */
    if (round > 0)
        return FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    ctx = ctx_table + fwk_id_get_element_idx(id);
    ctx->driver_dev_id = ctx->config->id;

    /* Bind to the driver API for the current device */
    driver_module_idx = fwk_id_get_module_idx(ctx->driver_dev_id);
    status = fwk_module_bind(ctx->driver_dev_id,
                             FWK_ID_API(driver_module_idx, 0),
                             &driver);
    if (status != FWK_SUCCESS)
        return status;

    /* Check that the driver API is completely fulfilled */
    if (driver->enable == NULL      ||
        driver->disable == NULL     ||
        driver->get_counter == NULL ||
        driver->get_frequency == NULL)
        return FWK_E_DEVICE;

    ctx->driver = driver;

    return FWK_SUCCESS;
}

static int timer_process_bind_request(fwk_id_t requester_id,
                                      fwk_id_t id,
                                      fwk_id_t api_id,
                                      const void **api)
{
    struct dev_ctx *ctx;
    struct alarm_ctx *alarm_ctx;

    if (fwk_id_is_equal(api_id, MOD_TIMER_API_ID_TIMER)) {
        if (!fwk_module_is_valid_element_id(id)) {
            assert(false);
            return FWK_E_PARAM;
        }

        *api = &timer_api;
        return FWK_SUCCESS;
    }

    /* Alarm API requested */

    if (!fwk_module_is_valid_sub_element_id(id)) {
        assert(false);
        return FWK_E_PARAM;
    }

    ctx = ctx_table + fwk_id_get_element_idx(id);
    alarm_ctx = &ctx->alarm_pool[fwk_id_get_sub_element_idx(id)];

    if (alarm_ctx->bound) {
        assert(false);
        return FWK_E_STATE;
    }

    alarm_ctx->bound = true;

    *api = &alarm_api;
    return FWK_SUCCESS;
}

static int timer_start(fwk_id_t id)
{
    struct dev_ctx *ctx;

    if (!fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    ctx = ctx_table + fwk_id_get_element_idx(id);

    fwk_list_init(&ctx->alarms_active);

    fwk_interrupt_set_isr_param(ctx->config->timer_irq,
                                timer_isr,
                                (uintptr_t)ctx);
    fwk_interrupt_enable(ctx->config->timer_irq);

    return FWK_SUCCESS;
}

/* Module descriptor */
const struct fwk_module module_timer = {
    .name = "Timer HAL",
    .api_count = MOD_TIMER_API_COUNT,
    .type = FWK_MODULE_TYPE_HAL,
    .init = timer_init,
    .element_init = timer_device_init,
    .bind = timer_bind,
    .process_bind_request = timer_process_bind_request,
    .start = timer_start,
};
