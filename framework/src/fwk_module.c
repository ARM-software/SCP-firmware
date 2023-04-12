/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Module facilities.
 */

#include <internal/fwk_core.h>
#include <internal/fwk_id.h>
#include <internal/fwk_module.h>

#include <fwk_assert.h>
#include <fwk_cli_dbg.h>
#include <fwk_dlist.h>
#include <fwk_list.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>

#if FMW_NOTIFICATION_MAX > 64
#    define FWK_MODULE_EVENT_COUNT FMW_NOTIFICATION_MAX
#else
#    define FWK_MODULE_EVENT_COUNT 64
#endif

#define FWK_MODULE_BIND_ROUND_MAX 1

/* Pre-runtime phase stages */
enum fwk_module_stage {
    MODULE_STAGE_INITIALIZE,
    MODULE_STAGE_BIND,
    MODULE_STAGE_START,
    MODULE_STAGE_STOP
};

static struct {
    /* Flag indicating whether all modules have been initialized */
    bool initialized;

    /* Table of module contexts */
    struct fwk_module_context module_ctx_table[FWK_MODULE_IDX_COUNT];

    /* Pre-runtime phase stage */
    enum fwk_module_stage stage;

    /*
     * Identifier of module or element currently binding to other modules or
     * elements as part as of the binding stage.
     */
    fwk_id_t bind_id;
} fwk_module_ctx;

extern const struct fwk_module *module_table[FWK_MODULE_IDX_COUNT];
extern const struct fwk_module_config
    *module_config_table[FWK_MODULE_IDX_COUNT];

#if (FWK_LOG_LEVEL < FWK_LOG_LEVEL_DISABLED)
static const char fwk_module_err_msg_line[] = "[MOD] Error %d in %s @%d";
static const char fwk_module_err_msg_func[] = "[MOD] Error %d in %s";
#endif

static size_t fwk_module_count_elements(const struct fwk_element *elements)
{
    size_t count = 0;

    for (; elements[count].name != NULL; count++) {
        continue;
    }

    return count;
}

#ifdef BUILD_HAS_NOTIFICATION
static void fwk_module_init_subscriptions(struct fwk_dlist **list, size_t count)
{
    *list = fwk_mm_calloc(count, sizeof((*list)[0]));
    if (*list == NULL) {
        fwk_trap();
    }

    for (size_t i = 0; i < count; i++) {
        fwk_list_init(&((*list)[i]));
    }
}
#endif

static void fwk_module_init_element_ctx(
    struct fwk_element_ctx *ctx,
    const struct fwk_element *element,
    size_t notification_count)
{
    *ctx = (struct fwk_element_ctx){
        .state = FWK_MODULE_STATE_UNINITIALIZED,
        .desc = element,
        .sub_element_count = element->sub_element_count,
    };

    fwk_list_init(&ctx->delayed_response_list);

#ifdef BUILD_HAS_NOTIFICATION
    if (notification_count > 0) {
        fwk_module_init_subscriptions(
            &ctx->subscription_dlist_table, notification_count);
    }
#endif
}

static void fwk_module_init_element_ctxs(
    struct fwk_module_context *ctx,
    const struct fwk_element *elements,
    size_t notification_count)
{
    ctx->element_count = fwk_module_count_elements(elements);

    ctx->element_ctx_table =
        fwk_mm_calloc(ctx->element_count, sizeof(ctx->element_ctx_table[0]));
    if (ctx->element_ctx_table == NULL) {
        fwk_trap();
    }

    for (size_t i = 0; i < ctx->element_count; i++) {
        fwk_module_init_element_ctx(
            &ctx->element_ctx_table[i], &elements[i], notification_count);
    }
}

void fwk_module_init(void)
{
    /*
     * The loop index i gets the values corresponding to the
     * enum fwk_module_idx
     */
    for (uint32_t i = 0U; i < (uint32_t)FWK_MODULE_IDX_COUNT; i++) {
        struct fwk_module_context *ctx = &fwk_module_ctx.module_ctx_table[i];

        fwk_id_t id = FWK_ID_MODULE(i);

        const struct fwk_module *desc = module_table[i];
        const struct fwk_module_config *config = module_config_table[i];

        *ctx = (struct fwk_module_context){
            .id = id,

            .desc = desc,
            .config = config,
        };

        fwk_assert(ctx->desc != NULL);
        fwk_assert(ctx->config != NULL);

        fwk_list_init(&ctx->delayed_response_list);

        if (config->elements.type == FWK_MODULE_ELEMENTS_TYPE_STATIC) {
            size_t notification_count = 0;

#ifdef BUILD_HAS_NOTIFICATION
            notification_count = desc->notification_count;
#endif

            fwk_module_init_element_ctxs(
                ctx, config->elements.table, notification_count);
        }

#ifdef BUILD_HAS_NOTIFICATION
        if (desc->notification_count > 0) {
            fwk_module_init_subscriptions(
                &ctx->subscription_dlist_table, desc->notification_count);
        }
#endif
    }
}

static void fwk_module_init_elements(struct fwk_module_context *ctx)
{
    int status;

    const struct fwk_module *desc = ctx->desc;

    if (desc->element_init == NULL) {
        fwk_trap();
    }

    for (size_t i = 0; i < ctx->element_count; i++) {
        fwk_id_t element_id = fwk_id_build_element_id(ctx->id, (unsigned int)i);

        const struct fwk_element *element = ctx->element_ctx_table[i].desc;

        fwk_module_ctx.bind_id = element_id;

        /* Each element must have a valid pointer to specific data */
        if (element->data == NULL) {
            fwk_trap();
        }

        status = desc->element_init(
            element_id, element->sub_element_count, element->data);
        if (status != FWK_SUCCESS) {
            fwk_trap();
        }

        ctx->state = FWK_MODULE_STATE_INITIALIZED;
    }
}

static void fwk_module_init_module(struct fwk_module_context *ctx)
{
    int status;

    const struct fwk_module *desc = ctx->desc;
    const struct fwk_module_config *config = ctx->config;

    if (desc->type >= FWK_MODULE_TYPE_COUNT) {
        fwk_trap();
    }

    if (desc->init == NULL) {
        fwk_trap();
    }

    if ((desc->api_count == 0) != (desc->process_bind_request == NULL)) {
        fwk_trap();
    }

    if (config->elements.type == FWK_MODULE_ELEMENTS_TYPE_DYNAMIC) {
        size_t notification_count = 0;

        const struct fwk_element *elements = NULL;

        if (config->elements.generator == NULL) {
            fwk_trap();
        }

        elements = config->elements.generator(ctx->id);
        if (elements == NULL) {
            fwk_trap();
        }

#ifdef BUILD_HAS_NOTIFICATION
        notification_count = desc->notification_count;
#endif

        fwk_module_init_element_ctxs(ctx, elements, notification_count);
    }

    status = desc->init(ctx->id, ctx->element_count, config->data);
    if (status != FWK_SUCCESS) {
        fwk_trap();
    }

    if (ctx->element_count > 0) {
        fwk_module_init_elements(ctx);
    }

    if (desc->post_init != NULL) {
        status = desc->post_init(ctx->id);
        if (status != FWK_SUCCESS) {
            fwk_trap();
        }
    }

    ctx->state = FWK_MODULE_STATE_INITIALIZED;
}

static void fwk_module_init_modules(void)
{
    for (unsigned int i = 0U; i < (unsigned int)FWK_MODULE_IDX_COUNT; i++) {
        fwk_module_init_module(&fwk_module_ctx.module_ctx_table[i]);
    }
}

static int fwk_module_bind_elements(
    struct fwk_module_context *fwk_mod_ctx,
    unsigned int round)
{
    int status;
    const struct fwk_module *module;
    unsigned int element_idx;

    module = fwk_mod_ctx->desc;

    for (element_idx = 0; element_idx < fwk_mod_ctx->element_count;
         element_idx++) {
        fwk_module_ctx.bind_id =
            fwk_id_build_element_id(fwk_mod_ctx->id, element_idx);
        status = module->bind(fwk_module_ctx.bind_id, round);
        if (!fwk_expect(status == FWK_SUCCESS)) {
            FWK_LOG_CRIT(fwk_module_err_msg_func, status, __func__);
            return status;
        }

        if (round == FWK_MODULE_BIND_ROUND_MAX) {
            fwk_mod_ctx->element_ctx_table[element_idx].state =
                FWK_MODULE_STATE_BOUND;
        }
    }

    return FWK_SUCCESS;
}

static int fwk_module_bind_module(
    struct fwk_module_context *fwk_mod_ctx,
    unsigned int round)
{
    int status;
    const struct fwk_module *module;

    module = fwk_mod_ctx->desc;
    if (module->bind == NULL) {
        fwk_mod_ctx->state = FWK_MODULE_STATE_BOUND;
        return FWK_SUCCESS;
    }

    fwk_module_ctx.bind_id = fwk_mod_ctx->id;
    status = module->bind(fwk_mod_ctx->id, round);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        FWK_LOG_CRIT(fwk_module_err_msg_func, status, __func__);
        return status;
    }

    if (round == FWK_MODULE_BIND_ROUND_MAX) {
        fwk_mod_ctx->state = FWK_MODULE_STATE_BOUND;
    }

    return fwk_module_bind_elements(fwk_mod_ctx, round);
}

static int fwk_module_bind_modules(unsigned int round)
{
    int status;
    unsigned int module_idx;
    struct fwk_module_context *fwk_mod_ctx;

    for (module_idx = 0; module_idx < FWK_MODULE_IDX_COUNT; module_idx++) {
        fwk_mod_ctx = &fwk_module_ctx.module_ctx_table[module_idx];
        status = fwk_module_bind_module(fwk_mod_ctx, round);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

static int fwk_module_start_elements(struct fwk_module_context *fwk_mod_ctx)
{
    int status;
    const struct fwk_module *module;
    unsigned int element_idx;

    module = fwk_mod_ctx->desc;
    for (element_idx = 0; element_idx < fwk_mod_ctx->element_count;
         element_idx++) {
        if (module->start != NULL) {
            status = module->start(
                fwk_id_build_element_id(fwk_mod_ctx->id, element_idx));
            if (!fwk_expect(status == FWK_SUCCESS)) {
                FWK_LOG_CRIT(fwk_module_err_msg_func, status, __func__);
                return status;
            }
        }

        fwk_mod_ctx->element_ctx_table[element_idx].state =
            FWK_MODULE_STATE_STARTED;
    }

    return FWK_SUCCESS;
}

static int fwk_module_start_module(struct fwk_module_context *fwk_mod_ctx)
{
    int status;
    const struct fwk_module *module;

    module = fwk_mod_ctx->desc;

    if (module->start != NULL) {
        status = module->start(fwk_mod_ctx->id);
        if (!fwk_expect(status == FWK_SUCCESS)) {
            FWK_LOG_CRIT(fwk_module_err_msg_func, status, __func__);
            return status;
        }
    }

    fwk_mod_ctx->state = FWK_MODULE_STATE_STARTED;

    return fwk_module_start_elements(fwk_mod_ctx);
}

static int start_modules(void)
{
    int status;
    unsigned int module_idx;
    struct fwk_module_context *fwk_mod_ctx;

    for (module_idx = 0; module_idx < FWK_MODULE_IDX_COUNT; module_idx++) {
        fwk_mod_ctx = &fwk_module_ctx.module_ctx_table[module_idx];
        status = fwk_module_start_module(fwk_mod_ctx);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

int fwk_module_start(void)
{
    int status;
    unsigned int bind_round;

    if (fwk_module_ctx.initialized) {
        FWK_LOG_CRIT(fwk_module_err_msg_func, FWK_E_STATE, __func__);
        return FWK_E_STATE;
    }

    CLI_DEBUGGER();

    status = __fwk_init(FWK_MODULE_EVENT_COUNT);
    if (status != FWK_SUCCESS) {
        return status;
    }

    fwk_module_ctx.stage = MODULE_STAGE_INITIALIZE;
    fwk_module_init_modules();

    fwk_module_ctx.stage = MODULE_STAGE_BIND;
    for (bind_round = 0; bind_round <= FWK_MODULE_BIND_ROUND_MAX;
         bind_round++) {
        status = fwk_module_bind_modules(bind_round);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    fwk_module_ctx.stage = MODULE_STAGE_START;
    status = start_modules();
    if (status != FWK_SUCCESS) {
        return status;
    }

    fwk_module_ctx.initialized = true;

    FWK_LOG_CRIT("[FWK] Module initialization complete!");

    return FWK_SUCCESS;
}

static int fwk_module_stop_elements(struct fwk_module_context *fwk_mod_ctx)
{
    int status;
    const struct fwk_module *module;
    unsigned int element_idx;

    module = fwk_mod_ctx->desc;

    for (element_idx = 0; element_idx < fwk_mod_ctx->element_count;
         element_idx++) {
        if (module->stop != NULL) {
            status = module->stop(
                fwk_id_build_element_id(fwk_mod_ctx->id, element_idx));
            if (!fwk_expect(status == FWK_SUCCESS)) {
                FWK_LOG_CRIT(fwk_module_err_msg_func, status, __func__);
                return status;
            }
        }

        fwk_mod_ctx->element_ctx_table[element_idx].state =
            FWK_MODULE_STATE_SUSPENDED;
    }

    return FWK_SUCCESS;
}

static int fwk_module_stop_module(struct fwk_module_context *fwk_mod_ctx)
{
    int status;
    const struct fwk_module *module;

    module = fwk_mod_ctx->desc;

    if (module->stop != NULL) {
        status = module->stop(fwk_mod_ctx->id);
        if (!fwk_expect(status == FWK_SUCCESS)) {
            FWK_LOG_CRIT(fwk_module_err_msg_func, status, __func__);
            return status;
        }
    }

    fwk_mod_ctx->state = FWK_MODULE_STATE_SUSPENDED;

    return fwk_module_stop_elements(fwk_mod_ctx);
}

static int stop_modules(void)
{
    int status;
    unsigned int module_idx;
    struct fwk_module_context *fwk_mod_ctx;

    for (module_idx = 0; module_idx < FWK_MODULE_IDX_COUNT; module_idx++) {
        fwk_mod_ctx = &fwk_module_ctx.module_ctx_table[module_idx];
        status = fwk_module_stop_module(fwk_mod_ctx);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

int fwk_module_stop(void)
{
    int status;

    if (!fwk_module_ctx.initialized) {
        FWK_LOG_CRIT(fwk_module_err_msg_func, FWK_E_STATE, __func__);
        return FWK_E_STATE;
    }

    fwk_module_ctx.stage = MODULE_STAGE_STOP;
    status = stop_modules();
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

struct fwk_module_context *fwk_module_get_ctx(fwk_id_t id)
{
    return &fwk_module_ctx.module_ctx_table[fwk_id_get_module_idx(id)];
}

struct fwk_element_ctx *fwk_module_get_element_ctx(fwk_id_t element_id)
{
    struct fwk_module_context *fwk_mod_ctx = fwk_module_get_ctx(element_id);

    return &fwk_mod_ctx->element_ctx_table[element_id.element.element_idx];
}

int fwk_module_get_state(fwk_id_t id, enum fwk_module_state *state)
{
    if (state == NULL) {
        return FWK_E_PARAM;
    }

    if (fwk_module_is_valid_element_id(id) ||
        fwk_module_is_valid_sub_element_id(id)) {
        *state = fwk_module_get_element_ctx(id)->state;
    } else {
        if (fwk_module_is_valid_module_id(id)) {
            *state = fwk_module_get_ctx(id)->state;
        } else {
            return FWK_E_PARAM;
        }
    }

    return FWK_SUCCESS;
}

void fwk_module_reset(void)
{
    fwk_module_init();
}

bool fwk_module_is_valid_module_id(fwk_id_t id)
{
    if (!fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return false;
    }

    if (fwk_id_get_module_idx(id) >= FWK_MODULE_IDX_COUNT) {
        return false;
    }

    return true;
}

bool fwk_module_is_valid_element_id(fwk_id_t id)
{
    unsigned int module_idx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return false;
    }

    module_idx = fwk_id_get_module_idx(id);
    if (module_idx >= FWK_MODULE_IDX_COUNT) {
        return false;
    }

    return (
        fwk_id_get_element_idx(id) <
        fwk_module_ctx.module_ctx_table[module_idx].element_count);
}

bool fwk_module_is_valid_sub_element_id(fwk_id_t id)
{
    unsigned int module_idx;
    struct fwk_module_context *fwk_mod_ctx;
    unsigned int element_idx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_SUB_ELEMENT)) {
        return false;
    }

    module_idx = fwk_id_get_module_idx(id);
    if (module_idx >= FWK_MODULE_IDX_COUNT) {
        return false;
    }
    fwk_mod_ctx = &fwk_module_ctx.module_ctx_table[module_idx];

    element_idx = fwk_id_get_element_idx(id);
    if (element_idx >= fwk_mod_ctx->element_count) {
        return false;
    }

    return (
        fwk_id_get_sub_element_idx(id) <
        fwk_mod_ctx->element_ctx_table[element_idx].sub_element_count);
}

bool fwk_module_is_valid_entity_id(fwk_id_t id)
{
    bool return_status = false;

    switch (fwk_id_get_type(id)) {
    case FWK_ID_TYPE_MODULE:
        return_status = fwk_module_is_valid_module_id(id);
        break;

    case FWK_ID_TYPE_ELEMENT:
        return_status = fwk_module_is_valid_element_id(id);
        break;

    case FWK_ID_TYPE_SUB_ELEMENT:
        return_status = fwk_module_is_valid_sub_element_id(id);
        break;

    default:
        break;
    }

    return return_status;
}

bool fwk_module_is_valid_api_id(fwk_id_t id)
{
    unsigned int module_idx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_API)) {
        return false;
    }

    module_idx = fwk_id_get_module_idx(id);
    if (module_idx >= FWK_MODULE_IDX_COUNT) {
        return false;
    }

    return (
        fwk_id_get_api_idx(id) <
        fwk_module_ctx.module_ctx_table[module_idx].desc->api_count);
}

bool fwk_module_is_valid_event_id(fwk_id_t id)
{
    unsigned int module_idx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_EVENT)) {
        return false;
    }

    module_idx = fwk_id_get_module_idx(id);
    if (module_idx >= FWK_MODULE_IDX_COUNT) {
        return false;
    }

    return (
        fwk_id_get_event_idx(id) <
        fwk_module_ctx.module_ctx_table[module_idx].desc->event_count);
}

bool fwk_module_is_valid_notification_id(fwk_id_t id)
{
#ifdef BUILD_HAS_NOTIFICATION
    unsigned int module_idx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_NOTIFICATION)) {
        return false;
    }

    module_idx = fwk_id_get_module_idx(id);
    if (module_idx >= FWK_MODULE_IDX_COUNT) {
        return false;
    }

    return (
        fwk_id_get_notification_idx(id) <
        fwk_module_ctx.module_ctx_table[module_idx].desc->notification_count);
#else
    return false;
#endif
}

int fwk_module_get_element_count(fwk_id_t id)
{
    if (fwk_module_is_valid_module_id(id)) {
        return (int)fwk_module_get_ctx(id)->element_count;
    } else {
        return FWK_E_PARAM;
    }
}

int fwk_module_get_sub_element_count(fwk_id_t element_id)
{
    if (fwk_module_is_valid_element_id(element_id)) {
        return (int)fwk_module_get_element_ctx(element_id)->sub_element_count;
    } else {
        return FWK_E_PARAM;
    }
}

const char *fwk_module_get_element_name(fwk_id_t id)
{
    if (fwk_module_is_valid_element_id(id)) {
        return fwk_module_get_element_ctx(id)->desc->name;
    }

    fwk_unexpected();
    return NULL;
}

const void *fwk_module_get_data(fwk_id_t id)
{
    if (fwk_module_is_valid_element_id(id) ||
        fwk_module_is_valid_sub_element_id(id)) {
        return fwk_module_get_element_ctx(id)->desc->data;
    } else if (fwk_module_is_valid_module_id(id)) {
        return fwk_module_get_ctx(id)->config->data;
    }

    fwk_unexpected();
    return NULL;
}

int fwk_module_bind(fwk_id_t target_id, fwk_id_t api_id, const void *api)
{
    int status = FWK_E_PARAM;
    struct fwk_module_context *fwk_mod_ctx;

    if (!fwk_module_is_valid_entity_id(target_id)) {
        goto error;
    }

    if (!fwk_module_is_valid_api_id(api_id)) {
        goto error;
    }

    if (fwk_id_get_module_idx(target_id) != fwk_id_get_module_idx(api_id)) {
        goto error;
    }

    if (api == NULL) {
        goto error;
    }

    fwk_mod_ctx = fwk_module_get_ctx(target_id);

    if (((fwk_module_ctx.stage != MODULE_STAGE_INITIALIZE) ||
         (fwk_mod_ctx->state != FWK_MODULE_STATE_INITIALIZED)) &&
        (fwk_module_ctx.stage != MODULE_STAGE_BIND)) {
        status = FWK_E_STATE;
        goto error;
    }

    status = fwk_mod_ctx->desc->process_bind_request(
        fwk_module_ctx.bind_id, target_id, api_id, (const void **)api);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        FWK_LOG_CRIT(fwk_module_err_msg_line, status, __func__, __LINE__);
        return status;
    }

    if (*(void **)api == NULL) {
        status = FWK_E_HANDLER;
        goto error;
    }

    return FWK_SUCCESS;

error:
    fwk_check((bool)false);
    FWK_LOG_CRIT(fwk_module_err_msg_func, status, __func__);
    return status;
}

int fwk_module_adapter(const struct fwk_io_adapter **adapter, fwk_id_t id)
{
    unsigned int idx;

    if (adapter == NULL) {
        return FWK_E_PARAM;
    }

    *adapter = NULL;

    if (!fwk_module_is_valid_entity_id(id)) {
        return FWK_E_PARAM;
    }

    idx = fwk_id_get_module_idx(id);

    *adapter = &module_table[idx]->adapter;

    return FWK_SUCCESS;
}
