/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLOCK_H
#define CLOCK_H

#include <mod_clock.h>
#ifdef FWK_MODULE_ID_POWER_DOMAIN
#    include <mod_power_domain.h>
#endif

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_list.h>

/* Device context */
struct clock_dev_ctx {
    /* Pointer to the element configuration data */
    const struct mod_clock_dev_config *config;

    /* Driver API */
    struct mod_clock_drv_api *api;

    struct {
        /* Cookie for the pre-transition notification response */
        unsigned int pre_power_transition_cookie;

        /* Counter of notifications sent */
        unsigned int transition_pending_sent;

        /* Status of the pending transition */
        unsigned int transition_pending_response_status;
    } pd_notif;

    struct {
        /* A request is on-going */
        bool is_ongoing;

        /* Cookie for the response event */
        uint32_t cookie;
    } request;

#ifdef BUILD_HAS_CLOCK_TREE_MGMT
    /* Identifier of the clock */
    fwk_id_t id;

    /* Parent identifier */
    fwk_id_t parent_id;

    /* List all clock children if any */
    struct fwk_slist children_list;

    /*
     * Node in the parent list if not the root
     */
    struct fwk_slist_node child_node;

    /* Clock state transition */
    struct {
        /* Number of pending responses */
        uint32_t pending_responses;
        /* Clock state transition */
        uint32_t state;
        /* Clock id caller to transition */
        fwk_id_t caller_id;
        /* Flag indicating if this clock is the starter of transition */
        bool is_transition_initiator;
    } state_transition;

    /* Reference count */
    uint32_t ref_count;
#endif

#ifdef FWK_MODULE_ID_POWER_DOMAIN
    /* Power domain module restricted API pointer */
    struct mod_pd_restricted_api *mod_pd_restricted_api;
#endif
};

/* Module context */
struct clock_ctx {
    /* Pointer to the module configuration data */
    const struct mod_clock_config *config;

    /* Table of elements context */
    struct clock_dev_ctx *dev_ctx_table;

    /* Number of clocks devices */
    uint32_t dev_count;
};

/* Set state event parameters */
struct clock_set_state_params {
    uint32_t target_state;
    int caller_status;
};

/* Set rate event parameters */
struct clock_set_rate_params {
    uint64_t input_rate;
};

/* List of state transition management state */
enum clock_state_transition_stage {
    /* This is the idle state */
    CLOCK_STATE_TRANSITION_IDLE,

    /* This state wait for parent run response */
    CLOCK_STATE_TRANSITION_WAIT_PARENT,

    /* Number of states in a transition */
    CLOCK_STATE_TRANSITION_COUNT
};

/* Get context helper function */
void clock_get_ctx(fwk_id_t clock_id, struct clock_dev_ctx **ctx);

/* Clock request complete function */
void clock_request_complete(
    fwk_id_t dev_id,
    struct mod_clock_driver_resp_params *response);

#ifdef BUILD_HAS_CLOCK_TREE_MGMT
/* Clock tree management state machine for setting state */
int clock_management_process_state(const struct fwk_event *event);

/* Clock tree management state machine for setting rate */
int clock_management_process_rate(const struct fwk_event *event);

/* Check if a clock is a single node. It returns true or false whether Clock is
 * a single node or not.
 */
bool clock_is_single_node(struct clock_dev_ctx *ctx);

/* Connect clock tree interconnecting parent to children nodes */
int clock_connect_tree(struct clock_ctx *module_ctx);
#endif

/*
 * Clock event indexes.
 */
enum clock_event_idx {
    CLOCK_EVENT_IDX_RESPONSE = MOD_CLOCK_EVENT_IDX_COUNT,

#ifdef BUILD_HAS_CLOCK_TREE_MGMT
    CLOCK_EVENT_IDX_SET_STATE_PRE_REQUEST,
    CLOCK_EVENT_IDX_SET_RATE_PRE_REQUEST,
#endif

    CLOCK_EVENT_IDX_COUNT
};

/*
 * Event identifiers.
 */

/* Identifier of the response event */
static const fwk_id_t mod_clock_event_id_response =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_EVENT_IDX_RESPONSE);

#ifdef BUILD_HAS_CLOCK_TREE_MGMT
/* Identifier of the set state pre-request */
static const fwk_id_t mod_clock_event_id_set_state_pre_request =
    FWK_ID_EVENT_INIT(
        FWK_MODULE_IDX_CLOCK,
        CLOCK_EVENT_IDX_SET_STATE_PRE_REQUEST);

/* Identifier of the set rate pre-request */
static const fwk_id_t mod_clock_event_id_set_rate_pre_request =
    FWK_ID_EVENT_INIT(
        FWK_MODULE_IDX_CLOCK,
        CLOCK_EVENT_IDX_SET_RATE_PRE_REQUEST);
#endif

#endif /* CLOCK_H */
