/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_scp_irq.h"
#include "n1sdp_scp_mmap.h"

#include <mod_n1sdp_timer_sync.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

static const struct fwk_element n1sdp_tsync_element_table[] = {
    [0] = {
        .name = "TIMER_SYNC",
        .data = &((struct mod_n1sdp_tsync_config) {
            .irq = TIMER_SYNC_IRQ,
            .reg = SCP_TIMER_SYNC_BASE,
            .ccix_delay = 25,
            .sync_timeout = 0x200,
            .sync_interval = 0x500,
            .off_threshold = 0xF,
            .target_cnt_base = 0x2A430000,
            .local_offset = SCP_SYS1_BASE,
            .remote_offset = (4UL * FWK_TIB),
        })
    },
    [1] = { 0 },
};

static const struct fwk_element *n1sdp_tsync_get_element_table(
    fwk_id_t module_id)
{
    return n1sdp_tsync_element_table;
}

struct fwk_module_config config_n1sdp_timer_sync = {
    .get_element_table = n1sdp_tsync_get_element_table,
};
