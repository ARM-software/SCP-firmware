/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLOCK_H
#define CLOCK_H

#include <mod_clock.h>

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
    } state_transition;

    /* Reference count */
    uint32_t ref_count;
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

/* Get context helper function */
void clock_get_ctx(fwk_id_t clock_id, struct clock_dev_ctx **ctx);

#ifdef BUILD_HAS_CLOCK_TREE_MGMT
/* Connect clock tree interconnecting parent to children nodes */
int clock_connect_tree(struct clock_ctx *module_ctx);
#endif

/*
 * Clock event indexes.
 */
enum clock_event_idx {
    CLOCK_EVENT_IDX_RESPONSE = MOD_CLOCK_EVENT_IDX_COUNT,
    CLOCK_EVENT_IDX_COUNT
};

/*
 * Event identifiers.
 */

/* Identifier of the response event */
static const fwk_id_t mod_clock_event_id_response =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_EVENT_IDX_RESPONSE);

#endif /* CLOCK_H */
