/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      N1SDP Timer Synchronization Driver.
 */

#include "n1sdp_scp_mmap.h"

#include <internal/timer_sync.h>

#include <mod_n1sdp_system.h>
#include <mod_n1sdp_timer_sync.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

#define COUNTER_DELTA_MAX      0x100
#define INT_STATUS_TIMEOUT     UINT32_C(1000)
#define SYNC_CHECK_INTERVAL_US UINT32_C(100000)
#define SYNC_RETRIES           100

#define CNTCTL_CR_OFFSET        UINT32_C(0x0)
#define CNTCTL_CVL_OFFSET       UINT32_C(0x8)
#define CNTCTL_CVH_OFFSET       UINT32_C(0xC)
#define CNTCONTROL_CR_EN        UINT32_C(0x00000001)
#define CNTCONTROL_CR_FCREQ     UINT32_C(0x00000100)

/* N1SDP timer synchronization device context */
struct tsync_device_ctx {
    /* Pointer to the device configuration */
    const struct mod_n1sdp_tsync_config *config;

    /* Pointer to the timer synchronization register */
    struct timer_sync_reg *reg;

    /* Storage to hold local counter address */
    uint32_t local_cnt_addr;

    /* Storage to hold remote counter address */
    uint32_t remote_cnt_addr;
};

/* N1SDP timer synchronization module context */
struct timer_sync_ctx {
    /* Table of device contexts */
    struct tsync_device_ctx *device_ctx_table;

    /* Windowed AP memory access API */
    struct mod_n1sdp_system_ap_memory_access_api *ap_mem_api;

    /* Timer alarm API */
    struct mod_timer_api *timer_api;
};

static struct timer_sync_ctx tsync_ctx;

static void timer_sync_isr(uintptr_t data)
{
    struct tsync_device_ctx *dev_ctx;
    uint32_t timeout;

    dev_ctx = (struct tsync_device_ctx *)data;
    if (dev_ctx == NULL)
        return;

    timeout = INT_STATUS_TIMEOUT;
    while (dev_ctx->reg->SLVCHIP_GCNT_INT_STATUS &
           SLVCHIP_GCNT_INT_STATUS_INT_MASK) {
        dev_ctx->reg->SLVCHIP_GCNT_INT_CLR = SLVCHIP_GCNT_INT_CLR_MASK;
        timeout--;
        if (timeout == 0)
            return;
    }
    dev_ctx->reg->SLVCHIP_GCNT_SYNC_CTRL = 0;
    dev_ctx->reg->SLVCHIP_GCNT_SYNC_CTRL = SLV_GCNT_SYNC_CTRL_EN_MASK;
}

bool is_timer_synced(struct tsync_device_ctx *ctx)
{
    uint32_t low;
    uint32_t low_r;

    fwk_assert(ctx != NULL);

    low = *(volatile uint32_t *)(ctx->local_cnt_addr + CNTCTL_CVL_OFFSET);
    tsync_ctx.ap_mem_api->enable_ap_memory_access(ctx->remote_cnt_addr);
    low_r = *(volatile uint32_t *)(SCP_AP_1MB_WINDOW_BASE +
        ((ctx->local_cnt_addr + CNTCTL_CVL_OFFSET) &
         SCP_AP_1MB_WINDOW_ADDR_MASK));
    return (((low_r - low) < COUNTER_DELTA_MAX));
}

void n1sdp_timer_reset_counter(struct tsync_device_ctx *ctx)
{
    *(volatile uint32_t *)(ctx->local_cnt_addr + CNTCTL_CR_OFFSET) = 0;
    *(volatile uint32_t *)(ctx->local_cnt_addr + CNTCTL_CVH_OFFSET) = 0;
    *(volatile uint32_t *)(ctx->local_cnt_addr + CNTCTL_CVL_OFFSET) = 0;
    *(volatile uint32_t *)(ctx->local_cnt_addr + CNTCTL_CR_OFFSET) |=
        (CNTCONTROL_CR_EN | CNTCONTROL_CR_FCREQ);
}

/*
 * Timer sync module driver API
 */
static int n1sdp_sync_master_timer(fwk_id_t id)
{
    unsigned int retries;
    struct tsync_device_ctx *device_ctx;

    device_ctx = &tsync_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

    n1sdp_timer_reset_counter(device_ctx);
    device_ctx->reg->GCNT_TIMEOUT = device_ctx->config->sync_timeout;
    device_ctx->reg->SLVCHIP_GCNT_NW_DELAY = device_ctx->config->ccix_delay;
    device_ctx->reg->MST_GCNT_SYNC_CTRL = MST_GCNT_SYNC_CTRL_EN_MASK;

    retries = SYNC_RETRIES;
    do {
        if (is_timer_synced(device_ctx))
            break;
        tsync_ctx.timer_api->delay(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                   SYNC_CHECK_INTERVAL_US);
        retries--;
        if ((retries % 10) == 0)
            FWK_LOG_INFO("[N1SDP_TIMER_SYNC] Retries: %u", retries);
    } while (retries != 0);

    if ((retries == 0) && (!is_timer_synced(device_ctx)))
        FWK_LOG_INFO("[N1SDP_TIMER_SYNC] Timeout!");
    else
        FWK_LOG_INFO("[N1SDP_TIMER_SYNC] Synced");

    return FWK_SUCCESS;
}

static int n1sdp_sync_slave_timer(fwk_id_t id)
{
    struct tsync_device_ctx *device_ctx;
    unsigned int irq;

    device_ctx = &tsync_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

    n1sdp_timer_reset_counter(device_ctx);
    irq = device_ctx->config->irq;
    fwk_interrupt_set_isr_param(irq, &timer_sync_isr, (uintptr_t)device_ctx);
    fwk_interrupt_clear_pending(irq);
    fwk_interrupt_enable(irq);

    device_ctx->reg->GCNT_TIMEOUT = device_ctx->config->sync_timeout;
    device_ctx->reg->SLVCHIP_GCNT_SYNC_INTERVAL =
        device_ctx->config->sync_interval;
    device_ctx->reg->SLVCHIP_GCNT_OFF_THRESHOLD =
        device_ctx->config->off_threshold;
    device_ctx->reg->SLVCHIP_GCNT_SYNC_CTRL = SLV_GCNT_SYNC_CTRL_EN_MASK;

    return FWK_SUCCESS;
}

const struct n1sdp_timer_sync_api n1sdp_tsync_api = {
    .master_sync = n1sdp_sync_master_timer,
    .slave_sync = n1sdp_sync_slave_timer,
};

/*
 * Framework handlers
 */

static int n1sdp_timer_sync_init(fwk_id_t module_id, unsigned int device_count,
    const void *unused)
{
    if (device_count == 0)
        return FWK_E_PARAM;

    tsync_ctx.device_ctx_table = fwk_mm_calloc(device_count,
        sizeof(tsync_ctx.device_ctx_table[0]));

    return FWK_SUCCESS;
}

static int n1sdp_timer_sync_device_init(fwk_id_t device_id,
    unsigned int unused, const void *data)
{
    struct mod_n1sdp_tsync_config *config =
        (struct mod_n1sdp_tsync_config *)data;
    struct tsync_device_ctx *device_ctx;

    if (config->reg == 0)
        return FWK_E_PARAM;

    device_ctx = &tsync_ctx.device_ctx_table[
        fwk_id_get_element_idx(device_id)];

    device_ctx->config = config;
    device_ctx->reg = (struct timer_sync_reg *)config->reg;
    device_ctx->remote_cnt_addr = (uint32_t)((config->remote_offset +
        (uint64_t)config->target_cnt_base) >> SCP_AP_1MB_WINDOW_SHIFT);
    device_ctx->local_cnt_addr = config->target_cnt_base +
                                 config->local_offset;

    return FWK_SUCCESS;
}

static int n1sdp_timer_sync_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if ((round == 0) && fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_SYSTEM),
            FWK_ID_API(FWK_MODULE_IDX_N1SDP_SYSTEM,
                       MOD_N1SDP_SYSTEM_API_IDX_AP_MEMORY_ACCESS),
            &tsync_ctx.ap_mem_api);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_module_bind(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER),
            &tsync_ctx.timer_api);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int n1sdp_timer_sync_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    *api = &n1sdp_tsync_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_n1sdp_timer_sync = {
    .name = "N1SDP Timer Sync",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = N1SDP_TIMER_SYNC_API_COUNT,
    .init = n1sdp_timer_sync_init,
    .element_init = n1sdp_timer_sync_device_init,
    .bind = n1sdp_timer_sync_bind,
    .process_bind_request = n1sdp_timer_sync_process_bind_request,
};
