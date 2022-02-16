/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef BUILD_HAS_CLOCK_TREE_MGMT
#    include "clock.h"

#    include <mod_clock.h>

#    include <fwk_assert.h>
#    include <fwk_core.h>
#    include <fwk_event.h>
#    include <fwk_id.h>
#    include <fwk_list.h>
#    include <fwk_log.h>
#    include <fwk_mm.h>
#    include <fwk_module.h>
#    include <fwk_notification.h>
#    include <fwk_status.h>

#    include <stdbool.h>
#    include <stddef.h>
#    include <stdint.h>

static int clk_mgmt_send_event_set(
    struct clock_set_state_params *params,
    fwk_id_t target)
{
    struct fwk_event event;

    event = (struct fwk_event){
        .target_id = target,
        .id = mod_clock_event_id_set_state_pre_request,
    };
    memcpy(event.params, params, sizeof(struct clock_set_state_params));

    return fwk_put_event(&event);
}

static int clk_mgmt_send_event_rate(
    struct clock_set_rate_params *params,
    fwk_id_t target)
{
    struct fwk_event event;

    event = (struct fwk_event){
        .target_id = target,
        .id = mod_clock_event_id_set_rate_pre_request,
    };
    memcpy(event.params, params, sizeof(struct clock_set_rate_params));

    return fwk_put_event(&event);
}

static int clk_mgmt_complete_response(
    struct clock_dev_ctx *ctx,
    struct clock_set_state_params *event_params)
{
    struct mod_clock_driver_resp_params response_params;

    ctx->state_transition.state = CLOCK_STATE_TRANSITION_IDLE;
    /*
     * It responds to the caller depending if it was other clock or
     * if it is the transition initiator.
     */
    if (!ctx->state_transition.is_transition_initiator) {
        /*
         * If requested_state is STOPPED there is no need to send a event to the
         * caller.
         */
        if (event_params->target_state != MOD_CLOCK_STATE_STOPPED) {
            return clk_mgmt_send_event_set(
                event_params, ctx->state_transition.caller_id);
        }

        return FWK_SUCCESS;
    }

    ctx->state_transition.is_transition_initiator = false;
    response_params.status = event_params->caller_status;
    response_params.value.state =
        (enum mod_clock_state)event_params->target_state;

    /*
     * This is the last clock dev in the tree, respond to the caller.
     */
    clock_request_complete(ctx->id, &response_params);

    return FWK_SUCCESS;
}

static int clk_mgmt_complete_transition(
    struct clock_dev_ctx *ctx,
    struct clock_set_state_params *event_params)
{
    int status;
    status = ctx->api->set_state(
        ctx->config->driver_id,
        (enum mod_clock_state)event_params->target_state);

    /* Async drivers not supported with clock tree management */
    if (status == FWK_PENDING) {
        FWK_LOG_WARN(
            "[CLOCK] Async drivers not supported with clock tree mgmt");
        return FWK_E_SUPPORT;
    }
    event_params->caller_status = status;
    return status;
}

int clock_management_process_state(const struct fwk_event *event)
{
    struct clock_set_state_params *event_params =
        (struct clock_set_state_params *)event->params;
    struct clock_dev_ctx *ctx;
    enum mod_clock_state current_state;
    fwk_id_t target_id;
    uint32_t *state;
    int status;

    clock_get_ctx(event->target_id, &ctx);
    state = &ctx->state_transition.state;

    switch (*state) {
    /*
     * At this stage when the first event arrives. It saves the caller_id
     * and depending on what the transition type it will call its parent
     * or children updating the state to the correspondent value.
     */
    case CLOCK_STATE_TRANSITION_IDLE:
        ctx->state_transition.caller_id = event->source_id;
        ctx->state_transition.pending_responses = 0;
        status = ctx->api->get_state(ctx->config->driver_id, &current_state);
        /* Async drivers not supported with clock tree management */
        if (status == FWK_PENDING) {
            FWK_LOG_WARN(
                "[CLOCK] Async drivers not supported with clock tree mgmt");
            return FWK_E_SUPPORT;
        }
        if (status != FWK_SUCCESS) {
            event_params->caller_status = status;
            return clk_mgmt_complete_response(ctx, event_params);
        }

        if (event_params->target_state == MOD_CLOCK_STATE_RUNNING) {
            if (current_state == event_params->target_state) {
                ctx->ref_count++;
                return clk_mgmt_complete_response(ctx, event_params);
            }

            if (fwk_id_is_equal(ctx->parent_id, FWK_ID_NONE)) {
                status = clk_mgmt_complete_transition(ctx, event_params);

                /*
                 * This should not be reached because. asynchronous drivers
                 * are not supported.
                 */
                fwk_assert(status != FWK_E_SUPPORT);

                if (status == FWK_E_SUPPORT) {
                    return status;
                } else if (status == FWK_SUCCESS) {
                    ctx->ref_count++;
                }

                return clk_mgmt_complete_response(ctx, event_params);
            }

            *state = CLOCK_STATE_TRANSITION_WAIT_PARENT;
            target_id = ctx->parent_id;

            /* Raise event */
            return clk_mgmt_send_event_set(event_params, target_id);

        } else if (event_params->target_state == MOD_CLOCK_STATE_STOPPED) {
            ctx->ref_count--;

            if (ctx->ref_count == 0) {
                status = clk_mgmt_complete_transition(ctx, event_params);

                /*
                 * This should not be reached because. asynchronous drivers
                 * are not supported.
                 */
                fwk_assert(status != FWK_E_SUPPORT);

                if (status == FWK_E_SUPPORT) {
                    return status;
                }

                status = clk_mgmt_complete_response(ctx, event_params);
                if (status != FWK_SUCCESS) {
                    return status;
                }
                /*
                 * A sanity check is performed and turn off every unneeded
                 * clock. If ref_count == 0 it will send an event to its parent
                 * to decrement their ref_count and turn it off in case it is
                 * not longer used by an external agent or one of its child.
                 */
                if (!fwk_id_is_equal(ctx->parent_id, FWK_ID_NONE)) {
                    return clk_mgmt_send_event_set(
                        event_params, ctx->parent_id);
                }

                return status;
            }

            return clk_mgmt_complete_response(ctx, event_params);
        } else {
            return FWK_E_PARAM;
        }
        break;

    /*
     * This state waits until parent clock completes running state.
     */
    case CLOCK_STATE_TRANSITION_WAIT_PARENT:

        /*
         * The parent has sent back an event to its child once their clock has
         * changed state successfully.
         */
        if (!fwk_id_is_equal(event->source_id, ctx->parent_id)) {
            /*
             * Assume this node is waiting for its parent to switch ON. Before
             * the parent respond, the same node received a request from one
             * of its children. This statement is going to return error to the
             * caller
             */
            event_params->caller_status = FWK_E_BUSY;
            return clk_mgmt_send_event_set(event_params, event->source_id);
        }
        if (event_params->caller_status == FWK_SUCCESS) {
            status = clk_mgmt_complete_transition(ctx, event_params);

            /*
             * This should not be reached because. asynchronous drivers
             * are not supported.
             */
            fwk_assert(status != FWK_E_SUPPORT);

            if (status == FWK_E_SUPPORT) {
                return status;
            } else if (status == FWK_SUCCESS) {
                ctx->ref_count++;
            } else {
                /*
                 * When transition can not be completed it is returned
                 * immediately to the parent the status and a STOPPED state
                 * to revert the state request.
                 */
                event_params->target_state = MOD_CLOCK_STATE_STOPPED;
                status = clk_mgmt_send_event_set(event_params, ctx->parent_id);
                if (status != FWK_SUCCESS) {
                    return status;
                }
                event_params->target_state = MOD_CLOCK_STATE_RUNNING;
            }
        }

        return clk_mgmt_complete_response(ctx, event_params);

    default:
        return FWK_E_STATE;
    }

    return FWK_SUCCESS;
}

int clock_management_process_rate(const struct fwk_event *event)
{
    struct clock_set_rate_params *event_params =
        (struct clock_set_rate_params *)event->params;
    struct clock_dev_ctx *ctx;
    struct clock_dev_ctx *child = NULL;
    struct fwk_slist *c_node = NULL;
    uint64_t in_rate, out_rate;
    int status;

    clock_get_ctx(event->target_id, &ctx);
    in_rate = event_params->input_rate;

    /*
     * Every child node receive new rate input rate and update its internal
     * output rate to match with its internal settings.
     */
    FWK_LIST_FOR_EACH(
        &ctx->children_list, c_node, struct clock_dev_ctx, child_node, child)
    {
        if (child->api->update_input_rate != NULL) {
            status = child->api->update_input_rate(
                child->config->driver_id, in_rate, &out_rate);
            if (status != FWK_SUCCESS) {
                return status;
            }

            event_params->input_rate = out_rate;
            clk_mgmt_send_event_rate(event_params, child->id);
        }
    }
    return FWK_SUCCESS;
}

bool clock_is_single_node(struct clock_dev_ctx *ctx)
{
    return fwk_id_is_equal(ctx->parent_id, FWK_ID_NONE) &&
        fwk_list_is_empty(&ctx->children_list);
}

/* Sub-routine of 'clock_start()' Build the clock tree */
int clock_connect_tree(struct clock_ctx *module_ctx)
{
    unsigned int clk_idx, parent_idx;
    int status;
    enum mod_clock_state current_state;
    struct clock_dev_ctx *clk, *parent;

    for (clk_idx = 0; clk_idx < module_ctx->dev_count; clk_idx++) {
        clk = &(module_ctx->dev_ctx_table[clk_idx]);

        if (!fwk_optional_id_is_defined(clk->config->parent_id)) {
            clk->parent_id = FWK_ID_NONE;
            continue;
        }

        clk->parent_id = clk->config->parent_id;
        parent_idx = fwk_id_get_element_idx(clk->parent_id);

        parent = &(module_ctx->dev_ctx_table[parent_idx]);
        if (parent == NULL) {
            return FWK_E_DATA;
        }

        fwk_list_push_tail(&parent->children_list, &clk->child_node);

        status = clk->api->get_state(clk->config->driver_id, &current_state);
        if (status == FWK_PENDING) {
            FWK_LOG_WARN(
                "[CLOCK] Async drivers not supported with clock tree mgmt");
            return FWK_E_SUPPORT;
        } else if (status != FWK_SUCCESS) {
            return status;
        }

        if (current_state != MOD_CLOCK_STATE_STOPPED) {
            parent->ref_count++;
        }
    }

    return FWK_SUCCESS;
}

#endif
