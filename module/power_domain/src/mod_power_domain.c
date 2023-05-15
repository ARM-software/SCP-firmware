/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power domain management support.
 */

#include <mod_power_domain.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Module and power domain contexts
 */

struct response_ctx {
    /* Pending response flag. */
    bool pending;

    /* Cookie of the event to respond to. */
    uint32_t cookie;
};

/* Context for the power state transition notification */
struct mod_power_state_transition_notification_ctx {
    /* Number of pending notification responses */
    unsigned int pending_responses;

    /*
     * Power state the power domain has transitioned to.
     */
    unsigned int state;

    /* Storage for pre-transition power state */
    unsigned int previous_state;
};

/* Context for the power state pre-transition notification */
struct mod_power_state_pre_transition_notification_ctx {
    /* Number of pending notification responses */
    unsigned int pending_responses;

    /* Target power state */
    unsigned int state;

    /*
     * Status of the responses received so far. Either FWK_SUCCESS if all the
     * responses received so far have indicated success, or FWK_E_DEVICE
     * otherwise.
     */
    int response_status;

    /*
     * Validity flag. Set to true when a notification is sent and reset to
     * false when the requested state for the power domain is changed.
     */
    bool valid;
};

struct pd_ctx {
    /* Identifier of the power domain */
    fwk_id_t id;

    /* Driver interface */
    struct mod_pd_driver_api *driver_api;

    /* Driver's identifier of the power domain */
    fwk_id_t driver_id;

    /* Power domain configuration data */
    const struct mod_power_domain_element_config *config;

    /*
     * Mask of the valid states. Bit \c n in ::valid_states_mask is equal
     * to one if and only if the state \c n is a valid state for the power
     * domain. The number of bits of this field has to be greater or equal than
     * MOD_PD_STATE_COUNT_MAX.
     */
    uint32_t valid_state_mask;

    /*
     * Table of allowed state masks. Bit \c n of the entry \c m is equal to
     * one if and only if the state \c n for the power domain is allowed when
     * its parent is in state \c m. The number of bits of each entry of this
     * table has to be greater or equal than MOD_PD_STATE_COUNT_MAX.
     */
    const uint32_t *allowed_state_mask_table;

    /* Size of the table of allowed state masks */
    size_t allowed_state_mask_table_size;

    /* Composite state is supported */
    bool cs_support;

    /*
     * Composite state mask table. This table provides the mask for each level
     */
    const uint32_t *composite_state_mask_table;

    /* Size of the composite state mask table */
    size_t composite_state_mask_table_size;

    /* Composite state number of levels mask */
    uint32_t composite_state_levels_mask;

    /* Pointer to the power domain's parent context */
    struct pd_ctx *parent;

    /*
     * List if all power domain children if any.
     */
    struct fwk_slist children_list;

    /*
     * Node in the parent list if not the root
     */
    struct fwk_slist_node child_node;

    /* Requested power state for the power domain */
    unsigned int requested_state;

    /* Last power state requested to the driver for the power domain */
    unsigned int state_requested_to_driver;

    /*
     * Current state of the power domain. When a transition is in progress, the
     * current state is different from the last requested state.
     */
    unsigned int current_state;

    /* Pending response context */
    struct response_ctx response;

    /* Context for the power state transition notification */
    struct mod_power_state_transition_notification_ctx
        power_state_transition_notification_ctx;

    /* Context for the power state pre-transition notification */
    struct mod_power_state_pre_transition_notification_ctx
        power_state_pre_transition_notification_ctx;
};

struct system_suspend_ctx {
    /*
     * Flag indicating if the last core is being turned off (true) or not
     * (false).
     */
    bool last_core_off_ongoing;

    /*
     * Flag indicating if the system is being suspended (true) or not (false).
     */
    bool suspend_ongoing;

    /* Last standing core context */
    struct pd_ctx *last_core_pd;

    /* Suspend state for the system power domain */
    unsigned int state;
};

struct system_shutdown_ctx {
    /* Flag indicating if a system shutdown is ongoing */
    bool ongoing;

    /* Total count of notifications sent for system shutdown */
    unsigned int notifications_count;

    /* Type of system shutdown */
    enum mod_pd_system_shutdown system_shutdown;

    /* Flag indicating if a response is requested or not */
    bool is_response_requested;

    /* Flag indicating if the response is delayed or not */
    bool is_delayed_response;

    /* Cookie of the event to respond to in case it is delayed */
    uint32_t cookie;

    /* Pointer to the event to respond in case it is not delayed */
    struct fwk_event *response_event;
};

struct mod_pd_mod_ctx {
    /* Module configuration data */
    struct mod_power_domain_config *config;

    /* Table of power domain contexts */
    struct pd_ctx *pd_ctx_table;

    /* Number of power domains */
    unsigned int pd_count;

    /* Context of the system power domain */
    struct pd_ctx *system_pd_ctx;

    /* System suspend context */
    struct system_suspend_ctx system_suspend;

    /* System shutdown context */
    struct system_shutdown_ctx system_shutdown;
};

/*
 * Power domain module events
 */

/* Power module event indexes */
enum pd_event_idx {
    PD_EVENT_IDX_RESET = MOD_PD_PUBLIC_EVENT_IDX_COUNT,
    PD_EVENT_IDX_REPORT_POWER_STATE_TRANSITION,
    PD_EVENT_IDX_SYSTEM_SUSPEND,
    PD_EVENT_IDX_SYSTEM_SHUTDOWN,
    PD_EVENT_COUNT
};

/* Standard parameter of a response event */
struct pd_response {
    /* Status of the event processing */
    int status;
};

/*
 * MOD_PD_PUBLIC_EVENT_IDX_SET_STATE
 * Parameters of the set state request event
 */
struct pd_set_state_request {
    /*
     * The composite state that defines the power state that the power domain,
     * target of the request, has to be put into and possibly the power states
     * the ancestors of the power domain have to be put into.
     */
    uint32_t composite_state;
};

/*
 * PD_EVENT_IDX_REPORT_POWER_STATE_TRANSITION
 * Parameters of the power state transition report event
 */
struct pd_power_state_transition_report {
    /* The new power state of the power domain */
    uint32_t state;
};

/*
 * PD_EVENT_IDX_SYSTEM_SUSPEND
 * Parameters of the system suspend request event
 */
struct pd_system_suspend_request {
    /* System suspend state, platform specific */
    unsigned int state;
};

/*
 * PD_EVENT_IDX_SYSTEM_SHUTDOWN
 * Parameters of the system shutdown request event
 */
struct pd_system_shutdown_request {
    /* System shutdown type */
    enum mod_pd_system_shutdown system_shutdown;
};

/* Mask of the core composite states */
static const uint32_t core_composite_state_mask_table[] = {
    MOD_PD_CS_STATE_MASK << MOD_PD_CS_LEVEL_0_STATE_SHIFT,
    MOD_PD_CS_STATE_MASK << MOD_PD_CS_LEVEL_1_STATE_SHIFT,
    MOD_PD_CS_STATE_MASK << MOD_PD_CS_LEVEL_2_STATE_SHIFT,
    MOD_PD_CS_STATE_MASK << MOD_PD_CS_LEVEL_3_STATE_SHIFT,
};

/*
 * Internal variables
 */
static struct mod_pd_mod_ctx mod_pd_ctx;

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
static const char driver_error_msg[] = "[PD] Driver error %s (%d) in %s @%d";

static const char *const default_state_name_table[] = {
    "OFF", "ON", "SLEEP", "3",  "4",  "5",  "6",  "7",
    "8",   "9",  "10",    "11", "12", "13", "14", "15"
};
#endif

/*
 * Utility functions
 */

/* State related utility functions */
static bool is_valid_state(const struct pd_ctx *pd, unsigned int state)
{
    return (state < MOD_PD_STATE_COUNT_MAX) &&
        ((pd->valid_state_mask & ((uint32_t)1 << state)) != (uint32_t)0);
}

static unsigned int normalize_state(unsigned int state)
{
    enum mod_pd_state state_type = (enum mod_pd_state)state;

    switch (state_type) {
    case MOD_PD_STATE_OFF:
        return (MOD_PD_STATE_COUNT_MAX + 1);

    case MOD_PD_STATE_SLEEP:
        return MOD_PD_STATE_COUNT_MAX;

    default:
        return state;
    }
}

static bool is_deeper_state(
    unsigned int state,
    unsigned int state_to_compare_to)
{
    return normalize_state(state) > normalize_state(state_to_compare_to);
}

static bool is_shallower_state(
    unsigned int state,
    unsigned int state_to_compare_to)
{
    return normalize_state(state) < normalize_state(state_to_compare_to);
}

static bool is_allowed_by_child(
    const struct pd_ctx *child,
    unsigned int parent_state,
    unsigned int child_state)
{
    if (parent_state >= child->allowed_state_mask_table_size) {
        return false;
    }

    return (
        (child->allowed_state_mask_table[parent_state] &
         ((uint32_t)1 << child_state)) != (uint32_t)0);
}

static bool is_allowed_by_children(const struct pd_ctx *pd, unsigned int state)
{
    const struct pd_ctx *child = NULL;
    struct fwk_slist *c_node = NULL;

    FWK_LIST_FOR_EACH(
        &pd->children_list, c_node, struct pd_ctx, child_node, child)
    {
        if (!is_allowed_by_child(child, state, child->requested_state)) {
            return false;
        }
    }

    return true;
}

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
static const char *get_state_name(const struct pd_ctx *pd, unsigned int state)
{
    static char const unknown_name[] = "Unknown";

    if (state < pd->config->state_name_table_size) {
        return pd->config->state_name_table[state];
    } else if (state < FWK_ARRAY_SIZE(default_state_name_table)) {
        return default_state_name_table[state];
    } else {
        return unknown_name;
    }
}
#endif

static unsigned int number_of_bits_to_shift(uint32_t mask)
{
    unsigned int num_bits = 0;

    if (mask == (uint32_t)0) {
        return 0;
    }

    while ((mask & (uint32_t)1) == (uint32_t)0) {
        mask = mask >> 1;
        num_bits++;
    }

    return num_bits;
}

/* Functions related to a composite state */
static unsigned int get_level_state_from_composite_state(
    const uint32_t *table,
    uint32_t composite_state,
    int level)
{
    uint32_t mask = table[level];
    unsigned int shift = number_of_bits_to_shift(mask);

    return (composite_state & mask) >> shift;
}

static int get_highest_level_from_composite_state(
    const struct pd_ctx *pd,
    uint32_t composite_state)
{
    uint32_t state;
    unsigned int shift, level;
    const uint32_t *state_mask_table;
    unsigned int table_size;

    if (!pd->cs_support) {
        return 0;
    }

    if (pd->composite_state_levels_mask) {
        shift = number_of_bits_to_shift(pd->composite_state_levels_mask);
        level = (pd->composite_state_levels_mask & composite_state) >> shift;
    } else {
        state_mask_table = pd->composite_state_mask_table;
        table_size = (unsigned int)pd->composite_state_mask_table_size;

        for (level = 0; ((level < table_size) && (pd != NULL));
             level++, pd = pd->parent) {
            state = get_level_state_from_composite_state(
                state_mask_table, composite_state, (int)level);
            if (!is_valid_state(pd, state)) {
                break;
            }
        }
        level--;
    }

    return (int)level;
}

static bool is_valid_composite_state(
    struct pd_ctx *target_pd,
    uint32_t composite_state)
{
    unsigned int level, highest_level;
    unsigned int state, child_state = (unsigned int)MOD_PD_STATE_OFF;
    struct pd_ctx *pd = target_pd;
    struct pd_ctx *child = NULL;
    const uint32_t *state_mask_table;
    unsigned int table_size;

    assert(target_pd != NULL);

    if (!pd->cs_support) {
        goto error;
    }

    highest_level = (unsigned int)get_highest_level_from_composite_state(
        pd, composite_state);

    state_mask_table = pd->composite_state_mask_table;
    table_size = (unsigned int)pd->composite_state_mask_table_size;

    if (highest_level >= table_size) {
        goto error;
    }

    for (level = 0; level <= highest_level; level++) {
        if (pd == NULL) {
            goto error;
        }

        state = get_level_state_from_composite_state(
            state_mask_table, composite_state, (int)level);

        if (!is_valid_state(pd, state)) {
            goto error;
        }

        if ((child != NULL) &&
            !is_allowed_by_child(child, state, child_state)) {
            goto error;
        }

        child = pd;
        child_state = state;
        pd = pd->parent;
    }

    return true;

error:
    FWK_LOG_ERR(
        "[PD] Invalid composite state for %s: 0x%" PRIX32,
        fwk_module_get_element_name(target_pd->id),
        composite_state);
    return false;
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
static bool is_upwards_transition_propagation(
    const struct pd_ctx *lowest_pd,
    uint32_t composite_state)
{
    int highest_level, level;
    const struct pd_ctx *pd;
    unsigned int state;
    const uint32_t *state_mask_table;

    highest_level =
        get_highest_level_from_composite_state(lowest_pd, composite_state);

    if (!lowest_pd->cs_support) {
        return is_deeper_state(composite_state, lowest_pd->requested_state);
    }

    state_mask_table = lowest_pd->composite_state_mask_table;

    for (level = 0, pd = lowest_pd; (level <= highest_level) && (pd != NULL);
         level++, pd = pd->parent) {
        state = get_level_state_from_composite_state(
            state_mask_table, composite_state, level);
        if (state == pd->requested_state) {
            continue;
        }

        return is_deeper_state(state, pd->requested_state);
    }

    return false;
}

/* Sub-routine of 'pd_post_init()', to build the power domain tree */
static int connect_pd_tree(void)
{
    unsigned int index;
    struct pd_ctx *pd, *parent;

    for (index = 0; index < mod_pd_ctx.pd_count; index++) {
        pd = &mod_pd_ctx.pd_ctx_table[index];
        if (pd->config->parent_idx >= mod_pd_ctx.pd_count) {
            pd->parent = NULL;
            continue;
        }

        parent = pd->parent = &mod_pd_ctx.pd_ctx_table[pd->config->parent_idx];
        if (parent == NULL) {
            return FWK_E_DATA;
        }
        fwk_list_push_tail(&parent->children_list, &pd->child_node);
    }

    return FWK_SUCCESS;
}

/*
 * Check whether a transition to a given power state for a power domain is
 * possible given the current state of its parent and children (if any).
 *
 * \param pd Description of the power domain to check the power state transition
 *      for.
 * \param state Power state.
 */
static bool is_allowed_by_parent_and_children(
    struct pd_ctx *pd,
    unsigned int state)
{
    struct pd_ctx *parent, *child = NULL;
    struct fwk_slist *c_node = NULL;

    parent = pd->parent;
    if (parent != NULL) {
        if (!is_allowed_by_child(pd, parent->current_state, state)) {
            return false;
        }
    }

    FWK_LIST_FOR_EACH(
        &pd->children_list, c_node, struct pd_ctx, child_node, child)
    {
        if (!is_allowed_by_child(child, state, child->current_state)) {
            return false;
        }
    }

    return true;
}

#ifdef BUILD_HAS_NOTIFICATION
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
static bool check_power_state_pre_transition_notification(
    struct pd_ctx *pd,
    unsigned int state)
{
    if ((state == pd->power_state_pre_transition_notification_ctx.state) &&
        pd->power_state_pre_transition_notification_ctx.valid) {
        return (
            pd->power_state_pre_transition_notification_ctx.response_status !=
            FWK_SUCCESS);
    }

    return true;
}
#endif /* BUILD_HAS_NOTIFICATION */

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
#ifdef BUILD_HAS_NOTIFICATION
    unsigned int state;
    struct fwk_event notification_event = {
        .id = mod_pd_notification_id_power_state_pre_transition,
        .response_requested = true,
        .source_id = FWK_ID_NONE
    };
    struct mod_pd_power_state_pre_transition_notification_params *params;
    int status;

    if (pd->config->disable_state_transition_notifications == true) {
        return false;
    }

    state = pd->requested_state;
    if (!check_power_state_pre_transition_notification(pd, state)) {
        return false;
    }

    /*
     * If still waiting for some responses on the previous power state
     * pre-transition notification, wait for them before to issue the next one.
     */
    if (pd->power_state_pre_transition_notification_ctx.pending_responses !=
        0) {
        return true;
    }

    params = (struct mod_pd_power_state_pre_transition_notification_params *)
                 notification_event.params;
    params->current_state = pd->current_state;
    params->target_state = state;

    notification_event.source_id = pd->id;
    status = fwk_notification_notify(
        &notification_event,
        &pd->power_state_pre_transition_notification_ctx.pending_responses);
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[PD] %s @%d", __func__, __LINE__);
    }

    pd->power_state_pre_transition_notification_ctx.state = state;
    pd->power_state_pre_transition_notification_ctx.response_status =
        FWK_SUCCESS;
    pd->power_state_pre_transition_notification_ctx.valid = true;

    return (
        pd->power_state_pre_transition_notification_ctx.pending_responses != 0);
#else
    return false;
#endif
}

/*
 * Initiate the transition to a power state for a power domain.
 *
 * \param pd Description of the power domain to initiate the state transition
 *      for.
 *
 * \retval ::FWK_SUCCESS The power state transition was initiated.
 * \retval ::FWK_E_DEVICE The power state transition was denied by the driver.
 * \return One of the other driver-defined error codes.
 */
static int initiate_power_state_transition(struct pd_ctx *pd)
{
    int status;
    unsigned int state = pd->requested_state;

    if ((pd->driver_api->deny != NULL) &&
        pd->driver_api->deny(pd->driver_id, state)) {
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_DEBUG
        FWK_LOG_DEBUG(
            "[PD] Transition of %s to state <%s> denied by driver",
            fwk_module_get_element_name(pd->id),
            get_state_name(pd, state));
#endif
        return FWK_E_DEVICE;
    }

    status = pd->driver_api->set_state(pd->driver_id, state);

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_DEBUG
    if (status == FWK_SUCCESS) {
        FWK_LOG_DEBUG(
            "[PD] Transition of %s from <%s> to <%s> succeeded",
            fwk_module_get_element_name(pd->id),
            get_state_name(pd, pd->state_requested_to_driver),
            get_state_name(pd, state));
    }
#endif

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            "[PD] Transition of %s from <%s> to <%s> failed: %s",
            fwk_module_get_element_name(pd->id),
            get_state_name(pd, pd->state_requested_to_driver),
            get_state_name(pd, state),
            fwk_status_str(status));
    }
#endif

    pd->state_requested_to_driver = state;

    return status;
}

/*
 * Respond to a request.
 *
 * \param pd Description of the power domain in charge of the response
 * \param resp_status Response status
 */
static void respond(struct pd_ctx *pd, int resp_status)
{
    int status;
    struct fwk_event resp_event;
    const struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)(&resp_event.params);
    struct pd_set_state_response *resp_params =
        (struct pd_set_state_response *)(&resp_event.params);

    if (!pd->response.pending) {
        return;
    }

    status = fwk_get_delayed_response(pd->id, pd->response.cookie, &resp_event);
    pd->response.pending = false;

    if (status != FWK_SUCCESS) {
        return;
    }

    resp_params->composite_state = req_params->composite_state;
    resp_params->status = resp_status;

    status = fwk_put_event(&resp_event);
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[PD] %s @%d", __func__, __LINE__);
    }
}

/*
 * Process a 'set state' request
 *
 * \param lowest_pd  Description of the target of the 'set state' request
 * \param req_params Parameters of the 'set state' request
 * \param [out] Response event
 */
static void process_set_state_request(
    struct pd_ctx *lowest_pd,
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;
    struct pd_set_state_request *req_params;
    struct pd_set_state_response *resp_params;
    uint32_t composite_state;
    bool up, first_power_state_transition_initiated, composite_state_operation;
    unsigned int lowest_level, highest_level, level;
    unsigned int nb_pds, pd_index, state;
    struct pd_ctx *pd, *pd_in_charge_of_response;
    const struct pd_ctx *parent;
    const uint32_t *state_mask_table = NULL;

    req_params = (struct pd_set_state_request *)event->params;
    resp_params = (struct pd_set_state_response *)resp_event->params;
    pd_in_charge_of_response = NULL;
    first_power_state_transition_initiated = false;

    /* A set state request cancels the completion of system suspend. */
    mod_pd_ctx.system_suspend.last_core_off_ongoing = false;

    composite_state = req_params->composite_state;
    up = is_upwards_transition_propagation(lowest_pd, composite_state);

    /*
     * It has already been tested as part of the composite state validation that
     * 'highest_level >= lowest_level' and 'highest_level' is lower
     * than the highest power level.
     */
    lowest_level = 0;
    highest_level = (unsigned int)get_highest_level_from_composite_state(
        lowest_pd, composite_state);
    nb_pds = highest_level + 1U;

    status = FWK_SUCCESS;
    pd = lowest_pd;

    composite_state_operation = pd->cs_support;
    if (composite_state_operation) {
        state_mask_table = pd->composite_state_mask_table;
    }

    for (pd_index = 0; pd_index < nb_pds; pd_index++, pd = pd->parent) {
        if (up) {
            level = pd_index;
        } else {
            /*
             * When walking down the power domain tree, get the context of the
             * next power domain to process as well as its level.
             */
            pd = lowest_pd;
            for (level = lowest_level; level < (highest_level - pd_index);
                 level++) {
                pd = pd->parent;
            }
        }

        if (composite_state_operation) {
            state = get_level_state_from_composite_state(
                state_mask_table, composite_state, (int)level);
        } else {
            state = composite_state;
        }

        if (state == pd->requested_state) {
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

        if (!is_allowed_by_children(pd, state)) {
            continue;
        }

        /*
         * A new valid power state is requested for the power domain. Send any
         * pending response concerning the previous requested power state.
         */
        pd->requested_state = state;
        pd->power_state_pre_transition_notification_ctx.valid = false;
        respond(pd, FWK_E_OVERWRITTEN);

        if (pd->state_requested_to_driver == state) {
            continue;
        }

        /*
         * The driver must be called thus the processing of the set state
         * request is going to be asynchronous. Assign the responsibility of
         * the response to the request to the power domain. If there is no
         * need for a driver call for the ancestors or descendants of the power
         * domain as part of the processing of the requested composite state,
         * the response to the request will be sent when the transition to the
         * new requested power state is completed.
         */
        pd_in_charge_of_response = pd;

        /*
         * If a power state transition has already been initiated for an
         * ancestor or descendant, we don't initiate the power state transition
         * now. It will be initiated on completion of the transition of one
         * of its ancestor or descendant.
         */
        if (first_power_state_transition_initiated) {
            continue;
        }

        /*
         * If the parent or a child is not currently in a power state
         * compatible with the new requested state for the power domain, do not
         * initiate the transition now as well. It will be initiated when the
         * parent and the children are in a proper state.
         */
        if (!is_allowed_by_parent_and_children(pd, state)) {
            continue;
        }

        /*
         * Defer the power state transition if power state pre-transition
         * notification responses need to be waited for.
         */
        if (initiate_power_state_pre_transition_notification(pd)) {
            continue;
        }

        status = initiate_power_state_transition(pd);
        if (status != FWK_SUCCESS) {
            /*
             * If the power state transition failed, then this power domain is
             * no longer in charge to delay the response.
             */
            pd_in_charge_of_response = NULL;
            break;
        }

        first_power_state_transition_initiated = true;
    }

    if (!event->response_requested) {
        return;
    }

    if (pd_in_charge_of_response != NULL) {
        resp_event->is_delayed_response = true;
        resp_event->source_id = pd_in_charge_of_response->id;
        pd_in_charge_of_response->response.pending = true;
        pd_in_charge_of_response->response.cookie = resp_event->cookie;
    } else {
        resp_params->status = status;
        resp_params->composite_state = composite_state;
    }
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
    unsigned int level;
    unsigned int shift, composite_state = 0;
    struct pd_ctx *pd = target_pd;
    struct fwk_event event, resp_event;
    struct pd_set_state_request *event_params =
        (struct pd_set_state_request *)event.params;
    struct pd_set_state_response *resp_params =
        (struct pd_set_state_response *)(&resp_event.params);
    const uint32_t *state_mask_table;
    int table_size;

    if (!pd->cs_support) {
        return FWK_E_PARAM;
    }

    state_mask_table = pd->composite_state_mask_table;
    table_size = (int)pd->composite_state_mask_table_size;

    mod_pd_ctx.system_suspend.suspend_ongoing = true;

    /*
     * Traverse the PD tree bottom-up from current power domain to the top
     * to build the composite state with MOD_PD_STATE_OFF power state for all
     * levels but the last one.
     */
    level = 0U;
    do {
        shift = number_of_bits_to_shift(state_mask_table[level]);
        composite_state |=
            ((pd->parent != NULL) ? MOD_PD_STATE_OFF :
                                    mod_pd_ctx.system_suspend.state)
            << shift;
        pd = pd->parent;
        level++;
    } while ((pd != NULL) && (level < (unsigned int)table_size));

    /*
     * Finally, we need to update the highest valid level in the composite
     * state.
     */
    composite_state |= (--level) << MOD_PD_CS_LEVEL_SHIFT;

    event = (struct fwk_event){ 0 };
    event_params->composite_state = composite_state;

    resp_event = (struct fwk_event){ 0 };

    process_set_state_request(target_pd, &event, &resp_event);

    return resp_params->status;
}

/*
 * Process a 'get state' request.
 *
 * pd Description of the target of the 'get state' request
 * state the required state to be filled in
 */
static void process_get_state_request(struct pd_ctx *pd, unsigned int *state)
{
    unsigned int level = 0U;
    struct pd_ctx *const base_pd = pd;
    unsigned int composite_state = 0U;
    uint32_t shift;
    const uint32_t *state_mask_table;
    int table_size, cs_idx = 0;

    if (!pd->cs_support) {
        *state = pd->current_state;
    } else {
        state_mask_table = pd->composite_state_mask_table;
        table_size = (int)pd->composite_state_mask_table_size;

        /*
         * Traverse the PD tree bottom-up from current power domain to the top,
         * collecting node's states and placing them in the correct position in
         * the composite state.
         */
        do {
            shift = number_of_bits_to_shift(state_mask_table[cs_idx]);
            composite_state |= pd->current_state << shift;
            pd = pd->parent;
            cs_idx++;
            level++;
        } while (pd != NULL && cs_idx < table_size);

        /*
         * Finally, we need to update the highest valid level in
         * the composite state.
         */
        if (base_pd->composite_state_levels_mask) {
            shift =
                number_of_bits_to_shift(base_pd->composite_state_levels_mask);
            composite_state |= (--level) << shift;
        }

        *state = composite_state;
    }
}

/*
 * Process a 'reset' request.
 *
 * pd Description of the target of the 'reset' request
 * resp_params Parameters of the 'reset' request response to be filled in
 */
static void process_reset_request(
    struct pd_ctx *pd,
    struct pd_response *resp_params)
{
    int status;
    struct pd_ctx *child = NULL;
    struct fwk_slist *c_node = NULL;

    status = FWK_E_PWRSTATE;
    if (pd->requested_state == MOD_PD_STATE_OFF) {
        goto exit;
    }

    FWK_LIST_FOR_EACH(
        &pd->children_list, c_node, struct pd_ctx, child_node, child)
    {
        if ((child->requested_state != MOD_PD_STATE_OFF) ||
            (child->current_state != MOD_PD_STATE_OFF)) {
            goto exit;
        }
    }

    status = pd->driver_api->reset(pd->driver_id);

exit:
    resp_params->status = status;
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
    unsigned int requested_state;
    int status;

    if (parent == NULL) {
        return;
    }

    requested_state = parent->requested_state;

    if (parent->state_requested_to_driver == requested_state) {
        return;
    }

    if (!is_allowed_by_parent_and_children(parent, requested_state)) {
        return;
    }

    if (!initiate_power_state_pre_transition_notification(parent)) {
        status = initiate_power_state_transition(parent);
        if (status != FWK_SUCCESS) {
            FWK_LOG_DEBUG("[PD] %s @%d", __func__, __LINE__);
        }
    }
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
    struct pd_ctx *child = NULL;
    unsigned int requested_state;
    struct fwk_slist *c_node = NULL;
    int status;

    FWK_LIST_FOR_EACH(
        &pd->children_list, c_node, struct pd_ctx, child_node, child)
    {
        requested_state = child->requested_state;
        if (child->state_requested_to_driver == requested_state) {
            continue;
        }

        if (!is_allowed_by_parent_and_children(child, requested_state)) {
            return;
        }

        if (!initiate_power_state_pre_transition_notification(child)) {
            status = initiate_power_state_transition(child);
            if (status != FWK_SUCCESS) {
                FWK_LOG_DEBUG("[PD] %s @%d", __func__, __LINE__);
            }
        }
    }
}

/*
 * Process a power state transition report
 *
 * \param pd Description of the target of the power state transition report
 * \param report_params Parameters of the power state transition report
 */
static void process_power_state_transition_report(
    struct pd_ctx *pd,
    const struct pd_power_state_transition_report *report_params)
{
    unsigned int new_state = report_params->state;
    unsigned int previous_state;
#ifdef BUILD_HAS_NOTIFICATION
    struct fwk_event notification_event = {
        .id = mod_pd_notification_id_power_state_transition,
        .response_requested = true,
        .source_id = FWK_ID_NONE
    };
    struct mod_pd_power_state_transition_notification_params *params;
#endif
    int status;

    if (new_state == pd->requested_state) {
        respond(pd, FWK_SUCCESS);
    }

    previous_state = pd->current_state;
    pd->current_state = new_state;

#ifdef BUILD_HAS_NOTIFICATION
    if (pd->power_state_transition_notification_ctx.pending_responses == 0 &&
        pd->config->disable_state_transition_notifications == false) {
        params = (struct mod_pd_power_state_transition_notification_params *)
                     notification_event.params;
        params->state = new_state;
        pd->power_state_transition_notification_ctx.state = new_state;
        status = fwk_notification_notify(
            &notification_event,
            &pd->power_state_transition_notification_ctx.pending_responses);
        if (status != FWK_SUCCESS) {
            FWK_LOG_DEBUG("[PD] %s @%d", __func__, __LINE__);
        }
    }
#endif

    if ((mod_pd_ctx.system_suspend.last_core_off_ongoing) &&
        (pd == mod_pd_ctx.system_suspend.last_core_pd)) {
        mod_pd_ctx.system_suspend.last_core_off_ongoing = false;
        status = complete_system_suspend(pd);
        if (status != FWK_SUCCESS) {
            FWK_LOG_DEBUG("[PD] %s @%d", __func__, __LINE__);
        }

        return;
    }

    if (pd->parent == NULL) {
        /* this is the top pd (SYSTOP) */
        if (mod_pd_ctx.system_suspend.state != MOD_PD_STATE_ON) {
            /* has gone down, invalidate the system suspend ongoing */
            mod_pd_ctx.system_suspend.suspend_ongoing = false;
        }
    }

#ifdef BUILD_HAS_NOTIFICATION
    /*
     * If notifications are pending, the transition report is delayed until all
     * the state change notifications responses have arrived.
     */
    if (pd->power_state_transition_notification_ctx.pending_responses > 0) {
        /*
         * Save previous state which will be used once all the notifications
         * have arrived to continue for deeper or shallower state for the next
         * power domain.
         */
        pd->power_state_transition_notification_ctx.previous_state =
            previous_state;

        return;
    }
#endif

    /* Update the pd states to follow the new transition */
    pd->requested_state = pd->state_requested_to_driver = pd->current_state;

    if (is_deeper_state(new_state, previous_state)) {
        process_power_state_transition_report_deeper_state(pd);
    } else if (is_shallower_state(new_state, previous_state)) {
        process_power_state_transition_report_shallower_state(pd);
    }
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
            (pd->current_state == MOD_PD_STATE_OFF)) {
            continue;
        }

        if (pd->config->attributes.pd_type == MOD_PD_TYPE_CORE) {
            if (last_core_pd != NULL) {
                resp_params->status = FWK_E_STATE;
                return;
            }
            last_core_pd = pd;
        } else if (pd->config->attributes.pd_type == MOD_PD_TYPE_CLUSTER) {
            if (last_cluster_pd != NULL) {
                resp_params->status = FWK_E_STATE;
                return;
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
            mod_pd_ctx.system_suspend.last_core_off_ongoing = true;

            mod_pd_ctx.system_suspend.last_core_pd = last_core_pd;
            mod_pd_ctx.system_suspend.state = req_params->state;
            last_core_pd->requested_state =
                last_core_pd->state_requested_to_driver =
                    (unsigned int)MOD_PD_STATE_OFF;
        }
    }

    resp_params->status = status;
}

int perform_shutdown_send_response(struct system_shutdown_ctx *ctx)
{
    int status;
    struct fwk_event delayed_resp;
    struct fwk_event *response_event;
    struct pd_response *resp_params;

    if (ctx->is_delayed_response) {
        status = fwk_get_delayed_response(
            fwk_module_id_power_domain, ctx->cookie, &delayed_resp);
        if (status != FWK_SUCCESS) {
            return status;
        }

        delayed_resp.source_id = fwk_module_id_power_domain;
        response_event = &delayed_resp;
    } else {
        response_event = ctx->response_event;
    }

    resp_params = (struct pd_response *)response_event->params;
    resp_params->status = FWK_E_PANIC;
    status = fwk_put_event(response_event);
    return status;
}

void perform_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    struct pd_ctx *pd;
    unsigned int pd_idx;
    fwk_id_t pd_id;
    int status;
    struct mod_pd_driver_api *api;

    for (pd_idx = 0; pd_idx < mod_pd_ctx.pd_count; pd_idx++) {
        pd = &mod_pd_ctx.pd_ctx_table[pd_idx];
        pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, pd_idx);
        api = pd->driver_api;

        (void)pd_id;
        FWK_LOG_INFO(
            "[PD] Shutting down %s", fwk_module_get_element_name(pd_id));

        if (api->shutdown != NULL) {
            status = pd->driver_api->shutdown(pd->driver_id, system_shutdown);
            if (status == FWK_PENDING) {
                /*
                 * Only drivers implementing shutdown API can defer request for
                 * now.
                 */
                return;
            }
        } else {
            if ((api->deny != NULL) &&
                api->deny(pd->driver_id, MOD_PD_STATE_OFF)) {
                status = FWK_E_DEVICE;
            } else {
                status = api->set_state(pd->driver_id, MOD_PD_STATE_OFF);
            }
        }

        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                "[PD] Shutdown of %s returned %s (%d)",
                fwk_module_get_element_name(pd_id),
                fwk_status_str(status),
                status);
        } else {
            FWK_LOG_INFO(
                "[PD] %s shutdown", fwk_module_get_element_name(pd_id));
        }

        pd->requested_state = pd->state_requested_to_driver =
            pd->current_state = (unsigned int)MOD_PD_STATE_OFF;
    }

    /*
     * At this time, the system is already down or will be down soon.
     * Regardless, we tentatively send the response event to the caller, should
     * the system fail to complete the shutdown process, the agent may want to
     * be notified.
     */
    if (mod_pd_ctx.system_shutdown.is_response_requested) {
        status = perform_shutdown_send_response(&mod_pd_ctx.system_shutdown);
        fwk_assert(status == FWK_SUCCESS);
    }

    mod_pd_ctx.system_shutdown.ongoing = false;

    return;
}

/*
 * Process a warm reset request
 */
static int notify_warm_reset(void)
{
#ifdef BUILD_HAS_NOTIFICATION
    unsigned int count;
    struct fwk_event notification = {
        .id = mod_pd_notification_id_pre_warm_reset,
        .source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_POWER_DOMAIN),
    };

    return fwk_notification_notify(&notification, &count);
#else
    return FWK_SUCCESS;
#endif
}

static bool check_and_notify_system_shutdown(
    enum mod_pd_system_shutdown system_shutdown)
{
#ifdef BUILD_HAS_NOTIFICATION
    struct mod_pd_pre_shutdown_notif_params *params;
    int status;

    struct fwk_event notification = { .id = mod_pd_notification_id_pre_shutdown,
                                      .source_id = fwk_module_id_power_domain,
                                      .response_requested = true };

    params = (struct mod_pd_pre_shutdown_notif_params *)notification.params;
    params->system_shutdown = system_shutdown;

    status = fwk_notification_notify(
        &notification, &mod_pd_ctx.system_shutdown.notifications_count);
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[PD] %s @%d", __func__, __LINE__);
    }

    return (mod_pd_ctx.system_shutdown.notifications_count != 0);
#else
    return false;
#endif
}

/*
 * Process a 'system shutdown' request
 */
static void process_system_shutdown_request(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    enum mod_pd_system_shutdown system_shutdown;

    const struct pd_system_shutdown_request *req_params =
        (struct pd_system_shutdown_request *)event->params;
    struct pd_response *resp_params = (struct pd_response *)resp->params;

    system_shutdown = req_params->system_shutdown;

    switch (system_shutdown) {
    case MOD_PD_SYSTEM_WARM_RESET:
        resp_params->status = notify_warm_reset();
        break;

    default:
        /* Check and send pre-shutdown notifications */
        if (check_and_notify_system_shutdown(system_shutdown)) {
            mod_pd_ctx.system_shutdown.ongoing = true;
            mod_pd_ctx.system_shutdown.system_shutdown = system_shutdown;

            if (event->response_requested) {
                mod_pd_ctx.system_shutdown.is_delayed_response = true;
                mod_pd_ctx.system_shutdown.cookie = event->cookie;
                resp->is_delayed_response = true;
            }

            /*
             * The shutdown procedure will be completed once all the
             * notification responses have been received.
             */
            return;
        } else {
            if (event->response_requested) {
                mod_pd_ctx.system_shutdown.is_delayed_response = false;
                mod_pd_ctx.system_shutdown.response_event = resp;
            }
        }

        mod_pd_ctx.system_shutdown.is_response_requested =
            event->response_requested;
        perform_shutdown(system_shutdown);

        resp_params->status = FWK_E_PANIC;
    }
}

/*
 * API functions
 */

/* Functions common to the public and restricted API */
static int pd_get_domain_type(fwk_id_t pd_id, enum mod_pd_type *type)
{
    struct pd_ctx *pd;

    if (type == NULL) {
        return FWK_E_PARAM;
    }

    if (!fwk_module_is_valid_element_id(pd_id)) {
        return FWK_E_PARAM;
    }

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    *type = pd->config->attributes.pd_type;

    return FWK_SUCCESS;
}

static int pd_get_domain_parent_id(fwk_id_t pd_id, fwk_id_t *parent_pd_id)
{
    const struct pd_ctx *pd;

    if (parent_pd_id == NULL) {
        return FWK_E_PARAM;
    }

    if (!fwk_module_is_valid_element_id(pd_id)) {
        return FWK_E_PARAM;
    }

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    *parent_pd_id = (pd->parent != NULL) ? pd->parent->id : FWK_ID_NONE;

    return FWK_SUCCESS;
}

/* Functions specific to the restricted API */
static int pd_set_state(fwk_id_t pd_id, bool response_requested, uint32_t state)
{
    struct pd_ctx *pd;
    struct fwk_event req;
    struct pd_set_state_request *req_params =
        (struct pd_set_state_request *)(&req.params);

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    if (pd->cs_support) {
        if (!is_valid_composite_state(pd, state)) {
            return FWK_E_PARAM;
        }
    } else {
        if (!is_valid_state(pd, state)) {
            return FWK_E_PARAM;
        }
    }

    req = (struct fwk_event){
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_PUBLIC_EVENT_IDX_SET_STATE),
        .source_id = pd->driver_id,
        .target_id = pd_id,
        .response_requested = response_requested,
    };

    req_params->composite_state = state;

    return fwk_put_event(&req);
}

static int pd_get_state(fwk_id_t pd_id, unsigned int *state)
{
    struct pd_ctx *pd = NULL;

    if (state == NULL) {
        return FWK_E_PARAM;
    }

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    process_get_state_request(pd, state);

    return FWK_SUCCESS;
}

static int pd_system_suspend(unsigned int state)
{
    struct fwk_event req;
    struct pd_system_suspend_request *req_params =
        (struct pd_system_suspend_request *)(&req.params);

    req = (struct fwk_event){
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_POWER_DOMAIN, PD_EVENT_IDX_SYSTEM_SUSPEND),
        .target_id = fwk_module_id_power_domain,
    };

    req_params->state = state;

    return fwk_put_event(&req);
}

static int pd_system_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    int status;
    struct fwk_event req;
    struct pd_system_shutdown_request *req_params =
        (struct pd_system_shutdown_request *)(&req.params);

    req = (struct fwk_event){
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_POWER_DOMAIN, PD_EVENT_IDX_SYSTEM_SHUTDOWN),
        .target_id = fwk_module_id_power_domain,
    };

    req_params->system_shutdown = system_shutdown;

    status = fwk_put_event(&req);
    if (status == FWK_SUCCESS) {
        return FWK_PENDING;
    }

    return status;
}

/* Functions specific to the driver input API */

static int pd_reset(fwk_id_t pd_id, bool response_requested)
{
    struct fwk_event_light req;
    struct pd_ctx *pd;

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    req = (struct fwk_event_light){
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_POWER_DOMAIN, PD_EVENT_IDX_RESET),
        .source_id = pd->driver_id,
        .target_id = pd_id,
        .response_requested = response_requested,
    };

    return fwk_put_event(&req);
}

static int report_power_state_transition(
    const struct pd_ctx *pd,
    unsigned int state)
{
    struct fwk_event report;
    struct pd_power_state_transition_report *report_params =
        (struct pd_power_state_transition_report *)(&report.params);

    report =
        (struct fwk_event){ .source_id = pd->driver_id,
                            .target_id = pd->id,
                            .id = FWK_ID_EVENT(
                                FWK_MODULE_IDX_POWER_DOMAIN,
                                PD_EVENT_IDX_REPORT_POWER_STATE_TRANSITION) };
    report_params->state = state;

    return fwk_put_event(&report);
}

static int pd_report_power_state_transition(fwk_id_t pd_id, unsigned int state)
{
    const struct pd_ctx *pd =
        &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    return report_power_state_transition(pd, state);
}

static int pd_get_last_core_pd_id(fwk_id_t *last_core_pd_id)
{
    bool system_suspend_ongoing = mod_pd_ctx.system_suspend.suspend_ongoing;
    if (last_core_pd_id == NULL) {
        return FWK_E_PARAM;
    }

    if (!system_suspend_ongoing) {
        return FWK_E_PWRSTATE;
    }

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
    .get_state = pd_get_state,
    .reset = pd_reset,
    .system_suspend = pd_system_suspend,
    .system_shutdown = pd_system_shutdown
};

static const struct mod_pd_driver_input_api pd_driver_input_api = {
    .set_state = pd_set_state,
    .reset = pd_reset,
    .report_power_state_transition = pd_report_power_state_transition,
    .get_last_core_pd_id = pd_get_last_core_pd_id,
};

/*
 * Framework handlers
 */
static int pd_init(fwk_id_t module_id, unsigned int dev_count, const void *data)
{
    if ((data == NULL) || (dev_count == 0)) {
        return FWK_E_PARAM;
    }

    mod_pd_ctx.config = (struct mod_power_domain_config *)data;

    if ((mod_pd_ctx.config->authorized_id_table == NULL) &&
        (mod_pd_ctx.config->authorized_id_table_size != 0)) {
        return FWK_E_PARAM;
    }

    mod_pd_ctx.pd_ctx_table = fwk_mm_calloc(dev_count, sizeof(struct pd_ctx));

    mod_pd_ctx.pd_count = dev_count;
    mod_pd_ctx.system_pd_ctx = &mod_pd_ctx.pd_ctx_table[dev_count - 1];

    return FWK_SUCCESS;
}

static int pd_power_domain_init(
    fwk_id_t pd_id,
    unsigned int unused,
    const void *config)
{
    const struct mod_power_domain_element_config *pd_config =
        (const struct mod_power_domain_element_config *)config;
    struct pd_ctx *pd;
    unsigned int state;

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(pd_id)];

    fwk_list_init(&pd->children_list);

    if (pd_config->attributes.pd_type >= MOD_PD_TYPE_COUNT) {
        return FWK_E_PARAM;
    }

    if ((pd_config->allowed_state_mask_table == NULL) ||
        (pd_config->allowed_state_mask_table_size == 0)) {
        return FWK_E_PARAM;
    }

    pd->allowed_state_mask_table = pd_config->allowed_state_mask_table;
    pd->allowed_state_mask_table_size =
        pd_config->allowed_state_mask_table_size;

    for (state = 0; state < pd->allowed_state_mask_table_size; state++) {
        pd->valid_state_mask |= pd->allowed_state_mask_table[state];
    }

    if ((pd_config->composite_state_mask_table != NULL) &&
        (pd_config->composite_state_mask_table_size > 0)) {
        pd->composite_state_mask_table = pd_config->composite_state_mask_table;
        pd->composite_state_mask_table_size =
            pd_config->composite_state_mask_table_size;
        pd->composite_state_levels_mask =
            pd_config->composite_state_levels_mask;
        pd->cs_support = true;

    } else if (pd_config->attributes.pd_type == MOD_PD_TYPE_CORE) {
        pd->composite_state_mask_table = core_composite_state_mask_table;
        pd->composite_state_mask_table_size =
            FWK_ARRAY_SIZE(core_composite_state_mask_table);
        pd->composite_state_levels_mask = MOD_PD_CS_STATE_MASK
            << MOD_PD_CS_LEVEL_SHIFT;
        pd->cs_support = true;

    } else {
        pd->cs_support = false;
    }

    pd->id = pd_id;
    pd->config = pd_config;

    return FWK_SUCCESS;
}

static int pd_post_init(fwk_id_t module_id)
{
    int status;

    status = connect_pd_tree();
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

static int pd_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct pd_ctx *pd;
    const struct mod_power_domain_element_config *config;
    struct mod_pd_driver_api *driver_api = NULL;

    /* Nothing to do but during the first round of calls */
    if (round != 0) {
        return FWK_SUCCESS;
    }

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(id)];
    config = pd->config;

    status = fwk_module_bind(config->driver_id, config->api_id, &driver_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    pd->driver_id = config->driver_id;
    if ((driver_api->set_state == NULL) || (driver_api->get_state == NULL) ||
        (driver_api->reset == NULL) ||
        ((config->attributes.pd_type == MOD_PD_TYPE_CORE) &&
         (driver_api->prepare_core_for_system_suspend == NULL))) {
        return FWK_E_PARAM;
    }

    pd->driver_api = driver_api;

    return FWK_SUCCESS;
}

static int pd_start(fwk_id_t id)
{
    int status;
    int index;
    struct pd_ctx *pd;
    unsigned int state;

    /* Nothing to do for elements */
    if (fwk_module_is_valid_element_id(id)) {
        return FWK_SUCCESS;
    }

    for (index = (int)(mod_pd_ctx.pd_count - 1); index >= 0; index--) {
        pd = &mod_pd_ctx.pd_ctx_table[index];
        pd->requested_state = (unsigned int)MOD_PD_STATE_OFF;
        pd->state_requested_to_driver = (unsigned int)MOD_PD_STATE_OFF;
        pd->current_state = (unsigned int)MOD_PD_STATE_OFF;

        /*
         * If the power domain parent is powered down, don't call the driver
         * to get the power domain state as the power domain registers may
         * not be accessible. That way, the drivers don't have to care about
         * this case.
         */
        if ((pd->parent != NULL) &&
            (pd->parent->requested_state == MOD_PD_STATE_OFF)) {
            continue;
        }

        /* Get the current power state of the power domain from its driver. */
        status = pd->driver_api->get_state(pd->driver_id, &state);
        if (status != FWK_SUCCESS) {
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
            FWK_LOG_ERR(
                driver_error_msg,
                fwk_status_str(status),
                status,
                __func__,
                __LINE__);
#endif
        } else {
            pd->requested_state = pd->state_requested_to_driver = state;

            if (state == MOD_PD_STATE_OFF) {
                continue;
            }

            status = report_power_state_transition(pd, state);
            if (status != FWK_SUCCESS) {
                FWK_LOG_DEBUG("[PD] %s @%d", __func__, __LINE__);
            }
        }
    }

    return FWK_SUCCESS;
}

static int pd_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    struct pd_ctx *pd;
    unsigned int id_idx;

    enum mod_pd_api_idx api_id_type =
        (enum mod_pd_api_idx)fwk_id_get_api_idx(api_id);

    switch (api_id_type) {
    case MOD_PD_API_IDX_PUBLIC:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_MODULE)) {
            return FWK_E_ACCESS;
        }
        *api = &pd_public_api;
        break;

    case MOD_PD_API_IDX_RESTRICTED:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_MODULE)) {
            return FWK_E_ACCESS;
        }
        if (mod_pd_ctx.config->authorized_id_table_size == 0) {
            *api = &pd_restricted_api;
            return FWK_SUCCESS;
        }
        for (id_idx = 0; id_idx < mod_pd_ctx.config->authorized_id_table_size;
             id_idx++) {
            if (fwk_id_is_equal(
                    source_id,
                    mod_pd_ctx.config->authorized_id_table[id_idx])) {
                *api = &pd_restricted_api;
                return FWK_SUCCESS;
            }
        }
        return FWK_E_ACCESS;

    case MOD_PD_API_IDX_DRIVER_INPUT:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
            return FWK_E_ACCESS;
        }
        pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(target_id)];
        if (!fwk_id_is_equal(source_id, pd->driver_id)) {
            return FWK_E_ACCESS;
        }
        *api = &pd_driver_input_api;
        break;

    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int pd_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    struct pd_ctx *pd = NULL;

    if (fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT)) {
        pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(event->target_id)];
    }

    switch (fwk_id_get_event_idx(event->id)) {
    case (unsigned int)MOD_PD_PUBLIC_EVENT_IDX_SET_STATE:
        fwk_assert(pd != NULL);

        process_set_state_request(pd, event, resp);

        return FWK_SUCCESS;

    case (unsigned int)PD_EVENT_IDX_RESET:
        fwk_assert(pd != NULL);

        process_reset_request(pd, (struct pd_response *)resp->params);

        return FWK_SUCCESS;

    case (unsigned int)PD_EVENT_IDX_REPORT_POWER_STATE_TRANSITION:
        fwk_assert(pd != NULL);

        process_power_state_transition_report(
            pd, (struct pd_power_state_transition_report *)event->params);

        return FWK_SUCCESS;

    case (unsigned int)PD_EVENT_IDX_SYSTEM_SUSPEND:
        process_system_suspend_request(
            (struct pd_system_suspend_request *)event->params,
            (struct pd_response *)resp->params);

        return FWK_SUCCESS;

    case (unsigned int)PD_EVENT_IDX_SYSTEM_SHUTDOWN:
        process_system_shutdown_request(event, resp);

        return FWK_SUCCESS;

    default:
        FWK_LOG_ERR(
            "[PD] Invalid power state request: %s.", FWK_ID_STR(event->id));

        return FWK_E_PARAM;
    }
}

#ifdef BUILD_HAS_NOTIFICATION
static int process_pre_shutdown_notification_response(void)
{
    if (mod_pd_ctx.system_shutdown.ongoing) {
        mod_pd_ctx.system_shutdown.notifications_count--;

        if (mod_pd_ctx.system_shutdown.notifications_count == 0) {
            /* All notifications for system shutdown have been received */
            perform_shutdown(mod_pd_ctx.system_shutdown.system_shutdown);
        }
        return FWK_SUCCESS;
    } else {
        return FWK_E_PARAM;
    }
}

static int process_power_state_pre_transition_notification_response(
    struct pd_ctx *pd,
    struct mod_pd_power_state_pre_transition_notification_resp_params *params)
{
    if (pd->power_state_pre_transition_notification_ctx.pending_responses ==
        0) {
        fwk_unexpected();
        return FWK_E_PANIC;
    }

    if (params->status != FWK_SUCCESS) {
        pd->power_state_pre_transition_notification_ctx.response_status =
            FWK_E_DEVICE;
    }

    pd->power_state_pre_transition_notification_ctx.pending_responses--;
    if (pd->power_state_pre_transition_notification_ctx.pending_responses !=
        0) {
        return FWK_SUCCESS;
    }

    if (pd->power_state_pre_transition_notification_ctx.valid == true) {
        /*
         * All the notification responses have been received, the requested
         * state for the power domain has not changed in the
         * meantime and all the notified entities agreed on the power state
         * transition, proceed with it.
         */
        if (pd->power_state_pre_transition_notification_ctx.response_status ==
            FWK_SUCCESS) {
            return initiate_power_state_transition(pd);
        }
    } else {
        /*
         * All the notification responses have been received but the
         * requested state for the power domain has changed, start the
         * processings for the new requested state.
         */
        if ((pd->requested_state == pd->state_requested_to_driver) ||
            (!is_allowed_by_parent_and_children(pd, pd->requested_state))) {
            return FWK_SUCCESS;
        }

        if (!initiate_power_state_pre_transition_notification(pd)) {
            return initiate_power_state_transition(pd);
        }
    }

    return FWK_SUCCESS;
}

static int process_power_state_transition_notification_response(
    struct pd_ctx *pd)
{
    struct fwk_event notification_event = { 0 };
    struct mod_pd_power_state_transition_notification_params *params;

    if (pd->power_state_transition_notification_ctx.pending_responses == 0) {
        fwk_unexpected();
        return FWK_E_PANIC;
    }

    pd->power_state_transition_notification_ctx.pending_responses--;
    if (pd->power_state_transition_notification_ctx.pending_responses != 0) {
        return FWK_SUCCESS;
    }

    if (pd->power_state_transition_notification_ctx.state ==
        pd->current_state) {
        /* All notifications received, complete the transition report */

        unsigned int previous_state =
            pd->power_state_transition_notification_ctx.previous_state;

        pd->power_state_transition_notification_ctx.previous_state =
            pd->current_state;
        /*
         * Complete the report state change now that we have all notifications
         */
        if (is_deeper_state(pd->current_state, previous_state)) {
            process_power_state_transition_report_deeper_state(pd);
        } else if (is_shallower_state(pd->current_state, previous_state)) {
            process_power_state_transition_report_shallower_state(pd);
        }

        return FWK_SUCCESS;
    }

    /*
     * While receiving the responses, the power state of the power domain
     * has changed. Send a notification for the current power state.
     */
    notification_event.id = mod_pd_notification_id_power_state_transition;
    notification_event.response_requested = true;
    notification_event.source_id = FWK_ID_NONE;

    params = (struct mod_pd_power_state_transition_notification_params *)
                 notification_event.params;
    params->state = pd->current_state;

    pd->power_state_transition_notification_ctx.state = pd->current_state;
    return fwk_notification_notify(
        &notification_event,
        &pd->power_state_transition_notification_ctx.pending_responses);
}

static int pd_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    struct pd_ctx *pd;

    /* Only responses are expected. */
    if (!event->is_response) {
        fwk_unexpected();
        return FWK_E_SUPPORT;
    }

    if (fwk_id_is_equal(event->id, mod_pd_notification_id_pre_shutdown)) {
        return process_pre_shutdown_notification_response();
    }

    if (!fwk_module_is_valid_element_id(event->target_id)) {
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    pd = &mod_pd_ctx.pd_ctx_table[fwk_id_get_element_idx(event->target_id)];

    if (fwk_id_is_equal(
            event->id, mod_pd_notification_id_power_state_transition)) {
        return process_power_state_transition_notification_response(pd);
    }

    return process_power_state_pre_transition_notification_response(
        pd,
        (struct mod_pd_power_state_pre_transition_notification_resp_params *)
            event->params);
}
#endif /* BUILD_HAS_NOTIFICATION */

/* Module definition */
const struct fwk_module module_power_domain = {
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = (unsigned int)MOD_PD_API_IDX_COUNT,
    .event_count = (unsigned int)PD_EVENT_COUNT,
#ifdef BUILD_HAS_NOTIFICATION
    .notification_count = (unsigned int)MOD_PD_NOTIFICATION_COUNT,
#endif
    .init = pd_init,
    .element_init = pd_power_domain_init,
    .post_init = pd_post_init,
    .bind = pd_bind,
    .start = pd_start,
    .process_bind_request = pd_process_bind_request,
    .process_event = pd_process_event,
#ifdef BUILD_HAS_NOTIFICATION
    .process_notification = pd_process_notification
#endif
};
