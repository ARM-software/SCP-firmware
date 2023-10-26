/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power domain and module contexts.
 */

#ifndef POWER_DOMAIN_H
#define POWER_DOMAIN_H

#include <mod_power_domain.h>

#include <fwk_core.h>
#include <fwk_id.h>
#include <fwk_log.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

extern struct mod_pd_mod_ctx mod_pd_ctx;

/*
 * Power domain module internal events
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

/*
 * Internal state-related utility functions
 */

/*
 * Check whether a state is valid for a given domain.
 *
 * \param pd Description of the relevant power domain.
 * \param state The state to be checked.
 *
 * \retval true State is valid.
 * \retval false State is not valid.
 */
bool is_valid_state(const struct pd_ctx *pd, unsigned int state);

/*
 * Normalize a given power state.
 *
 * \param state Power state.
 *
 * \return Normalized state.
 */
unsigned int normalize_state(unsigned int state);

/*
 * Find if one state is deeper than an another.
 *
 * \param state Power state.
 * \param state_to_compare_to Power state to compare against.
 *
 * \return Whether or not the given state is deeper.
 */
bool is_deeper_state(unsigned int state, unsigned int state_to_compare_to);

/*
 * Find if one state is shallower than another.
 *
 * \param state Power state.
 * \param state_to_compare_to Power state to compare against.
 *
 * \return Whether or not the given state is shallower.
 */
bool is_shallower_state(unsigned int state, unsigned int state_to_compare_to);

/*
 * Check whether or not a parent power state is allowed by a given child.
 *
 * \param child Child power domain description.
 * \param parent_state Parent power state.
 * \param child_state Child power state.
 *
 * \retval true The state is allowed.
 * \retval true The state is not allowed.
 */
bool is_allowed_by_child(
    const struct pd_ctx *child,
    unsigned int parent_state,
    unsigned int child_state);

/*
 * Check whether or not a given parent state is allowed by that parent's
 * children.
 *
 * \param pd Parent power domain description.
 * \param state Parent power state.
 *
 * \retval true The state is allowed.
 * \retval true The state is not allowed.
 */
bool is_allowed_by_children(const struct pd_ctx *pd, unsigned int state);

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
/*
 * Get the name of a state.
 *
 * \param pd Power domain description.
 * \param state Power state.
 *
 * \return The name of the state.
 */
const char *get_state_name(const struct pd_ctx *pd, unsigned int state);
#endif

/*
 * Find the number of bits to right shift in order for the mask to be all
 * zeroes.
 *
 * \param mask 32-bit mask.
 *
 * \return The number of bits to shift.
 */
unsigned int number_of_bits_to_shift(uint32_t mask);

/*
 * Get the level state from a given composite power state.
 *
 * \param table State mask table.
 * \param composite_state Composite state.
 * \param level Level in power domain tree.
 *
 * \return The level state.
 */
unsigned int get_level_state_from_composite_state(
    const uint32_t *table,
    uint32_t composite_state,
    int level);

/*
 * Get the highest level from a given composite power state.
 *
 * \param pd Power domain description.
 * \param composite_state Composite state.
 *
 * \return The highest level.
 */
int get_highest_level_from_composite_state(
    const struct pd_ctx *pd,
    uint32_t composite_state);

/*
 * Check if a composite state is valid for a given domain.
 *
 * \param target_pd Power domain description.
 * \param composite_state Composite state.
 *
 * \retval true The composite state is valid.
 * \retval true The composite state is not valid.
 */
bool is_valid_composite_state(
    struct pd_ctx *target_pd,
    uint32_t composite_state);

/*
 * Determine whether a composite state requires that the transition begins
 * with the highest or lowest level.
 *
 * \param lowest_pd Target of the composite state transition request.
 * \param uint32_t composite_state Target composite state.
 * \retval true The power state transition must propagate upwards.
 * \retval false The power state transition must propagate downwards.
 */
bool is_upwards_transition_propagation(
    const struct pd_ctx *lowest_pd,
    uint32_t composite_state);

/*
 * Check whether a transition to a given power state for a power domain is
 * possible given the current state of its parent and children (if any).
 *
 * \param pd Description of the power domain to check the power state transition
 *      for.
 * \param state Power state.
 */
bool is_allowed_by_parent_and_children(struct pd_ctx *pd, unsigned int state);

#endif /* POWER_DOMAIN_H */
