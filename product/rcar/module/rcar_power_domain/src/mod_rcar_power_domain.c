/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_status.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_thread.h>
#include <fwk_multi_thread.h>
#include <fwk_notification.h>
#include <mod_rcar_power_domain.h>
#include <config_rcar_power_domain.h>
#include <mmio.h>
#include <rcar_core.h>
#include <rcar_scmi_id.h>

/*
 * Internal variables
 */
static struct mod_pd_ctx mod_pd_ctx;

/*
 * Utility functions
 */

/* Functions related to power domain positions in the power domain tree */
static bool is_valid_tree_pos(uint64_t tree_pos)
{
    return (tree_pos < MOD_PD_TREE_POS(MOD_PD_LEVEL_COUNT, 0, 0, 0, 0));
}

static enum mod_pd_level get_level_from_tree_pos(uint64_t tree_pos)
{
    return (enum mod_pd_level)((tree_pos >> MOD_PD_TREE_POS_LEVEL_SHIFT) &
                              MOD_PD_TREE_POS_LEVEL_MASK);
}

/* State related utility functions */
static bool is_valid_state(const struct pd_ctx *pd, unsigned int state)
{
    return (state < MOD_PD_STATE_COUNT_MAX) &&
            ((pd->valid_state_mask & (1 << state)) != 0);
}

static unsigned int normalize_state(unsigned int state)
{
    switch (state) {
    case MOD_PD_STATE_OFF:
        return (MOD_PD_STATE_COUNT_MAX + 1);

    case MOD_PD_STATE_SLEEP:
        return MOD_PD_STATE_COUNT_MAX;

    default:
        return state;
    }
}

static bool is_deeper_state(unsigned int state,
                            unsigned int state_to_compare_to)
{
    return normalize_state(state) > normalize_state(state_to_compare_to);
}

static bool is_shallower_state(unsigned int state,
                               unsigned int state_to_compare_to)
{
    return normalize_state(state) < normalize_state(state_to_compare_to);
}

static bool is_allowed_by_child(const struct pd_ctx *child,
    unsigned int parent_state, unsigned int child_state)
{
    if (parent_state >= child->allowed_state_mask_table_size)
        return false;

    return ((child->allowed_state_mask_table[parent_state]
            & (1 << child_state)) != 0);
}

static const char *get_state_name(const struct pd_ctx *pd, unsigned int state)
{
    static char const unknown_name[] = "Unknown";

    if (state < pd->config->state_name_table_size)
        return pd->config->state_name_table[state];
    else if (state < FWK_ARRAY_SIZE(default_state_name_table))
        return default_state_name_table[state];
    else
        return unknown_name;
}

/* Functions related to a composite state */
static unsigned int get_level_state_from_composite_state(
    uint32_t composite_state, enum mod_pd_level level)
{
    return (composite_state >> mod_pd_cs_level_state_shift[level])
            & MOD_PD_CS_STATE_MASK;
}

static enum mod_pd_level get_highest_level_from_composite_state(
    uint32_t composite_state)
{
    return (enum mod_pd_level)((composite_state >> MOD_PD_CS_LEVEL_SHIFT) &
                              MOD_PD_CS_STATE_MASK);
}

static bool is_valid_composite_state(struct pd_ctx *target_pd,
                                     uint32_t composite_state)
{
    enum mod_pd_level level;
    enum mod_pd_level highest_level;
    unsigned int state, child_state = MOD_PD_STATE_OFF;
    struct pd_ctx *pd = target_pd;
    struct pd_ctx *child = NULL;

    assert(target_pd != NULL);

    if (composite_state & (~MOD_PD_CS_VALID_BITS))
        goto error;

    level = get_level_from_tree_pos(pd->config->tree_pos);
    highest_level = get_highest_level_from_composite_state(composite_state);

    if ((highest_level < level) ||
        (highest_level >= MOD_PD_LEVEL_COUNT))
        goto error;

    for (; level <= highest_level; level++) {
        if (pd == NULL)
            goto error;

        state = get_level_state_from_composite_state(composite_state, level);
        if (!is_valid_state(pd, state))
            goto error;

        if ((child != NULL) && !is_allowed_by_child(child, state, child_state))
            goto error;

        child = pd;
        child_state = state;
        pd = pd->parent;
    }

    return true;

error:
    FWK_LOG_ERR("[PD] ERROR: Invalid composite state for %s: 0x%08x.",
        fwk_module_get_name(target_pd->id), composite_state);

    return false;
}

/*
 * Check whether a transition to a given power state for a power domain is
 * possible given the current state of its parent and children (if any).
 *
 * \param pd Description of the power domain to check the power state transition
 *      for.
 * \param state Power state.
 */
static bool is_allowed_by_parent_and_children(struct pd_ctx *pd,
    unsigned int state)
{
    struct pd_ctx *parent, *child;

    parent = pd->parent;
    if (parent != NULL) {
        if (!is_allowed_by_child(pd, parent->current_state, state))
            return false;
    }

    child = pd->first_child;
    while (child != NULL) {
        if (!is_allowed_by_child(child, state, child->current_state))
            return false;
        child = child->sibling;
    }

    return true;
}

/*
 * Check whether a power state pre-transition notification must be sent.
 *
 * \param pd Description of the power domain
 * \param state Power state the power domain has to transit to
 *
 * \retval true A power state pre-transition notification must be sent.
 * \retval false A power state pre-transition notification doesn't have to be
 *      sent.
 */
static bool check_power_state_pre_transition_notification(struct pd_ctx *pd,
    unsigned int state)
{
    if (!is_deeper_state(state, pd->state_requested_to_driver))
        return false;

    if ((state == pd->power_state_pre_transition_notification_ctx.state) &&
        pd->power_state_pre_transition_notification_ctx.valid) {
        return (pd->power_state_pre_transition_notification_ctx.response_status
                != FWK_SUCCESS);
    }

    return true;
}

/*
 * Initiate a power state pre-transition notification if necessary.
 *
 * \param pd Description of the power domain to initiate the notification
 *      for.
 *
 * \retval true Waiting for notification responses.
 * \retval false Not waiting for any notification response.
 */
static bool initiate_power_state_pre_transition_notification(struct pd_ctx *pd)
{
    unsigned int state;
    struct fwk_event notification_event = {
        .id = mod_pd_notification_id_power_state_pre_transition,
        .response_requested = true
    };
    struct mod_pd_power_state_pre_transition_notification_params *params;

    state = pd->requested_state;
    if (!check_power_state_pre_transition_notification(pd, state))
        return false;

    /*
     * If still waiting for some responses on the previous power state
     * pre-transition notification, wait for them before to issue the next one.
     */
    if (pd->power_state_pre_transition_notification_ctx.pending_responses != 0)
        return true;

    params = (struct mod_pd_power_state_pre_transition_notification_params *)
        notification_event.params;
    params->current_state = pd->current_state;
    params->target_state = state;
    fwk_notification_notify(&notification_event,
        &pd->power_state_pre_transition_notification_ctx.pending_responses);

    pd->power_state_pre_transition_notification_ctx.state = state;
    pd->power_state_pre_transition_notification_ctx.response_status =
        FWK_SUCCESS;
    pd->power_state_pre_transition_notification_ctx.valid = true;

    return (pd->power_state_pre_transition_notification_ctx.pending_responses
            != 0);
}

/*
 * Initiate the transition to a power state for a power domain.
 *
 * \param pd Description of the power domain to initiate the state transition
 *      for.
 *
 * \retval FWK_SUCCESS The power state transition was initiated.
 * \retval FWK_E_DEVICE The power state transition was denied by the driver.
 * \return One of the other driver-defined error codes.
 */
static int initiate_power_state_transition(struct pd_ctx *pd)
{
    int status;
    unsigned int state = pd->requested_state;

    status = pd->driver_api->set_state(pd->driver_id, state);
    pd->current_state = state;

    FWK_LOG_TRACE("[PD] %s: %s->%s, %d.", fwk_module_get_name(pd->id),
        get_state_name(pd, pd->state_requested_to_driver),
        get_state_name(pd, state), status);

    pd->state_requested_to_driver = state;

    return status;
}

/*
 * Respond to a request.
 *
 * \param pd Description of the power domain in charge of the response
 * \param status Response status
 */
static void respond(struct pd_ctx *pd, int status)
{
    struct fwk_event resp_event;
    const struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)(&resp_event.params);
    struct pd_set_state_response *resp_params =
        (struct pd_set_state_response *)(&resp_event.params);

    if (!pd->response.pending)
        return;

    status = fwk_thread_get_delayed_response(
        pd->id, pd->response.cookie, &resp_event);
    pd->response.pending = false;

    if (status != FWK_SUCCESS)
        return;

    resp_params->composite_state = req_params->composite_state;
    resp_params->status = status;

    fwk_thread_put_event(&resp_event);
}

/*
 * Determine whether a composite state requires that the transition begins
 * with the highest or lowest level.
 *
 * \param lowest_pd Target of the composite state transition request.
 * \param uint32_t composite_state Target composite state.
 * \retval true The power state transition must propagate upwards.
 * \retval false The power state transition must propagate downwards.
 */
static bool is_upwards_transition_propagation(const struct pd_ctx *lowest_pd,
    uint32_t composite_state)
{
    enum mod_pd_level lowest_level, highest_level, level;
    const struct pd_ctx *pd;
    unsigned int state;

    lowest_level = get_level_from_tree_pos(lowest_pd->config->tree_pos);
    highest_level = get_highest_level_from_composite_state(composite_state);

    for (level = lowest_level, pd = lowest_pd; level <= highest_level;
         level++, pd = pd->parent) {

        state = get_level_state_from_composite_state(composite_state, level);
        if (state == pd->requested_state)
            continue;

        return is_deeper_state(state, pd->requested_state);
    }

    return false;
}
static bool is_allowed_by_children(const struct pd_ctx *pd, unsigned int state)
{
    const struct pd_ctx *child;

    for (child = pd->first_child; child != NULL; child = child->sibling) {
        if (!is_allowed_by_child(child, state, child->requested_state))
            return false;
    }

    return true;
}

static void process_set_state_request(struct pd_ctx *lowest_pd,
    struct pd_set_state_request *req_params, struct fwk_event *resp_event)
{
    int status;
    struct pd_set_state_response *resp_params =
        (struct pd_set_state_response *)resp_event->params;
    uint32_t composite_state;
    bool up;
    enum mod_pd_level lowest_level, highest_level, level;
    unsigned int nb_pds;
    struct pd_ctx *pd;
    const struct pd_ctx *parent;
    unsigned int pd_index;
    unsigned int state;
    bool first_power_state_transition_initiated = false;

    /* A set state request cancels any pending system suspend. */
    mod_pd_ctx.system_suspend.ongoing = false;

    composite_state = req_params->composite_state;
    up = is_upwards_transition_propagation(lowest_pd, composite_state);

    /*
     * It has already been tested as part of the composite state validation that
     * 'highest_level >= lowest_level' and 'highest_level' is lower
     * than the highest power level.
     */
    lowest_level = get_level_from_tree_pos(lowest_pd->config->tree_pos);
    highest_level = get_highest_level_from_composite_state(composite_state);
    nb_pds = highest_level - lowest_level + 1;


    status = FWK_SUCCESS;
    pd = lowest_pd;
    for (pd_index = 0; pd_index < nb_pds; pd_index++, pd = pd->parent) {
        if (up)
            level = lowest_level + pd_index;
        else {
            /*
             * When walking down the power domain tree, get the context of the
             * next power domain to process as well as its level.
             */
            pd = lowest_pd;
            for (level = lowest_level;
                 level < (highest_level - pd_index); level++)
                pd = pd->parent;
        }

        state = get_level_state_from_composite_state(composite_state, level);
        if (state == pd->requested_state){
            continue;
        }

        /*
         * Check that the requested power state is compatible with the states
         * currently requested for the parent and children of the power domain.
         */
        parent = pd->parent;
        if ((parent != NULL) &&
            (!is_allowed_by_child(pd, parent->requested_state, state))) {
            status = FWK_E_PWRSTATE;
            break;
        }

        if (!is_allowed_by_children(pd, state))
            continue;

        /*
         * A new valid power state is requested for the power domain. Send any
         * pending response concerning the previous requested power state.
         */
        pd->requested_state = state;
        pd->power_state_pre_transition_notification_ctx.valid = false;
        respond(pd, FWK_E_OVERWRITTEN);

        if (pd->state_requested_to_driver == state)
            continue;

        /*
         * If a power state transition has already been initiated for an
         * ancestor or descendant, we don't initiate the power state transition
         * now. It will be initiated on completion of the transition of one
         * of its ancestor or descendant.
         */
        if (first_power_state_transition_initiated)
           continue;

        /*
         * If the parent or a child is not currently in a power state
         * compatible with the new requested state for the power domain, do not
         * initiate the transition now as well. It will be initiated when the
         * parent and the children are in a proper state.
         */
        if (!is_allowed_by_parent_and_children(pd, state))
           continue;

        /*
         * Defer the power state transition if power state pre-transition
         * notification responses need to be waited for.
         */
        if (initiate_power_state_pre_transition_notification(pd))
            continue;

        status = initiate_power_state_transition(pd);
        first_power_state_transition_initiated = (status == FWK_SUCCESS);
    }

    if (!resp_event->response_requested)
        return;

    resp_params->status = status;
    resp_params->composite_state = composite_state;
}

/*
 * Complete a system suspend
 *
 * Following the shutdown of the last standing core put all of its ancestors
 * in the MOD_PD_STATE_OFF state but the system power domain which is put
 * into the state that has been asked for.
 *
 * target_pd Description of the power domain target of the 'set composite state'
 *     request to suspend the system in the desired state.
 */
static int complete_system_suspend(struct pd_ctx *target_pd)
{
    enum mod_pd_level level;
    unsigned int composite_state = 0;
    struct pd_ctx *pd = target_pd;
    struct fwk_event resp_event;
    struct pd_set_state_response *resp_params =
        (struct pd_set_state_response *)(&resp_event.params);

    /*
     * Traverse the PD tree bottom-up from current power domain to the top
     * to build the composite state with MOD_PD_STATE_OFF power state for all
     * levels but the last one.
     */
    level = get_level_from_tree_pos(target_pd->config->tree_pos);
    do {
        composite_state |= ((pd->parent != NULL) ? MOD_PD_STATE_OFF :
                            mod_pd_ctx.system_suspend.state)
                           << mod_pd_cs_level_state_shift[level++];
        pd = pd->parent;
    } while (pd != NULL);

    /*
     * Finally, we need to update the highest valid level in the composite
     * state.
     */
    composite_state |= (--level) << MOD_PD_CS_LEVEL_SHIFT;

    resp_event = (struct fwk_event) { 0 };

    process_set_state_request(target_pd,
        &((struct pd_set_state_request){
            .composite_state = composite_state,
        }), &resp_event);

    return resp_params->status;
}

/*
 * Process a 'get composite state' request.
 *
 * pd Description of the target of the 'get state' request
 * req_params Parameters of the 'get state' request
 * resp_params Parameters of the 'get state' request response to be filled in
 */
static void process_get_state_request(struct pd_ctx *pd,
    const struct pd_get_state_request *req_params,
    struct pd_get_state_response *resp_params)
{
    enum mod_pd_level level = get_level_from_tree_pos(pd->config->tree_pos);
    unsigned int composite_state = 0;
    if (!req_params->composite) {
        resp_params->state = pd->current_state;
    } else {
        /*
         * Traverse the PD tree bottom-up from current power domain to the top,
         * collecting node's states and placing them in the correct position in
         * the composite state.
         */
        do {
            composite_state |= pd->current_state <<
                               mod_pd_cs_level_state_shift[level++];
            pd = pd->parent;
        } while (pd != NULL);

        /*
         * Finally, we need to update the highest valid level in
         * the composite state.
         */
        composite_state |= (--level) << MOD_PD_CS_LEVEL_SHIFT;

        resp_params->state = composite_state;
    }

    resp_params->status = FWK_SUCCESS;
}

/*
 * Process a 'reset' request.
 *
 * pd Description of the target of the 'reset' request
 * resp_params Parameters of the 'reset' request response to be filled in
 */
static void process_reset_request(struct pd_ctx *pd,
                                  struct pd_response *resp_params)
{
    struct pd_ctx *child;

    if (pd->requested_state == MOD_PD_STATE_OFF) {
        resp_params->status = FWK_E_PWRSTATE;
        return;
    }

    child = pd->first_child;
    while (child != NULL) {
        if ((child->requested_state != MOD_PD_STATE_OFF) ||
            (child->current_state != MOD_PD_STATE_OFF)) {
            resp_params->status = FWK_E_PWRSTATE;
            return;
        }
        child = child->sibling;
    }

    resp_params->status = pd->driver_api->reset(pd->driver_id);
}

/*
 * Process a power state transition report describing a transition to a deeper
 * state.
 *
 * \param pd Target power domain context
 */
static void process_power_state_transition_report_deeper_state(
    struct pd_ctx *pd)
{
    struct pd_ctx *parent = pd->parent;
    unsigned int requested_state = parent->requested_state;

    if (parent == NULL)
        return;

    if (parent->state_requested_to_driver == requested_state)
        return;

    if (!is_allowed_by_parent_and_children(parent, requested_state))
        return;

    if (!initiate_power_state_pre_transition_notification(parent))
        initiate_power_state_transition(parent);
}

/*
 * Process a power state transition report describing a transition to a
 * shallower state.
 *
 * \param pd Target power domain context
 */
static void process_power_state_transition_report_shallower_state(
    struct pd_ctx *pd)
{
    struct pd_ctx *child;
    unsigned int requested_state;

    for (child = pd->first_child; child != NULL; child = child->sibling) {
        requested_state = child->requested_state;
        if (child->state_requested_to_driver == requested_state)
            continue;

        if (!is_allowed_by_parent_and_children(child, requested_state))
            return;

        if (!initiate_power_state_pre_transition_notification(child))
            initiate_power_state_transition(child);
    }
}

/*
 * Process a power state transition report
 *
 * \param pd Description of the target of the power state transition report
 * \param report_params Parameters of the power state transition report
 */
static void process_power_state_transition_report(struct pd_ctx *pd,
    const struct pd_power_state_transition_report *report_params)
{
    unsigned int new_state = report_params->state;
    unsigned int previous_state;
    struct fwk_event notification_event = {
        .id = mod_pd_notification_id_power_state_transition,
        .response_requested = true
    };
    struct mod_pd_power_state_transition_notification_params *params;

    if (new_state == pd->requested_state)
        respond(pd, FWK_SUCCESS);

    previous_state = pd->current_state;
    pd->current_state = new_state;

    if (pd->power_state_transition_notification_ctx.pending_responses == 0) {
        params = (struct mod_pd_power_state_transition_notification_params *)
            notification_event.params;
        params->state = new_state;
        pd->power_state_transition_notification_ctx.state = new_state;
        fwk_notification_notify(&notification_event,
            &pd->power_state_transition_notification_ctx.pending_responses);
    }

    if ((mod_pd_ctx.system_suspend.ongoing) &&
        (pd == mod_pd_ctx.system_suspend.last_core_pd)) {
        mod_pd_ctx.system_suspend.ongoing = false;
        complete_system_suspend(pd);
    }

    if (is_deeper_state(new_state, previous_state))
        process_power_state_transition_report_deeper_state(pd);
    else if (is_shallower_state(new_state, previous_state))
        process_power_state_transition_report_shallower_state(pd);
}

/*
 * Process a 'system suspend' request
 *
 * req_params Parameters of the 'system suspend' request
 * resp_params Parameters of the 'system suspend' request response to be filled
 *     in
 */
static void process_system_suspend_request(
    const struct pd_system_suspend_request *req_params,
    struct pd_response *resp_params)
{
    int status;
    unsigned int pd_idx;
    struct pd_ctx *pd;
    struct pd_ctx *last_core_pd = NULL;
    struct pd_ctx *last_cluster_pd = NULL;

    /*
     * All core related power domains have to be in the MOD_PD_STATE_OFF state
     * but one core and its ancestors.
     */
    for (pd_idx = 0; pd_idx < mod_pd_ctx.pd_count; pd_idx++) {
        pd = &mod_pd_ctx.pd_ctx_table[pd_idx];
        if ((pd->requested_state == MOD_PD_STATE_OFF) &&
            (pd->current_state == MOD_PD_STATE_OFF))
            continue;

        if (pd->config->attributes.pd_type == MOD_PD_TYPE_CORE) {
            if (last_core_pd != NULL) {
                resp_params->status = FWK_E_STATE;
                return;
            }
            last_core_pd = pd;
        } else if (pd->config->attributes.pd_type == MOD_PD_TYPE_CLUSTER) {
            if (last_cluster_pd != NULL) {
                if (PD_RCAR_CLUSTER1 != pd_idx) {
                    resp_params->status = FWK_E_STATE;
                    return;
                }
            }
            last_cluster_pd = pd;
        }
    }

    if (last_core_pd == NULL) {
        status = complete_system_suspend(
            (last_cluster_pd != NULL) ? last_cluster_pd :
            mod_pd_ctx.system_pd_ctx);
    } else {
        status = last_core_pd->driver_api->prepare_core_for_system_suspend(
            last_core_pd->driver_id);
        if (status == FWK_SUCCESS) {
            mod_pd_ctx.system_suspend.ongoing = true;
            mod_pd_ctx.system_suspend.last_core_pd = last_core_pd;
            mod_pd_ctx.system_suspend.state = req_params->state;
            last_core_pd->requested_state =
                last_core_pd->state_requested_to_driver = MOD_PD_STATE_OFF;
        }
    }

    resp_params->status = status;
}

/*
 * Process a 'system shutdown' request
 *
 * req_params Parameters of the 'system shutdown' request
 * resp_params Parameters of the 'system shutdown' request response to be filled
 *     in
 */
static void process_system_shutdown_request(
    const struct pd_system_shutdown_request *req_params,
    struct pd_response *resp_params)
{
    int status;
    unsigned int pd_idx;
    struct pd_ctx *pd;
    fwk_id_t pd_id;

    for (pd_idx = 0; pd_idx < mod_pd_ctx.pd_count; pd_idx++) {
        pd = &mod_pd_ctx.pd_ctx_table[pd_idx];
        pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_RCAR_POWER_DOMAIN, pd_idx);

        FWK_LOG_TRACE("[PD] Shutting down %s.", fwk_module_get_name(pd_id));

        if (pd->driver_api->shutdown != NULL) {
            status = pd->driver_api->shutdown(pd->driver_id,
                                              req_params->system_shutdown);
        } else
            status = pd->driver_api->set_state(pd->driver_id, MOD_PD_STATE_OFF);

        if (status != FWK_SUCCESS)
            FWK_LOG_ERR("[PD] ERROR: Shutdown of %s returned %d.",
                fwk_module_get_name(pd_id), status);
        else
            FWK_LOG_TRACE("[PD] %s shutdown.", fwk_module_get_name(pd_id));

        pd->requested_state =
            pd->state_requested_to_driver =
            pd->current_state = MOD_PD_STATE_OFF;
    }

    resp_params->status = FWK_E_PANIC;
}

/*
 * API functions
 */

/* Functions common to the public and restricted API */
static int pd_get_domain_type(fwk_id_t pd_id, enum mod_pd_type *type)
{
    struct pd_ctx *pd;

    if (type == NULL)
        return FWK_E_PARAM;

    if (!fwk_module_is_valid_element_id(pd_id))
        return FWK_E_PARAM;

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    *type = pd->config->attributes.pd_type;

    return FWK_SUCCESS;
}

static int pd_get_domain_parent_id(fwk_id_t pd_id, fwk_id_t *parent_pd_id)
{
    const struct pd_ctx *pd;

    if (parent_pd_id == NULL)
        return FWK_E_PARAM;

    if (!fwk_module_is_valid_element_id(pd_id))
        return FWK_E_PARAM;

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    *parent_pd_id = (pd->parent != NULL) ? pd->parent->id : FWK_ID_NONE;

    return FWK_SUCCESS;
}

/* Functions specific to the restricted API */

static int pd_set_state(fwk_id_t pd_id, unsigned int state)
{
    int status;
    struct pd_ctx *pd;
    enum mod_pd_level level;
    struct fwk_event req;
    struct fwk_event resp;
    struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)(&req.params);
    struct pd_set_state_response *resp_params =
        (struct pd_set_state_response *)(&resp.params);

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    if (!is_valid_state(pd, state))
        return FWK_E_PARAM;

    level = get_level_from_tree_pos(pd->config->tree_pos);

    req = (struct fwk_event) {
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_RCAR_POWER_DOMAIN,
                            PD_EVENT_IDX_SET_STATE),
        .target_id = pd_id,
    };

    req_params->composite_state = (level << MOD_PD_CS_LEVEL_SHIFT) |
                                  (state << mod_pd_cs_level_state_shift[level]);

    status = fwk_thread_put_event_and_wait(&req, &resp);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return resp_params->status;
}

#if 0
static int pd_set_state_async(fwk_id_t pd_id,
                              bool response_requested, unsigned int state)
{
    return FWK_SUCCESS;
}
#endif

static int pd_set_composite_state(fwk_id_t pd_id, uint32_t composite_state)
{
    int status;
    struct pd_ctx *pd;
    struct fwk_event req;
    struct fwk_event resp;
    struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)(&req.params);
    struct pd_set_state_response *resp_params =
        (struct pd_set_state_response *)(&resp.params);

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    if (!is_valid_composite_state(pd, composite_state))
        return FWK_E_PARAM;

    req = (struct fwk_event) {
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_RCAR_POWER_DOMAIN,
                            PD_EVENT_IDX_SET_STATE),
        .source_id = pd->driver_id,
        .target_id = pd_id,
    };

    req_params->composite_state = composite_state;

    status = fwk_thread_put_event_and_wait(&req, &resp);
    if (status != FWK_SUCCESS)
        return status;

    return resp_params->status;
}

static int pd_set_composite_state_async(fwk_id_t pd_id,
                                        bool response_requested,
                                        uint32_t composite_state)
{
    return pd_set_composite_state(pd_id, composite_state);
}

static int pd_get_state(fwk_id_t pd_id, unsigned int *state)
{
    int status;
    struct fwk_event req;
    struct fwk_event resp;
    struct pd_get_state_request *req_params =
        (struct pd_get_state_request *)(&req.params);
    struct pd_get_state_response *resp_params =
        (struct pd_get_state_response *)(&resp.params);

    if (state == NULL)
        return FWK_E_PARAM;

    req = (struct fwk_event) {
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_RCAR_POWER_DOMAIN,
                            PD_EVENT_IDX_GET_STATE),
        .target_id = pd_id,
    };

    req_params->composite = false;

    status = fwk_thread_put_event_and_wait(&req, &resp);
    if (status != FWK_SUCCESS)
        return status;

    if (resp_params->status != FWK_SUCCESS)
        return resp_params->status;

    *state = resp_params->state;

    return FWK_SUCCESS;
}

#if 0
static int pd_get_composite_state(fwk_id_t pd_id, unsigned int *composite_state)
{
    int status;
    struct fwk_event req;
    struct fwk_event resp;
    struct pd_get_state_request *req_params =
        (struct pd_get_state_request *)(&req.params);
    struct pd_get_state_response *resp_params =
        (struct pd_get_state_response *)(&resp.params);

    if (composite_state == NULL)
        return FWK_E_PARAM;

    req = (struct fwk_event) {
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_RCAR_POWER_DOMAIN,
                            PD_EVENT_IDX_GET_STATE),
        .target_id = pd_id,
    };

    req_params->composite = true;

    status = fwk_thread_put_event_and_wait(&req, &resp);
    if (status != FWK_SUCCESS)
        return status;

    if (resp_params->status != FWK_SUCCESS)
        return resp_params->status;

    *composite_state = resp_params->state;

    return FWK_SUCCESS;
}
#endif

static int pd_reset(fwk_id_t pd_id)
{
    int status;
    struct fwk_event req;
    struct fwk_event resp;
    struct pd_response *resp_params = (struct pd_response *)(&resp.params);

    req = (struct fwk_event) {
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_RCAR_POWER_DOMAIN,
                            PD_EVENT_IDX_RESET),
        .target_id = pd_id,
    };

    status = fwk_thread_put_event_and_wait(&req, &resp);
    if (status != FWK_SUCCESS)
        return status;

    return resp_params->status;

}

static int pd_system_suspend(unsigned int state)
{
    int status;
    struct fwk_event req;
    struct fwk_event resp;
    struct pd_system_suspend_request *req_params =
        (struct pd_system_suspend_request *)(&req.params);
    struct pd_response *resp_params = (struct pd_response *)(&resp.params);

    req = (struct fwk_event) {
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_RCAR_POWER_DOMAIN,
                           PD_EVENT_IDX_SYSTEM_SUSPEND),
        .target_id = fwk_module_id_rcar_power_domain,
    };

    req_params->state = state;

    status = fwk_thread_put_event_and_wait(&req, &resp);
    if (status != FWK_SUCCESS)
        return status;

    return resp_params->status;
}

static int pd_system_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    int status;
    struct fwk_event req;
    struct fwk_event resp;
    struct pd_system_shutdown_request *req_params =
        (struct pd_system_shutdown_request *)(&req.params);
    struct pd_response *resp_params = (struct pd_response *)(&resp.params);

    req = (struct fwk_event) {
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_RCAR_POWER_DOMAIN,
                           PD_EVENT_IDX_SYSTEM_SHUTDOWN),
        .target_id = fwk_module_id_rcar_power_domain,
    };

    req_params->system_shutdown = system_shutdown;

    status = fwk_thread_put_event_and_wait(&req, &resp);
    if (status != FWK_SUCCESS)
        return status;

    return resp_params->status;
}

/* Functions specific to the driver input API */

static int pd_reset_async(fwk_id_t pd_id, bool response_requested)
{
    struct fwk_event req;
    struct pd_ctx *pd;

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    req = (struct fwk_event) {
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_RCAR_POWER_DOMAIN,
                            PD_EVENT_IDX_RESET),
        .source_id = pd->driver_id,
        .target_id = pd_id,
        .response_requested = response_requested,
    };

    return fwk_thread_put_event(&req);
}

static int report_power_state_transition(const struct pd_ctx *pd,
    unsigned int state)
{
    struct fwk_event report;
    struct pd_power_state_transition_report *report_params =
        (struct pd_power_state_transition_report *)(&report.params);

    report = (struct fwk_event){
        .source_id = pd->driver_id,
        .target_id = pd->id,
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_RCAR_POWER_DOMAIN,
                           PD_EVENT_IDX_REPORT_POWER_STATE_TRANSITION)
    };
    report_params->state = state;

    return fwk_thread_put_event(&report);
}

static int pd_report_power_state_transition(fwk_id_t pd_id, unsigned int state)
{
    const struct pd_ctx *pd;

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    return report_power_state_transition(pd, state);
}

static int pd_get_last_core_pd_id(fwk_id_t *last_core_pd_id)
{
    if (last_core_pd_id == NULL)
        return FWK_E_PARAM;

    *last_core_pd_id = mod_pd_ctx.system_suspend.last_core_pd->id;

    return FWK_SUCCESS;
}

/* Module APIs */

static const struct mod_pd_public_api pd_public_api = {
    .get_domain_type = pd_get_domain_type,
    .get_domain_parent_id = pd_get_domain_parent_id,
};

static const struct mod_pd_restricted_api pd_restricted_api = {
    .get_domain_type = pd_get_domain_type,
    .get_domain_parent_id = pd_get_domain_parent_id,

    .set_state = pd_set_state,
    .set_state_async = pd_set_composite_state_async,
    .get_state = pd_get_state,
    .reset = pd_reset,
    .system_suspend = pd_system_suspend,
    .system_shutdown = pd_system_shutdown
};

static const struct mod_pd_driver_input_api pd_driver_input_api = {
    .set_state_async = pd_set_composite_state_async,
    .reset_async = pd_reset_async,
    .report_power_state_transition = pd_report_power_state_transition,
    .get_last_core_pd_id = pd_get_last_core_pd_id,
};

/*
 * Framework handlers
 */
static int pd_init(fwk_id_t module_id, unsigned int dev_count,
                   const void *data)
{
    if ((data == NULL) || (dev_count == 0))
        return FWK_E_PARAM;

    mod_pd_ctx.config = (struct mod_power_domain_config *)data;

    if ((mod_pd_ctx.config->authorized_id_table == NULL) &&
        (mod_pd_ctx.config->authorized_id_table_size != 0))
        return FWK_E_PARAM;

    mod_pd_ctx.pd_ctx_table = fwk_mm_calloc(dev_count, sizeof(struct pd_ctx));
    if (mod_pd_ctx.pd_ctx_table == NULL)
        return FWK_E_NOMEM;

    mod_pd_ctx.pd_count = dev_count;
    mod_pd_ctx.system_pd_ctx = &mod_pd_ctx.pd_ctx_table[dev_count - 1];

    return FWK_SUCCESS;
}

static int pd_power_domain_init(fwk_id_t pd_id, unsigned int unused,
                                const void *config)
{
    static uint64_t max_tree_pos = MOD_PD_INVALID_TREE_POS;

    const struct mod_power_domain_element_config *pd_config =
        (const struct mod_power_domain_element_config *)config;
    struct pd_ctx *pd;
    unsigned int state;

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    if (!is_valid_tree_pos(pd_config->tree_pos))
        return FWK_E_PARAM;

    if (pd_config->attributes.pd_type >=
        MOD_PD_TYPE_COUNT)
        return FWK_E_PARAM;

    /*
     * Check that the power domains are declared by increasing order of their
     * tree position.
     */
    if ((max_tree_pos != MOD_PD_INVALID_TREE_POS) &&
        (pd_config->tree_pos <= max_tree_pos))
            return FWK_E_PARAM;
    max_tree_pos = pd_config->tree_pos;

    if ((pd_config->allowed_state_mask_table == NULL) ||
        (pd_config->allowed_state_mask_table_size == 0))
        return FWK_E_PARAM;

    pd->allowed_state_mask_table = pd_config->allowed_state_mask_table;
    pd->allowed_state_mask_table_size =
        pd_config->allowed_state_mask_table_size;

    for (state = 0; state < pd->allowed_state_mask_table_size; state++)
        pd->valid_state_mask |= pd->allowed_state_mask_table[state];

    pd->id = pd_id;
    pd->config = pd_config;

    return FWK_SUCCESS;
}

static int pd_post_init(fwk_id_t module_id)
{
    return FWK_SUCCESS;
}

static int pd_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct pd_ctx *pd;
    const struct mod_power_domain_element_config *config;
    struct mod_pd_driver_api *driver_api = NULL;

    /* Nothing to do but during the first round of calls */
    if (round != 0){
        return FWK_SUCCESS;
    }

#if 0
    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_LOG),
            FWK_ID_API(FWK_MODULE_IDX_LOG, 0), &mod_pd_ctx.log_api);
    }
#endif

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(id)];
    config = pd->config;


    status = fwk_module_bind(config->driver_id, config->api_id, &driver_api);
    if (status != FWK_SUCCESS){
        return status;
    }

    pd->driver_id = config->driver_id;
    if ((driver_api->set_state == NULL) ||
        (driver_api->get_state == NULL) ||
        (driver_api->reset == NULL) ||
        ((config->attributes.pd_type == MOD_PD_TYPE_CORE) &&
    (driver_api->prepare_core_for_system_suspend == NULL))){
        return FWK_E_PARAM;
    }

    pd->driver_api = driver_api;

    return FWK_SUCCESS;
}

static int pd_start(fwk_id_t id)
{
    struct pd_ctx *pd;
    int index;
    struct fwk_event notification_event = {
        .source_id = FWK_ID_ELEMENT(
            FWK_MODULE_IDX_RCAR_POWER_DOMAIN,
            CONFIG_POWER_DOMAIN_CHILD_COUNT + rcar_core_get_count()),
        .id = mod_pd_notification_id_power_state_transition,
        .response_requested = true,
    };
    struct mod_pd_power_state_transition_notification_params *params;
    unsigned int notification_count;

    /* Nothing to do for elements */
    if (fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    for (index = mod_pd_ctx.pd_count - 1; index >= 0; index--) {
        pd = &mod_pd_ctx.pd_ctx_table[index];
        pd->requested_state = MOD_PD_STATE_OFF;
        pd->state_requested_to_driver = MOD_PD_STATE_OFF;
        pd->current_state = MOD_PD_STATE_OFF;
    }

    /* CPU0 is always on. */
    pd = &mod_pd_ctx.pd_ctx_table[PD_RCAR_CLUS0CORE0];
    pd->current_state = MOD_PD_STATE_ON;
    /* CLUSTER0 is always on. */
    pd = &mod_pd_ctx.pd_ctx_table[PD_RCAR_CLUSTER0];
    pd->current_state = MOD_PD_STATE_ON;
    /* CLUSTER1 is always on. */
    pd = &mod_pd_ctx.pd_ctx_table[PD_RCAR_CLUSTER1];
    pd->current_state = MOD_PD_STATE_ON;

    params = (struct mod_pd_power_state_transition_notification_params *)
        notification_event.params;
    params->state = MOD_PD_STATE_ON;
    fwk_notification_notify(&notification_event, &notification_count);

    return FWK_SUCCESS;
}

static int pd_process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
                                   fwk_id_t api_id, const void **api)
{
    struct pd_ctx *pd;
    unsigned int id_idx;

    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_PD_API_IDX_PUBLIC:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_MODULE))
            return FWK_E_ACCESS;
        *api = &pd_public_api;
        break;

    case MOD_PD_API_IDX_RESTRICTED:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_MODULE))
            return FWK_E_ACCESS;
        if (mod_pd_ctx.config->authorized_id_table_size == 0) {
            *api = &pd_restricted_api;
            return FWK_SUCCESS;
        }
        for (id_idx = 0;
             id_idx < mod_pd_ctx.config->authorized_id_table_size;
             id_idx++) {

            if (fwk_id_is_equal(source_id,
                mod_pd_ctx.config->authorized_id_table[id_idx])) {
                *api = &pd_restricted_api;
                return FWK_SUCCESS;
            }
        }
        return FWK_E_ACCESS;

    case MOD_PD_API_IDX_DRIVER_INPUT:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
            return FWK_E_ACCESS;
        pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(target_id)];
        if (!fwk_id_is_equal(source_id, pd->driver_id))
            return FWK_E_ACCESS;
        *api = &pd_driver_input_api;
        break;

    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int pd_process_event(const struct fwk_event *event,
                            struct fwk_event *resp)
{
    struct pd_ctx *pd = NULL;

    if (fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT))
        pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(event->target_id)];

    switch (fwk_id_get_event_idx(event->id)) {
    case PD_EVENT_IDX_SET_STATE:
        assert(pd != NULL);

        process_set_state_request(pd,
            (struct pd_set_state_request *)event->params, resp);

        return FWK_SUCCESS;

    case PD_EVENT_IDX_GET_STATE:
        assert(pd != NULL);

        process_get_state_request(pd,
            (struct pd_get_state_request *)event->params,
            (struct pd_get_state_response *)resp->params);

        return FWK_SUCCESS;

    case PD_EVENT_IDX_RESET:
        assert(pd != NULL);

        process_reset_request(pd, (struct pd_response *)resp->params);

        return FWK_SUCCESS;

    case PD_EVENT_IDX_REPORT_POWER_STATE_TRANSITION:
        assert(pd != NULL);

        process_power_state_transition_report(pd,
            (struct pd_power_state_transition_report *)event->params);

        return FWK_SUCCESS;

    case PD_EVENT_IDX_SYSTEM_SUSPEND:
        process_system_suspend_request(
            (struct pd_system_suspend_request *)event->params,
            (struct pd_response *)resp->params);

        return FWK_SUCCESS;

    case PD_EVENT_IDX_SYSTEM_SHUTDOWN:
        process_system_shutdown_request(
            (struct pd_system_shutdown_request *)event->params,
            (struct pd_response *)resp->params);

        return FWK_SUCCESS;

    default:
        FWK_LOG_ERR("[PD] ERROR: Invalid power state request: <%d>.",
            event->id.value);

        return FWK_E_PARAM;
    }
}

static int process_power_state_pre_transition_notification_response(
    struct pd_ctx *pd,
    struct mod_pd_power_state_pre_transition_notification_resp_params *params)
{
    if (pd->power_state_pre_transition_notification_ctx.pending_responses
        == 0) {
        assert(false);
        return FWK_E_PANIC;
    }

    if (params->status != FWK_SUCCESS) {
        pd->power_state_pre_transition_notification_ctx.response_status =
            FWK_E_DEVICE;
    }

    pd->power_state_pre_transition_notification_ctx.pending_responses--;
    if (pd->power_state_pre_transition_notification_ctx.pending_responses != 0)
        return FWK_SUCCESS;

    if (pd->power_state_pre_transition_notification_ctx.valid == true) {
        /*
         * All the notification responses have been received, the requested
         * state for the power domain has not changed in the
         * meantime and all the notified entities agreed on the power state
         * transition, proceed with it.
         */
        if (pd->power_state_pre_transition_notification_ctx.response_status ==
            FWK_SUCCESS)
            initiate_power_state_transition(pd);
    } else {
        /*
         * All the notification responses have been received but the
         * requested state for the power domain has changed, start the
         * processings for the new requested state.
         */
        if ((pd->requested_state == pd->state_requested_to_driver) ||
            (!is_allowed_by_parent_and_children(pd, pd->requested_state)))
            return FWK_SUCCESS;

        if (!initiate_power_state_pre_transition_notification(pd))
            initiate_power_state_transition(pd);
    }

    return FWK_SUCCESS;
}

static int process_power_state_transition_notification_response(
    struct pd_ctx *pd)
{
    struct fwk_event notification_event;
    struct mod_pd_power_state_transition_notification_params *params;

    if (pd->power_state_transition_notification_ctx.pending_responses == 0) {
        assert(false);
        return FWK_E_PANIC;
    }

    pd->power_state_transition_notification_ctx.pending_responses--;
    if (pd->power_state_transition_notification_ctx.pending_responses != 0)
        return FWK_SUCCESS;

    if (pd->power_state_transition_notification_ctx.state == pd->current_state)
        return FWK_SUCCESS;

    /*
     * While receiving the responses, the power state of the power domain
     * has changed. Send a notification for the current power state.
     */
    notification_event.id = mod_pd_notification_id_power_state_transition;
    notification_event.response_requested = true;
    params = (struct mod_pd_power_state_transition_notification_params *)
        notification_event.params;
    params->state = pd->current_state;

    pd->power_state_transition_notification_ctx.state = pd->current_state;
    fwk_notification_notify(&notification_event,
        &pd->power_state_transition_notification_ctx.pending_responses);

    return FWK_SUCCESS;
}

static int pd_process_notification(const struct fwk_event *event,
                                   struct fwk_event *resp)
{
    struct pd_ctx *pd;

    /* Only responses are expected. */
    if (!event->is_response) {
        assert(false);
        return FWK_E_SUPPORT;
    }

    if (!fwk_module_is_valid_element_id(event->target_id)) {
        assert(false);
        return FWK_E_PARAM;
    }

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(event->target_id)];

    if (fwk_id_is_equal(event->id,
                        mod_pd_notification_id_power_state_transition))
        return process_power_state_transition_notification_response(pd);

    return process_power_state_pre_transition_notification_response(pd,
        (struct mod_pd_power_state_pre_transition_notification_resp_params *)
        event->params);
}

/* Module definition */
const struct fwk_module module_rcar_power_domain = {
    .name = "POWER DOMAIN",
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = MOD_PD_API_IDX_COUNT,
    .event_count = PD_EVENT_COUNT,
    .notification_count = MOD_PD_NOTIFICATION_COUNT,
    .init = pd_init,
    .element_init = pd_power_domain_init,
    .post_init = pd_post_init,
    .bind = pd_bind,
    .start = pd_start,
    .process_bind_request = pd_process_bind_request,
    .process_event = pd_process_event,
    .process_notification = pd_process_notification
};
