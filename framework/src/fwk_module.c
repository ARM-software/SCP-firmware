/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Module facilities.
 */

#include <internal/fwk_id.h>
#include <internal/fwk_module.h>
#include <internal/fwk_thread.h>

#ifdef BUILD_HAS_NOTIFICATION
#    include <internal/fwk_notification.h>
#endif

#include <fwk_assert.h>
#include <fwk_cli_dbg.h>
#include <fwk_dlist.h>
#include <fwk_element.h>
#include <fwk_list.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>

#define EVENT_COUNT 64
#define BIND_ROUND_MAX 1

#ifndef NOTIFICATION_COUNT
#define NOTIFICATION_COUNT 64
#endif

/* Pre-runtime phase stages */
enum module_stage {
    MODULE_STAGE_INITIALIZE,
    MODULE_STAGE_BIND,
    MODULE_STAGE_START
};

struct context {
    /* Flag indicating whether all modules have been initialized */
    bool initialized;

    /* Number of modules */
    unsigned int module_count;

    /* Table of module contexts */
    struct fwk_module_ctx *module_ctx_table;

    /* Pre-runtime phase stage */
    enum module_stage stage;

    /*
     * Identifier of module or element currently binding to other modules or
     * elements as part as of the binding stage.
     */
    fwk_id_t bind_id;
};

extern const struct fwk_module *module_table[];
extern const struct fwk_module_config *module_config_table[];

static struct context ctx;

static const char err_msg_line[] = "[MOD] Error %d in %s @%d";
static const char err_msg_func[] = "[MOD] Error %d in %s";

/*
 * Static functions
 */

#ifdef BUILD_HAS_NOTIFICATION
static int init_notification_dlist_table(size_t count,
    struct fwk_dlist **notification_dlist_table)
{
    struct fwk_dlist *dlist_table;
    unsigned int dlist_idx;

    dlist_table = fwk_mm_calloc(count, sizeof(struct fwk_dlist));
    *notification_dlist_table = dlist_table;

    for (dlist_idx = 0; dlist_idx < count; dlist_idx++)
        fwk_list_init(&dlist_table[dlist_idx]);

    return FWK_SUCCESS;
}
#endif

static int init_elements(struct fwk_module_ctx *module_ctx,
                         const struct fwk_element *element_table)
{
    int status;
    const struct fwk_module *module;
    unsigned int element_idx;
    fwk_id_t element_id;
    struct fwk_element_ctx *element_ctx;
    const struct fwk_element *element;

    module = module_ctx->desc;
    if (!fwk_expect(module->element_init != NULL))
        return FWK_E_PARAM;

    module_ctx->element_ctx_table =
        fwk_mm_calloc(module_ctx->element_count,
                      sizeof(struct fwk_element_ctx));

    for (element_idx = 0; element_idx < module_ctx->element_count;
         element_idx++) {

        element_ctx = &module_ctx->element_ctx_table[element_idx];
        element = &element_table[element_idx];
        element_id = fwk_id_build_element_id(module_ctx->id, element_idx);
        ctx.bind_id = element_id;

        /* Each element must have a valid pointer to specific data */
        if (!fwk_expect(element->data != NULL)) {
            FWK_LOG_CRIT(err_msg_line, FWK_E_DATA, __func__, __LINE__);
            return FWK_E_DATA;
        }

        element_ctx->desc = element;
        element_ctx->sub_element_count = element->sub_element_count;
        fwk_list_init(&element_ctx->delayed_response_list);

        #ifdef BUILD_HAS_NOTIFICATION
        if (module->notification_count) {
            status = init_notification_dlist_table(module->notification_count,
                &element_ctx->subscription_dlist_table);
            if (!fwk_expect(status == FWK_SUCCESS))
                return status;
        }
        #endif

        status = module->element_init(
            element_id, element->sub_element_count, element->data);
        if (!fwk_expect(status == FWK_SUCCESS)) {
            FWK_LOG_CRIT(err_msg_func, status, __func__);
            return status;
        }

        element_ctx->state = FWK_MODULE_STATE_INITIALIZED;
    }

    return FWK_SUCCESS;
}

static int init_module(struct fwk_module_ctx *module_ctx,
                       const struct fwk_module *module,
                       const struct fwk_module_config *module_config)
{
    int status;
    const struct fwk_element *element_table = NULL;
    unsigned int count;

    if ((module->name == NULL) ||
        (module->type >= FWK_MODULE_TYPE_COUNT) ||
        (module->init == NULL) ||
        (module_config == NULL) ||
        ((module->api_count > 0) && (module->process_bind_request == NULL))) {
        fwk_expect(false);
        return FWK_E_PARAM;
    }

    module_ctx->desc = module;
    module_ctx->config = module_config;
    fwk_list_init(&module_ctx->delayed_response_list);
    ctx.bind_id = module_ctx->id;

    #ifdef BUILD_HAS_NOTIFICATION
    if (module->notification_count) {
        status = init_notification_dlist_table(module->notification_count,
            &module_ctx->subscription_dlist_table);
        if (!fwk_expect(status == FWK_SUCCESS))
            return status;
    }
    #endif

    if (module_config->get_element_table != NULL) {
        element_table = module_config->get_element_table(module_ctx->id);
        if (!fwk_expect(element_table != NULL))
            return FWK_E_PARAM;

        for (count = 0; element_table[count].name != NULL; count++)
            continue;

        module_ctx->element_count = count;
    }

    status = module->init(module_ctx->id, module_ctx->element_count,
                          module_config->data);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        FWK_LOG_CRIT(err_msg_line, status, __func__, __LINE__);
        return status;
    }

    if (module_ctx->element_count > 0) {
        status = init_elements(module_ctx, element_table);
        if (status != FWK_SUCCESS) {
            FWK_LOG_CRIT(err_msg_line, status, __func__, __LINE__);
            return status;
        }
    }

    if (module->post_init != NULL) {
        status = module->post_init(module_ctx->id);
        if (!fwk_expect(status == FWK_SUCCESS)) {
            FWK_LOG_CRIT(err_msg_line, status, __func__, __LINE__);
            return status;
        }
    }

    module_ctx->state = FWK_MODULE_STATE_INITIALIZED;

    return FWK_SUCCESS;
}

static int init_modules(void)
{
    int status;
    unsigned int module_idx;
    struct fwk_module_ctx *module_ctx;

    while (module_table[ctx.module_count] != NULL)
        ctx.module_count++;

    ctx.module_ctx_table = fwk_mm_calloc(ctx.module_count,
                                         sizeof(struct fwk_module_ctx));

    for (module_idx = 0; module_idx < ctx.module_count; module_idx++) {
        module_ctx = &ctx.module_ctx_table[module_idx];
        module_ctx->id = FWK_ID_MODULE(module_idx);
        status = init_module(module_ctx, module_table[module_idx],
                             module_config_table[module_idx]);
        if (status != FWK_SUCCESS) {
            FWK_LOG_CRIT(err_msg_line, status, __func__, __LINE__);
            return status;
        }
    }

    return FWK_SUCCESS;
}

static int bind_elements(struct fwk_module_ctx *module_ctx,
                         unsigned int round)
{
    int status;
    const struct fwk_module *module;
    unsigned int element_idx;

    module = module_ctx->desc;

    for (element_idx = 0; element_idx < module_ctx->element_count;
         element_idx++) {

        ctx.bind_id = fwk_id_build_element_id(module_ctx->id, element_idx);
        status = module->bind(ctx.bind_id, round);
        if (!fwk_expect(status == FWK_SUCCESS)) {
            FWK_LOG_CRIT(err_msg_func, status, __func__);
            return status;
        }

        if (round == BIND_ROUND_MAX) {
            module_ctx->element_ctx_table[element_idx].state =
                FWK_MODULE_STATE_BOUND;
        }
    }

    return FWK_SUCCESS;
}

static int bind_module(struct fwk_module_ctx *module_ctx,
                       unsigned int round)
{
    int status;
    const struct fwk_module *module;

    module = module_ctx->desc;
    if (module->bind == NULL) {
        module_ctx->state = FWK_MODULE_STATE_BOUND;
        return FWK_SUCCESS;
    }

    ctx.bind_id = module_ctx->id;
    status = module->bind(module_ctx->id, round);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        FWK_LOG_CRIT(err_msg_func, status, __func__);
        return status;
    }

    if (round == BIND_ROUND_MAX)
        module_ctx->state = FWK_MODULE_STATE_BOUND;

    return bind_elements(module_ctx, round);
}

static int bind_modules(unsigned int round)
{
    int status;
    unsigned int module_idx;
    struct fwk_module_ctx *module_ctx;

    for (module_idx = 0; module_idx < ctx.module_count; module_idx++) {
        module_ctx = &ctx.module_ctx_table[module_idx];
        status = bind_module(module_ctx, round);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int start_elements(struct fwk_module_ctx *module_ctx)
{
    int status;
    const struct fwk_module *module;
    unsigned int element_idx;

    module = module_ctx->desc;
    for (element_idx = 0; element_idx < module_ctx->element_count;
         element_idx++) {

        if (module->start != NULL) {
            status = module->start(
                fwk_id_build_element_id(module_ctx->id, element_idx));
            if (!fwk_expect(status == FWK_SUCCESS)) {
                FWK_LOG_CRIT(err_msg_func, status, __func__);
                return status;
            }
        }

        module_ctx->element_ctx_table[element_idx].state =
            FWK_MODULE_STATE_STARTED;
    }

    return FWK_SUCCESS;
}

static int start_module(struct fwk_module_ctx *module_ctx)
{
    int status;
    const struct fwk_module *module;

    module = module_ctx->desc;

    if (module->start != NULL) {
        status = module->start(module_ctx->id);
        if (!fwk_expect(status == FWK_SUCCESS)) {
            FWK_LOG_CRIT(err_msg_func, status, __func__);
            return status;
        }
    }

    module_ctx->state = FWK_MODULE_STATE_STARTED;

    return start_elements(module_ctx);
}

static int start_modules(void)
{
    int status;
    unsigned int module_idx;
    struct fwk_module_ctx *module_ctx;

    for (module_idx = 0; module_idx < ctx.module_count; module_idx++) {
        module_ctx = &ctx.module_ctx_table[module_idx];
        status = start_module(module_ctx);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

/*
 * Private interface functions
 */

int __fwk_module_init(void)
{
    int status;
    unsigned int bind_round;

    if (ctx.initialized) {
        FWK_LOG_CRIT(err_msg_func, FWK_E_STATE, __func__);
        return FWK_E_STATE;
    }

    CLI_DEBUGGER();

    status = __fwk_thread_init(EVENT_COUNT);
    if (status != FWK_SUCCESS)
        return status;

    ctx.stage = MODULE_STAGE_INITIALIZE;
    status = init_modules();
    if (status != FWK_SUCCESS)
        return status;

    ctx.stage = MODULE_STAGE_BIND;
    for (bind_round = 0; bind_round <= BIND_ROUND_MAX; bind_round++) {
        status = bind_modules(bind_round);
        if (status != FWK_SUCCESS)
            return status;
    }

    #ifdef BUILD_HAS_NOTIFICATION
    status = __fwk_notification_init(NOTIFICATION_COUNT);
    if (status != FWK_SUCCESS)
        return status;
    #endif

    ctx.stage = MODULE_STAGE_START;
    status = start_modules();
    if (status != FWK_SUCCESS)
        return status;

    ctx.initialized = true;

    __fwk_thread_run();

    return FWK_SUCCESS;
}

struct fwk_module_ctx *__fwk_module_get_ctx(fwk_id_t id)
{
    return &ctx.module_ctx_table[fwk_id_get_module_idx(id)];
}

struct fwk_element_ctx *__fwk_module_get_element_ctx(fwk_id_t element_id)
{
    struct fwk_module_ctx *module_ctx = __fwk_module_get_ctx(element_id);

    return &module_ctx->element_ctx_table[element_id.element.element_idx];
}

int __fwk_module_get_state(fwk_id_t id, enum fwk_module_state *state)
{
    if (state == NULL)
        return FWK_E_PARAM;

    if (fwk_module_is_valid_element_id(id) ||
        fwk_module_is_valid_sub_element_id(id))
        *state = __fwk_module_get_element_ctx(id)->state;
    else {
        if (fwk_module_is_valid_module_id(id))
            *state = __fwk_module_get_ctx(id)->state;
        else
            return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

void __fwk_module_reset(void)
{
    ctx = (struct context){ 0 };
}

/*
 * Public interface functions
 */

bool fwk_module_is_valid_module_id(fwk_id_t id)
{
    if (!fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return false;

    if (fwk_id_get_module_idx(id) >= ctx.module_count)
        return false;

    return true;
}

bool fwk_module_is_valid_element_id(fwk_id_t id)
{
    unsigned int module_idx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return false;

    module_idx = fwk_id_get_module_idx(id);
    if (module_idx >= ctx.module_count)
        return false;

    return (fwk_id_get_element_idx(id) <
            ctx.module_ctx_table[module_idx].element_count);
}

bool fwk_module_is_valid_sub_element_id(fwk_id_t id)
{
    unsigned int module_idx;
    struct fwk_module_ctx *module_ctx;
    unsigned int element_idx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_SUB_ELEMENT))
        return false;

    module_idx = fwk_id_get_module_idx(id);
    if (module_idx >= ctx.module_count)
        return false;
    module_ctx = &ctx.module_ctx_table[module_idx];

    element_idx = fwk_id_get_element_idx(id);
    if (element_idx >= module_ctx->element_count)
        return false;

    return (fwk_id_get_sub_element_idx(id) <
            module_ctx->element_ctx_table[element_idx].sub_element_count);
}

bool fwk_module_is_valid_entity_id(fwk_id_t id)
{
    switch (fwk_id_get_type(id)) {
    case FWK_ID_TYPE_MODULE:
        return fwk_module_is_valid_module_id(id);

    case FWK_ID_TYPE_ELEMENT:
        return fwk_module_is_valid_element_id(id);

    case FWK_ID_TYPE_SUB_ELEMENT:
        return fwk_module_is_valid_sub_element_id(id);

    default:
        break;
    }

    return false;
}

bool fwk_module_is_valid_api_id(fwk_id_t id)
{
    unsigned int module_idx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_API))
        return false;

    module_idx = fwk_id_get_module_idx(id);
    if (module_idx >= ctx.module_count)
        return false;

    return (fwk_id_get_api_idx(id) <
            ctx.module_ctx_table[module_idx].desc->api_count);
}

bool fwk_module_is_valid_event_id(fwk_id_t id)
{
    unsigned int module_idx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_EVENT))
        return false;

    module_idx = fwk_id_get_module_idx(id);
    if (module_idx >= ctx.module_count)
        return false;

    return (fwk_id_get_event_idx(id) <
            ctx.module_ctx_table[module_idx].desc->event_count);
}

bool fwk_module_is_valid_notification_id(fwk_id_t id)
{
    #ifdef BUILD_HAS_NOTIFICATION
    unsigned int module_idx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_NOTIFICATION))
        return false;

    module_idx = fwk_id_get_module_idx(id);
    if (module_idx >= ctx.module_count)
        return false;

    return (fwk_id_get_notification_idx(id) <
            ctx.module_ctx_table[module_idx].desc->notification_count);
    #else
    return false;
    #endif
}

int fwk_module_get_element_count(fwk_id_t id)
{
    if (fwk_module_is_valid_module_id(id))
        return __fwk_module_get_ctx(id)->element_count;
    else
        return FWK_E_PARAM;
}

int fwk_module_get_sub_element_count(fwk_id_t element_id)
{
    if (fwk_module_is_valid_element_id(element_id))
        return __fwk_module_get_element_ctx(element_id)->sub_element_count;
    else
        return FWK_E_PARAM;
}

const char *fwk_module_get_name(fwk_id_t id)
{
    if (fwk_module_is_valid_element_id(id))
        return __fwk_module_get_element_ctx(id)->desc->name;
    else if (fwk_module_is_valid_module_id(id))
        return __fwk_module_get_ctx(id)->desc->name;

    return NULL;
}

const void *fwk_module_get_data(fwk_id_t id)
{
    if (fwk_module_is_valid_element_id(id) ||
        fwk_module_is_valid_sub_element_id(id))
        return __fwk_module_get_element_ctx(id)->desc->data;
    else if (fwk_module_is_valid_module_id(id))
        return __fwk_module_get_ctx(id)->config->data;

    return NULL;
}

int fwk_module_bind(fwk_id_t target_id, fwk_id_t api_id, const void *api)
{
    int status = FWK_E_PARAM;
    struct fwk_module_ctx *module_ctx;

    if (!fwk_module_is_valid_entity_id(target_id))
        goto error;

    if (!fwk_module_is_valid_api_id(api_id))
        goto error;

    if (fwk_id_get_module_idx(target_id) !=
        fwk_id_get_module_idx(api_id))
        goto error;

    if (api == NULL)
        goto error;

    module_ctx = __fwk_module_get_ctx(target_id);

    if (((ctx.stage != MODULE_STAGE_INITIALIZE) ||
         (module_ctx->state != FWK_MODULE_STATE_INITIALIZED)) &&
        (ctx.stage != MODULE_STAGE_BIND)) {

        status = FWK_E_STATE;
        goto error;
    }

    status = module_ctx->desc->process_bind_request(ctx.bind_id, target_id,
                                                    api_id, (const void **)api);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        FWK_LOG_CRIT(err_msg_line, status, __func__, __LINE__);
        return status;
    }

    if (*(void **)api == NULL) {
        status = FWK_E_HANDLER;
        goto error;
    }

    return FWK_SUCCESS;

error:
    fwk_expect(false);
    FWK_LOG_CRIT(err_msg_func, status, __func__);
    return status;
}
