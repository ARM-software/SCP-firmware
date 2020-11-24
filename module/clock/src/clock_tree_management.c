/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock.h"

#include <mod_clock.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_list.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

        status = clk->api->get_state(clk->id, &current_state);
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