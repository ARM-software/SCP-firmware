/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI power capping and monitoring protocol completer support.
 */

#ifndef INTERNAL_SCMI_POWER_CAPPING_FAST_CHANNELS_H
#define INTERNAL_SCMI_POWER_CAPPING_FAST_CHANNELS_H

#include "internal/scmi_power_capping.h"

#include <fwk_event.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMI_POWER_CAPPING_FAST_CHANNELS SCMI power capping fast
 * channels
 * \{
 */

struct pcapping_fast_channel_info {
    uint64_t fch_address;
    uint32_t fch_channel_size;
    uint32_t fch_attributes;
    uint32_t fch_rate_limit;
};

int pcapping_fast_channel_get_info(
    uint32_t domain_idx,
    uint32_t message_id,
    struct pcapping_fast_channel_info *info);

int pcapping_fast_channel_bind(void);

int pcapping_fast_channel_process_event(const struct fwk_event *event);

void pcapping_fast_channel_ctx_init(struct mod_scmi_power_capping_context *ctx);

void pcapping_fast_channel_set_domain_config(
    uint32_t domain_idx,
    const struct mod_scmi_power_capping_domain_config *config);

void pcapping_fast_channel_start(void);

bool pcapping_fast_channel_get_domain_supp(uint32_t domain_idx);

bool pcapping_fast_channel_get_msg_supp(uint32_t domain_idx);

void pcapping_fast_channel_set_power_apis(
    const struct mod_scmi_power_capping_power_apis *power_management_apis);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* INTERNAL_SCMI_POWER_CAPPING_FAST_CHANNELS_H */
