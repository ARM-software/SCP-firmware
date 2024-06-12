/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System Monitoring Control Framwork.
 */

#include "smcf_data.h"

#include <mod_smcf.h>

#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#ifdef BUILD_HAS_NOTIFICATION
#    include <fwk_notification.h>
#endif

/* SMCF module event indexes */
enum pd_event_idx { SMCF_NEW_DATA_SAMPLE, SMCF_EVENT_COUNT };

/* New data sample event identifier */
static const fwk_id_t smcf_event_id_new_data_sample =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SMCF, SMCF_NEW_DATA_SAMPLE);

/* Element context */
struct smcf_element_ctx {
    /* Context Domain ID */
    fwk_id_t domain_id;

    /* Element config */
    const struct mod_smcf_element_config *config;

    /* MGI register */
    struct smcf_mgi_reg *mgi;

    /* Number of monitors for this MGI */
    uint32_t monitor_count;

    /* Current sample type */
    uint32_t sample_type;

    /* Data attributes */
    struct smcf_data_attr data_attr;

    /* Timer context*/
    struct smcf_mgi_timer_ctx *timer_ctx;
};

/* Module context */
struct smcf_ctx {
    /* Table of the element contexts */
    struct smcf_element_ctx *element_ctx_table;

    /* Number of MGI domains */
    size_t element_ctx_table_size;
};

struct smcf_interrupt_methods {
    /* Check if the interrupt source */
    bool (*is_irq_triggered)(struct smcf_mgi_reg *smcf_mgi);

    /* Interrupt clear method */
    void (*clear_irq)(struct smcf_mgi_reg *smcf_mgi);

    /* Interrupt handler */
    void (*irq_handler)(struct smcf_element_ctx *element_ctx);
};

static struct smcf_ctx mod_ctx;

static struct smcf_element_ctx *get_domain_ctx(fwk_id_t domain_id)
{
    uint32_t idx = fwk_id_get_element_idx(domain_id);
    if (idx >= mod_ctx.element_ctx_table_size) {
        return NULL;
    }

    return &mod_ctx.element_ctx_table[idx];
}

static int smcf_start_data_sample(fwk_id_t element_id)
{
    struct smcf_element_ctx *element_ctx;

    element_ctx = get_domain_ctx(element_id);
    if (element_ctx == NULL) {
        return FWK_E_PARAM;
    }

    mgi_enable_sample(element_ctx->mgi);

    return FWK_SUCCESS;
}

static int smcf_validate_tag(
    struct smcf_element_ctx *element_ctx,
    struct mod_smcf_buffer tag_buffer)
{
    uint32_t tag_length;

    if (tag_buffer.ptr == NULL) {
        return FWK_E_PARAM;
    }

    tag_length = smcf_data_get_tag_length(element_ctx->data_attr.header);
    if (tag_length == 0) {
        return FWK_E_SUPPORT;
    }

    if (tag_buffer.size < tag_length) {
        return FWK_E_NOMEM;
    }

    return FWK_SUCCESS;
}

static int smcf_get_element_data(
    fwk_id_t monitor_id,
    struct mod_smcf_buffer data_buffer,
    struct mod_smcf_buffer tag_buffer)
{
    struct smcf_element_ctx *element_ctx;
    unsigned int monitor_index;
    uint32_t dest_size;
    int status;

    if ((data_buffer.size == 0) || (data_buffer.ptr == NULL)) {
        return FWK_E_PARAM;
    }

    if (!fwk_module_is_valid_sub_element_id(monitor_id)) {
        return FWK_E_PARAM;
    }

    element_ctx = get_domain_ctx(monitor_id);
    if (element_ctx == NULL) {
        return FWK_E_PARAM;
    }

    monitor_index = fwk_id_get_sub_element_idx(monitor_id);

    dest_size = smcf_data_get_data_buffer_size(element_ctx->data_attr);
    if (data_buffer.size < dest_size) {
        return FWK_E_NOMEM;
    }

    if (tag_buffer.size == 0) {
        tag_buffer.ptr = NULL;
    } else {
        status = smcf_validate_tag(element_ctx, tag_buffer);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return smcf_data_get_data(
        element_ctx->data_attr, monitor_index, data_buffer.ptr, tag_buffer.ptr);
}

static void sample_data_set_complete_handler(
    struct smcf_element_ctx *element_ctx)
{
    struct fwk_event_light req;
    int status;

    req = (struct fwk_event_light){
        .target_id = element_ctx->domain_id,
        .source_id = element_ctx->domain_id,
        .id = smcf_event_id_new_data_sample,
    };

    status = fwk_put_event(&req);
    if (status != FWK_SUCCESS) {
        FWK_TRACE("[SMCF] Send data sample event failed!");
    }
}

static void no_handler_for_this_interrupt_source(
    struct smcf_element_ctx *element_ctx)
{
    FWK_TRACE("[SMCF] Interrupt received but the event is not handled");
}

static void (*mgi_interrupt_manager_table[SMCF_MGI_IRQ_SOURCE_MAX])(
    struct smcf_element_ctx *element_ctx) = {
    [SMCF_MGI_IRQ_SOURCE_SMP_CMP] = sample_data_set_complete_handler,
    [SMCF_MGI_IRQ_SOURCE_MON_EN] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_MON_MODE] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_CMD_RECV] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_ERR] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_MON_TRIG] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_IN_TRIG] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_CFG] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_DATA_WR] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_ALT0] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_ALT1] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_ALT2] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_ALT3] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_ALT4] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_ALT5] = no_handler_for_this_interrupt_source,
    [SMCF_MGI_IRQ_SOURCE_ALT6] = no_handler_for_this_interrupt_source,
};

static void smcf_mgi_interrupt_handler(
    struct smcf_element_ctx *element_ctx,
    uint32_t interrupt_source)
{
    if (mgi_interrupt_manager_table[interrupt_source] != NULL) {
        mgi_interrupt_manager_table[interrupt_source](element_ctx);
    }

    mgi_interrupt_source_clear(element_ctx->mgi, interrupt_source);
}

static void smcf_interrupt_handlers(uintptr_t element_ctx_param)
{
    struct smcf_element_ctx *element_ctx =
        (struct smcf_element_ctx *)element_ctx_param;
    struct smcf_mgi_reg *mgi = element_ctx->mgi;
    uint32_t interrupt_source;

    for (interrupt_source = 0; interrupt_source < SMCF_MGI_IRQ_SOURCE_MAX;
         interrupt_source++) {
        if (mgi_is_the_source_triggered_the_interrupt(mgi, interrupt_source)) {
            smcf_mgi_interrupt_handler(element_ctx, interrupt_source);
        }
    }
}

static int smcf_external_module_handle_the_interrupt_api(fwk_id_t element_id)
{
    struct smcf_element_ctx *element_ctx;

    element_ctx = get_domain_ctx(element_id);
    if (element_ctx == NULL) {
        return FWK_E_PARAM;
    }

    smcf_interrupt_handlers((uintptr_t)element_ctx);

    return FWK_SUCCESS;
}

static int smcf_mli_config_mode_validate(
    fwk_id_t mli_id,
    uint32_t mode_value,
    uint32_t mode_index)
{
    struct smcf_element_ctx *element_ctx;
    uint32_t num_mode_entries;
    uint32_t num_mode_bits;

    if (!fwk_module_is_valid_sub_element_id(mli_id)) {
        return FWK_E_PARAM;
    }

    element_ctx = get_domain_ctx(mli_id);

    num_mode_entries = mgi_get_number_of_mode_registers(element_ctx->mgi);
    if (mode_index + 1 > (num_mode_entries)) {
        return FWK_E_RANGE;
    }

    num_mode_bits = mgi_get_number_of_bits_in_mode_registers(element_ctx->mgi);
    if (num_mode_bits == SMCF_MGI_MODE_MAX_WORD_LEN) {
        return FWK_SUCCESS;
    } else if (mode_value > ((1U << num_mode_bits) - 1U)) {
        return FWK_E_DATA;
    }

    return FWK_SUCCESS;
}

static int smcf_mli_config_mode_set(
    fwk_id_t mli_id,
    uint32_t mode_value,
    uint32_t mode_index)
{
    int status = FWK_SUCCESS;
    uint32_t mli_index = fwk_id_get_sub_element_idx(mli_id);
    struct smcf_element_ctx *element_ctx = get_domain_ctx(mli_id);

    status = mgi_enable_program_mode(element_ctx->mgi, mli_index);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = mgi_set_monitor_mode(
        element_ctx->mgi, element_ctx->timer_ctx, mode_index, mode_value);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return mgi_disable_program_mode(element_ctx->mgi, mli_index);
}

static int smcf_mli_config_mode(
    fwk_id_t mli_id,
    uint32_t mode_value,
    uint32_t mode_index)
{
    int status = FWK_SUCCESS;

    status = smcf_mli_config_mode_validate(mli_id, mode_value, mode_index);
    if (status != FWK_SUCCESS) {
        return FWK_SUCCESS;
    }

    status = smcf_mli_config_mode_set(mli_id, mode_value, mode_index);

    return status;
}

static uint32_t smcf_get_group_id(fwk_id_t monitor_group_id)
{
    struct smcf_element_ctx *element_ctx;

    element_ctx = get_domain_ctx(monitor_group_id);

    return smcf_data_get_group_id(element_ctx->data_attr);
}

static int smcf_mli_enable(fwk_id_t mli_id)
{
    struct smcf_element_ctx *element_ctx;
    uint32_t mli_index;

    if (!fwk_module_is_valid_sub_element_id(mli_id)) {
        return FWK_E_PARAM;
    }

    element_ctx = get_domain_ctx(mli_id);

    mli_index = fwk_id_get_sub_element_idx(mli_id);

    return mgi_enable_monitor(element_ctx->mgi, mli_index);
}

static int smcf_mli_disable(fwk_id_t mli_id)
{
    struct smcf_element_ctx *element_ctx;
    uint32_t mli_index;

    if (!fwk_module_is_valid_sub_element_id(mli_id)) {
        return FWK_E_PARAM;
    }

    element_ctx = get_domain_ctx(mli_id);

    mli_index = fwk_id_get_sub_element_idx(mli_id);

    return mgi_disable_monitor(element_ctx->mgi, mli_index);
}

static const struct smcf_data_api data_api = {
    .start_data_sampling = smcf_start_data_sample,
    .get_data = smcf_get_element_data,
};

static const struct smcf_control_api control_api = {
    .config_mode = smcf_mli_config_mode,
    .get_group_id = smcf_get_group_id,
    .mli_enable = smcf_mli_enable,
    .mli_disable = smcf_mli_disable,
};

static const struct smcf_monitor_group_interrupt_api smcf_interrupt_api = {
    .handle_interrupt = smcf_external_module_handle_the_interrupt_api,
};

static int smcf_mod_init(
    fwk_id_t module_id,
    unsigned int mgi_count,
    const void *unused)
{
    mod_ctx.element_ctx_table =
        fwk_mm_calloc(mgi_count, sizeof(struct smcf_element_ctx));

    mod_ctx.element_ctx_table_size = mgi_count;

    return FWK_SUCCESS;
}

static int smcf_element_init_config_sample_type(
    struct smcf_element_ctx *element_ctx)
{
    int status;

    switch (element_ctx->config->sample_type) {
    case SMCF_SAMPLE_TYPE_MANUAL:
        status =
            mgi_set_sample_type(element_ctx->mgi, SMCF_MGI_SAMPLE_TYPE_MANUAL);
        break;

    case SMCF_SAMPLE_TYPE_PERIODIC:
        status = mgi_set_sample_type(
            element_ctx->mgi, SMCF_MGI_SAMPLE_TYPE_PERIODIC);
        break;

    case SMCF_SAMPLE_TYPE_DATA_READ:
        status = mgi_set_sample_type(
            element_ctx->mgi, SMCF_MGI_SAMPLE_TYPE_DATA_READ);
        break;

    case SMCF_SAMPLE_TYPE_TRIGGER_INPUT:
        status = mgi_set_sample_type(
            element_ctx->mgi, SMCF_MGI_SAMPLE_TYPE_TRIGGER_INPUT);
        break;

    default:
        return FWK_E_PANIC;
    }

    if (status != FWK_SUCCESS) {
        element_ctx->sample_type = SMCF_SAMPLE_TYPE_MANUAL;
    } else {
        element_ctx->sample_type = element_ctx->config->sample_type;
    }

    return status;
}

static int smcf_element_init_set_data_attributes(struct smcf_element_ctx *ctx)
{
    ctx->data_attr.num_of_data =
        mgi_number_of_data_values_per_monitor(ctx->mgi);
    ctx->data_attr.data_width = mgi_monitor_data_width(ctx->mgi);
    ctx->data_attr.packed = mgi_is_data_packed(ctx->mgi);

    return smcf_data_set_data_address(
        ctx->mgi, ctx->config->data_config, &ctx->data_attr);
}

static void smcf_enable_interrupt(struct smcf_element_ctx *element_ctx)
{
    uint32_t interrupt_source;

    for (interrupt_source = 0; interrupt_source < SMCF_MGI_IRQ_SOURCE_MAX;
         interrupt_source++) {
        if (mgi_interrupt_manager_table[interrupt_source] != NULL) {
            mgi_interrupt_source_unmask(element_ctx->mgi, interrupt_source);
        }
    }
}

static void smcf_element_init_setup_interrupt(
    struct smcf_element_ctx *element_ctx)
{
    if (element_ctx->config->irq == FWK_INTERRUPT_NONE) {
        return;
    }

    fwk_interrupt_set_isr_param(
        element_ctx->config->irq,
        smcf_interrupt_handlers,
        (uintptr_t)element_ctx);

    smcf_enable_interrupt(element_ctx);
}

static int smcf_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    const struct mod_smcf_element_config *config = data;
    struct smcf_element_ctx *ctx;
    int status;

    ctx = get_domain_ctx(element_id);

    ctx->domain_id = element_id;
    ctx->config = config;
    ctx->mgi = (struct smcf_mgi_reg *)(config->reg_base);

    if (sub_element_count != mgi_get_num_of_monitors(ctx->mgi)) {
        return FWK_E_PARAM;
    }
    ctx->monitor_count = sub_element_count;

#ifdef BUILD_HAS_MOD_TIMER
    if (config->timer_config == NULL) {
        ctx->timer_ctx = NULL;
    } else {
        ctx->timer_ctx = fwk_mm_calloc(1, sizeof(struct smcf_mgi_timer_ctx));
        if (ctx->timer_ctx == NULL) {
            return FWK_E_NOMEM;
        }
        /* Check for valid timeout value if timer ID is specified */
        if (config->timer_config->set_state_timeout_us == 0) {
            return FWK_E_PARAM;
        }
        /* Save the timer ID to context */
        ctx->timer_ctx->timer_id = config->timer_config->timer_id;
        ctx->timer_ctx->delay_us = config->timer_config->set_state_timeout_us;
    }
#else
    ctx->timer_ctx = NULL;
#endif

    status = smcf_element_init_config_sample_type(ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    smcf_element_init_set_data_attributes(ctx);

    smcf_element_init_setup_interrupt(ctx);

    return mgi_enable_all_monitor(ctx->mgi);
}

static int smcf_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    struct smcf_element_ctx *element_ctx;

    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_SMCF_API_IDX_DATA:
        *api = &data_api;
        break;

    case MOD_SMCF_API_IDX_CONTROL:
        *api = &control_api;
        break;

    case MOD_SMCF_API_IDX_INTERRUPT:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
            return FWK_E_ACCESS;
        }

        element_ctx = get_domain_ctx(target_id);
        smcf_enable_interrupt(element_ctx);
        *api = &smcf_interrupt_api;
        break;

    default:
        return FWK_E_RANGE;
    }

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_NOTIFICATION
static int smcf_new_data_sample_ready_notify(void)
{
    unsigned int subscribers_count;
    struct fwk_event new_data_event = {
        .id = mod_smcf_notification_id_new_data_sample_ready,
        .response_requested = false,
        .source_id = FWK_ID_NONE
    };

    return fwk_notification_notify(&new_data_event, &subscribers_count);
}
#endif

static int smcf_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct smcf_element_ctx *ctx = get_domain_ctx(event->target_id);
    int status = FWK_SUCCESS;
    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    /*
     * local SMCF event
     */
    if (fwk_id_is_equal(event->id, smcf_event_id_new_data_sample)) {
        FWK_TRACE("[SMCF] New data sample event received");
#ifdef BUILD_HAS_NOTIFICATION
        status = smcf_new_data_sample_ready_notify();
#endif
    }

    return status;
}

#ifdef BUILD_HAS_MOD_TIMER
static int smcf_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_SUCCESS;
    struct smcf_element_ctx *ctx = get_domain_ctx(id);

    /* Only bind in first round of calls. */
    if (round > 0) {
        return FWK_SUCCESS;
    }

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    if (ctx->timer_ctx != NULL &&
        !fwk_id_is_equal(ctx->timer_ctx->timer_id, FWK_ID_NONE)) {
        /* Bind to the timer */
        status = fwk_module_bind(
            ctx->timer_ctx->timer_id,
            MOD_TIMER_API_ID_TIMER,
            &ctx->timer_ctx->timer_api);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return status;
}
#endif

const struct fwk_module module_smcf = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = (unsigned int)MOD_SMCF_API_IDX_COUNT,
    .event_count = (unsigned int)SMCF_EVENT_COUNT,
#ifdef BUILD_HAS_NOTIFICATION
    .notification_count = (unsigned int)MOD_SMCF_NOTIFY_IDX_COUNT,
#endif
    .init = smcf_mod_init,
    .element_init = smcf_element_init,
    .process_bind_request = smcf_process_bind_request,
    .process_event = smcf_process_event,
#ifdef BUILD_HAS_MOD_TIMER
    .bind = smcf_bind,
#endif
};
