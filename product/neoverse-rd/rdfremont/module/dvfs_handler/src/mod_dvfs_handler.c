/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/dvfs_handler_reg.h>

#include <mod_dvfs_handler.h>
#include <mod_transport.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#define MOD_NAME         "[DVFS HANDLER] "
#define DEVICE_IDX       "[DEVICE:%u] "
#define MOD_NAME_DEV_IDX MOD_NAME DEVICE_IDX

/* Module element context */
struct dvfs_handler_device_ctx {
    /* ID of the element representing a DVFS domain */
    fwk_id_t device_id;

    /* Module configuration data */
    struct mod_dvfs_handler_element_config *config;

    /* Current perf level */
    unsigned int current_level;

    /* Transport fastchannel API */
    struct mod_transport_fast_channels_api *transport_fch_api;
};

/* Pointer to array of module element context data */
struct dvfs_handler_device_ctx *device_ctx;

/*
 * Update the DVFS handler 'policy frequency' register with the new frequency
 * in Hz.
 */
static inline void set_frequency(unsigned int device_idx, unsigned int freq)
{
    struct dvfs_frame *dvfs_frame =
        (struct dvfs_frame *)device_ctx[device_idx].config->dvfs_frame_addr;

    if (dvfs_frame->policy_frequency_status != freq) {
        dvfs_frame->policy_frequency = freq;
        FWK_LOG_INFO(
            MOD_NAME_DEV_IDX "setting frequency to %uHz", device_idx, freq);
    }
}

/*
 * Update the DVFS handler 'policy voltage' register with the new voltage
 * in micro-volts.
 */
static inline void set_voltage(unsigned int device_idx, unsigned int vlt)
{
    struct dvfs_frame *dvfs_frame =
        (struct dvfs_frame *)device_ctx[device_idx].config->dvfs_frame_addr;

    if (dvfs_frame->policy_voltage_status != vlt) {
        dvfs_frame->policy_voltage = vlt;
        FWK_LOG_INFO(
            MOD_NAME_DEV_IDX "setting voltage to %uuV", device_idx, vlt);
    }
}

/*
 * Check whether the frequency and voltage settings are reflected in status
 * register.
 */
static int check_dvfs_status(
    unsigned int device_idx,
    unsigned int freq,
    unsigned int vlt)
{
    struct dvfs_frame *dvfs_frame =
        (struct dvfs_frame *)device_ctx[device_idx].config->dvfs_frame_addr;

    if (dvfs_frame->policy_frequency_status != freq) {
        FWK_LOG_CRIT(
            MOD_NAME_DEV_IDX "setting frequency to %uHz pending",
            device_idx,
            freq);

        return FWK_PENDING;
    }

    if (dvfs_frame->policy_voltage_status != vlt) {
        FWK_LOG_CRIT(
            MOD_NAME_DEV_IDX "setting voltage to %uuV pending",
            device_idx,
            vlt);

        return FWK_PENDING;
    }

    return FWK_SUCCESS;
}

/*
 * Ensure OPP table is sorted in ascending order. Count the number of OPP
 * entries in the OPP table.
 */
static int validate_opps(
    const struct mod_dvfs_handler_opp *opps,
    unsigned int device_idx)
{
    struct dvfs_handler_device_ctx *dev_ctx = &device_ctx[device_idx];
    unsigned int prev_opp_level = 0;
    unsigned int opp_idx;

    for (opp_idx = 0; opp_idx < dev_ctx->config->opp_count; opp_idx++) {
        if ((opps[opp_idx].voltage == 0) || (opps[opp_idx].frequency == 0) ||
            (opps[opp_idx].level <= prev_opp_level)) {
            return FWK_E_PARAM;
        }
        prev_opp_level = opps[opp_idx].level;
    }

    return FWK_SUCCESS;
}

/*
 * Get the OPP table entry corresponding to the requested performance level.
 */
static const struct mod_dvfs_handler_opp *get_opp_for_level(
    unsigned int device_idx,
    unsigned int level)
{
    struct dvfs_handler_device_ctx *dev_ctx = &device_ctx[device_idx];
    struct mod_dvfs_handler_opp *opp_table;
    unsigned int opp_idx;

    opp_table = dev_ctx->config->opps;

    /*
     * Find an OPP level that is equal to or the nearest lower
     * OPP level that is supported.
     */

    if (level <= opp_table[0].level) {
        return &opp_table[0];
    }

    for (opp_idx = 1; opp_idx < dev_ctx->config->opp_count; opp_idx++) {
        /*
         * The OPP level requested is not present in OPP table. Return
         * a valid OPP table entry just below the requested level.
         */
        if ((level < opp_table[opp_idx].level) &&
            (level >= opp_table[opp_idx - 1].level))
            return &opp_table[opp_idx - 1];
    }

    /*
     * Requested level is equal or higher than supported highest perf level.
     * So return the highest supported perf level.
     */
    return &opp_table[dev_ctx->config->opp_count - 1];
}

/* Set a requested perf level */
static int dvfs_handler_set_level(unsigned int device_idx, unsigned int level)
{
    struct dvfs_handler_device_ctx *dev_ctx = &device_ctx[device_idx];
    const struct mod_dvfs_handler_opp *new_opp;
    unsigned int retry;
    int status;

    if (dev_ctx->current_level == level) {
        return FWK_SUCCESS;
    }

    /* Find an OPP for the performance level requested */
    new_opp = get_opp_for_level(device_idx, level);
    if (new_opp == NULL) {
        FWK_LOG_ERR(
            MOD_NAME_DEV_IDX "invalid level %u requested", device_idx, level);

        return FWK_E_RANGE;
    } else if (new_opp->level == dev_ctx->current_level) {
        return FWK_SUCCESS;
    }

    /* Program the hardware registers with frequency and voltage values */
    if (new_opp->level < dev_ctx->current_level) {
        /* Perf reduction, lower the frequency before reducing voltage */
        set_frequency(device_idx, new_opp->frequency);
        set_voltage(device_idx, new_opp->voltage);
    } else {
        /* Perf increase, raise the voltage before increasing frequency */
        set_voltage(device_idx, new_opp->voltage);
        set_frequency(device_idx, new_opp->frequency);
    }

    /*
     * Check the status registers to determine whether the frequency and
     * voltage updates are reflected.
     */
    for (retry = 0; retry <= dev_ctx->config->status_check_max; retry++) {
        status =
            check_dvfs_status(device_idx, new_opp->frequency, new_opp->voltage);
        if (status == FWK_SUCCESS) {
            dev_ctx->current_level = new_opp->level;

            return FWK_SUCCESS;
        }
    }

    FWK_LOG_ERR(
        MOD_NAME_DEV_IDX "failed to set requested level %u", device_idx, level);

    return FWK_E_TIMEOUT;
}

/*
 * Callback function registered with transport module and there by with MHU3
 * module. This function will be invoked by the MHU3 module on receiving a
 * data over fastchannel with performance level set request.
 */
static void dvfs_handler_fch_set_level_callback(uintptr_t dvfs_id)
{
    struct mod_transport_fast_channel_addr fch;
    struct dvfs_handler_device_ctx *dev_ctx;
    fwk_id_t id = *(fwk_id_t *)dvfs_id;
    FWK_R unsigned int *perf_level;
    unsigned int device_idx;
    int status;

    device_idx = fwk_id_get_element_idx(id);
    dev_ctx = &device_ctx[device_idx];

    status = dev_ctx->transport_fch_api->transport_get_fch_address(
        dev_ctx->config->transport_element_id, &fch);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME_DEV_IDX "failed to get fastchannel address, status: %d",
            device_idx,
            status);

        return;
    }

    perf_level = (FWK_R unsigned int *)fch.local_view_address;
    status = dvfs_handler_set_level(device_idx, *perf_level);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME_DEV_IDX "failed to set level %u, status: %d",
            device_idx,
            *perf_level,
            status);
    }
}

/* Module framework handlers */

static int dvfs_handler_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    device_ctx =
        fwk_mm_calloc(element_count, sizeof(struct dvfs_handler_device_ctx));
    if (device_ctx == NULL) {
        FWK_LOG_CRIT(MOD_NAME "failed to allocate memory for module context");

        return FWK_E_NOMEM;
    }

    return FWK_SUCCESS;
}

static int dvfs_handler_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    unsigned int device_idx = fwk_id_get_element_idx(element_id);
    struct dvfs_handler_device_ctx *dev_ctx = &device_ctx[device_idx];

    dev_ctx->device_id = element_id;
    dev_ctx->config = (struct mod_dvfs_handler_element_config *)data;

    if (dev_ctx->config->opps == NULL) {
        return FWK_E_PARAM;
    }

    if (dev_ctx->config->opp_count == 0) {
        FWK_LOG_ERR(MOD_NAME_DEV_IDX "OPP table count invalid", device_idx);

        return FWK_E_PARAM;
    }

    if (validate_opps(dev_ctx->config->opps, device_idx) != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME_DEV_IDX "OPP table validation failed", device_idx);

        return FWK_E_PARAM;
    }

    FWK_LOG_INFO(
        MOD_NAME_DEV_IDX "found %d supported operating points",
        device_idx,
        dev_ctx->config->opp_count);

    return FWK_SUCCESS;
}

static int dvfs_handler_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct dvfs_handler_device_ctx *dev_ctx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_SUCCESS;
    }

    dev_ctx = &device_ctx[fwk_id_get_element_idx(id)];

    /* Bind to transport channel */
    status = fwk_module_bind(
        dev_ctx->config->transport_element_id,
        dev_ctx->config->transport_fch_api_id,
        &dev_ctx->transport_fch_api);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME_DEV_IDX "bind with transport failed, status: %d",
            fwk_id_get_element_idx(id),
            status);

        return status;
    }

    return FWK_SUCCESS;
}

static int dvfs_handler_start(fwk_id_t id)
{
    struct dvfs_handler_device_ctx *dev_ctx;
    struct mod_dvfs_handler_opp *opp_table;
    unsigned sustain_level;
    unsigned int device_idx;
    int status;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_SUCCESS;
    }

    device_idx = fwk_id_get_element_idx(id);
    dev_ctx = &device_ctx[device_idx];
    opp_table = dev_ctx->config->opps;
    sustain_level = opp_table[dev_ctx->config->sustained_idx].level;

    /* Start by setting the sustained level */
    status = dvfs_handler_set_level(device_idx, sustain_level);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME_DEV_IDX "failed to set sustained perf level, status: %d",
            device_idx,
            status);

        return status;
    }

    /* Register callback function for performance level set */
    status = dev_ctx->transport_fch_api->transport_fch_register_callback(
        dev_ctx->config->transport_element_id,
        (uintptr_t)&dev_ctx->device_id,
        &dvfs_handler_fch_set_level_callback);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME_DEV_IDX
            "failed to register 'set-level' fastchannel callback, status: %d",
            device_idx,
            status);

        return status;
    }

    return FWK_SUCCESS;
}

/* Module description */
const struct fwk_module module_dvfs_handler = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = dvfs_handler_init,
    .element_init = dvfs_handler_element_init,
    .bind = dvfs_handler_bind,
    .start = dvfs_handler_start,
};
