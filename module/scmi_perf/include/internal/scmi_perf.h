/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI performance domain management protocol support.
 */

#ifndef INTERNAL_SCMI_PERF_H
#define INTERNAL_SCMI_PERF_H

#include <internal/scmi.h>

#include <mod_scmi_perf.h>

#include <fwk_event.h>
#include <fwk_id.h>

#include <stdbool.h>
#include <stdint.h>

#define SCMI_PROTOCOL_VERSION_PERF UINT32_C(0x20000)

#define SCMI_PERF_SUPPORTS_STATS_SHARED_MEM_REGION  0
#define SCMI_PERF_STATS_SHARED_MEM_REGION_ADDR_LOW  0
#define SCMI_PERF_STATS_SHARED_MEM_REGION_ADDR_HIGH 0
#define SCMI_PERF_STATS_SHARED_MEM_REGION_LENGTH    0

enum scmi_perf_notification_id {
    SCMI_PERF_LIMITS_CHANGED = 0x000,
    SCMI_PERF_LEVEL_CHANGED = 0x001,
};

/* Event indices */
enum scmi_perf_event_idx {
    SCMI_PERF_EVENT_IDX_LEVEL_GET_REQUEST,
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    SCMI_PERF_EVENT_IDX_FAST_CHANNELS_PROCESS,
#endif
    SCMI_PERF_EVENT_IDX_COUNT,
};

struct perf_operations {
    /*
     * Service identifier currently requesting operation.
     * A 'none' value means that there is no pending request.
     */
    fwk_id_t service_id;
};

struct perf_opp_table {
    /* Pointer to DVFS OPP table */
    struct mod_dvfs_opp *opps;

    /* Number of OPPs */
    size_t opp_count;

    /* The DVFS identifier for this OPP table */
    fwk_id_t dvfs_id;
};

/*!
 * \brief Domain context.
 */
struct scmi_perf_domain_ctx {
    /* Current limits */
    struct mod_scmi_perf_level_limits level_limits;

    /* Current level */
    uint32_t curr_level;

    /* Tables of OPPs */
    struct perf_opp_table *opp_table;
};

struct mod_scmi_perf_ctx {
    /* SCMI Performance Module Configuration */
    const struct mod_scmi_perf_config *config;

    /* Number of performance domains */
    uint32_t domain_count;

    /* SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /* DVFS module API */
    const struct mod_dvfs_domain_api *dvfs_api;

    struct scmi_perf_domain_ctx *domain_ctx_table;

    struct perf_opp_table *opp_table;

    unsigned int dvfs_doms_count;
};

/*
 * PROTOCOL_ATTRIBUTES
 */

#define SCMI_PERF_PROTOCOL_ATTRIBUTES_POWER_MW_POS     16
#define SCMI_PERF_PROTOCOL_ATTRIBUTES_NUM_DOMAINS_POS   0

#define SCMI_PERF_PROTOCOL_ATTRIBUTES_POWER_MW_MASK \
    (UINT32_C(0x1) << SCMI_PERF_PROTOCOL_ATTRIBUTES_POWER_MW_POS)
#define SCMI_PERF_PROTOCOL_ATTRIBUTES_NUM_DOMAINS_MASK   \
    (UINT32_C(0xFFFF) << SCMI_PERF_PROTOCOL_ATTRIBUTES_NUM_DOMAINS_POS)

#define SCMI_PERF_PROTOCOL_ATTRIBUTES(POWER_MW, NUM_DOMAINS) \
    ( \
        (((POWER_MW) << SCMI_PERF_PROTOCOL_ATTRIBUTES_POWER_MW_POS) & \
            SCMI_PERF_PROTOCOL_ATTRIBUTES_POWER_MW_MASK) | \
        (((NUM_DOMAINS) << SCMI_PERF_PROTOCOL_ATTRIBUTES_NUM_DOMAINS_POS) & \
            SCMI_PERF_PROTOCOL_ATTRIBUTES_NUM_DOMAINS_MASK) \
    )

struct scmi_perf_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    uint32_t statistics_address_low;
    uint32_t statistics_address_high;
    uint32_t statistics_len;
};

/*
 * PERFORMANCE_DOMAIN_ATTRIBUTES
 */

#define SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LIMITS_POS 31U
#define SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LEVEL_POS  30U
#define SCMI_PERF_DOMAIN_ATTRIBUTES_LIMITS_NOTIFY_POS  29U
#define SCMI_PERF_DOMAIN_ATTRIBUTES_LEVEL_NOTIFY_POS   28U
#define SCMI_PERF_DOMAIN_ATTRIBUTES_FAST_CHANNEL_POS   27U

#define SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LIMITS_MASK \
    (UINT32_C(0x1) << SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LIMITS_POS)
#define SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LEVEL_MASK \
    (UINT32_C(0x1) << SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LEVEL_POS)

#define SCMI_PERF_DOMAIN_ATTRIBUTES_LIMITS_NOTIFY_MASK \
    (UINT32_C(0x1) << SCMI_PERF_DOMAIN_ATTRIBUTES_LIMITS_NOTIFY_POS)
#define SCMI_PERF_DOMAIN_ATTRIBUTES_LEVEL_NOTIFY_MASK \
    (UINT32_C(0x1) << SCMI_PERF_DOMAIN_ATTRIBUTES_LEVEL_NOTIFY_POS)

#define SCMI_PERF_FC_MIN_RATE_LIMIT 4000U

#define SCMI_PERF_DOMAIN_ATTRIBUTES_FAST_CHANNEL_MASK \
    (UINT32_C(0x1) << SCMI_PERF_DOMAIN_ATTRIBUTES_FAST_CHANNEL_POS)

#define SCMI_PERF_DOMAIN_ATTRIBUTES_FAST_CHANNEL_MASK \
    (UINT32_C(0x1) << SCMI_PERF_DOMAIN_ATTRIBUTES_FAST_CHANNEL_POS)

#define SCMI_PERF_DOMAIN_ATTRIBUTES(LEVEL_NOTIFY, LIMITS_NOTIFY, \
                                    CAN_SET_LEVEL, CAN_SET_LIMITS, \
                                    FAST_CHANNEL) \
    ( \
        (((LEVEL_NOTIFY) << \
            SCMI_PERF_DOMAIN_ATTRIBUTES_LEVEL_NOTIFY_POS) & \
            SCMI_PERF_DOMAIN_ATTRIBUTES_LEVEL_NOTIFY_MASK) | \
        (((LIMITS_NOTIFY) << \
            SCMI_PERF_DOMAIN_ATTRIBUTES_LIMITS_NOTIFY_POS) & \
            SCMI_PERF_DOMAIN_ATTRIBUTES_LIMITS_NOTIFY_MASK) | \
        (((CAN_SET_LEVEL) << \
            SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LEVEL_POS) & \
            SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LEVEL_MASK) | \
        (((CAN_SET_LIMITS) << \
            SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LIMITS_POS) & \
            SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LIMITS_MASK) | \
        (((FAST_CHANNEL) << \
            SCMI_PERF_DOMAIN_ATTRIBUTES_FAST_CHANNEL_POS) & \
            SCMI_PERF_DOMAIN_ATTRIBUTES_FAST_CHANNEL_MASK) \
    )

struct scmi_perf_domain_attributes_a2p {
    uint32_t domain_id;
};

#define SCMI_PERF_DOMAIN_RATE_LIMIT_POS  0
#define SCMI_PERF_DOMAIN_RATE_LIMIT_MASK \
    (UINT32_C(0xFFFFF) << SCMI_PERF_DOMAIN_RATE_LIMIT_POS)

struct scmi_perf_domain_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    uint32_t rate_limit;
    uint32_t sustained_freq;
    uint32_t sustained_perf_level;
    uint8_t name[16];
};

/*
 * PERFORMANCE_DESCRIBE_LEVELS
 */

#define SCMI_PERF_LEVELS_MAX(MAILBOX_SIZE) \
    ((sizeof(struct scmi_perf_describe_levels_p2a) < MAILBOX_SIZE)  ? \
     ((MAILBOX_SIZE - sizeof(struct scmi_perf_describe_levels_p2a))   \
        / sizeof(struct scmi_perf_level)) : 0)

#define SCMI_PERF_LEVEL_ATTRIBUTES_POS  0
#define SCMI_PERF_LEVEL_ATTRIBUTES_MASK \
    (UINT32_C(0xFFFF) << SCMI_PERF_LEVEL_ATTRIBUTES_POS)

#define SCMI_PERF_LEVEL_ATTRIBUTES(LATENCY) \
    (((LATENCY) << SCMI_PERF_LEVEL_ATTRIBUTES_POS) & \
        SCMI_PERF_LEVEL_ATTRIBUTES_MASK)

struct scmi_perf_level {
    uint32_t performance_level;
    uint32_t power_cost;
    uint32_t attributes;
};

struct scmi_perf_describe_levels_a2p {
    uint32_t domain_id;
    uint32_t level_index;
};

#define SCMI_PERF_NUM_LEVELS_REMAINING_LEVELS_POS  16
#define SCMI_PERF_NUM_LEVELS_NUM_LEVELS_POS        0

#define SCMI_PERF_NUM_LEVELS_REMAINING_LEVELS_MASK \
    (UINT32_C(0xFFFF) << SCMI_PERF_NUM_LEVELS_REMAINING_LEVELS_POS)
#define SCMI_PERF_NUM_LEVELS_NUM_LEVELS_MASK \
    (UINT32_C(0xFFF) << SCMI_PERF_NUM_LEVELS_NUM_LEVELS_POS)

#define SCMI_PERF_NUM_LEVELS(NUM_LEVELS, REMAINING_LEVELS) \
    ((((NUM_LEVELS) << SCMI_PERF_NUM_LEVELS_NUM_LEVELS_POS) & \
        SCMI_PERF_NUM_LEVELS_NUM_LEVELS_MASK) | \
     (((REMAINING_LEVELS) << SCMI_PERF_NUM_LEVELS_REMAINING_LEVELS_POS) & \
        SCMI_PERF_NUM_LEVELS_REMAINING_LEVELS_MASK))

struct scmi_perf_describe_levels_p2a {
    int32_t status;
    uint32_t num_levels;

    struct scmi_perf_level perf_levels[];
};

/*
 * PERFORMANCE_LIMITS_SET
 */

struct scmi_perf_limits_set_a2p {
    uint32_t domain_id;
    uint32_t range_max;
    uint32_t range_min;
};

struct scmi_perf_limits_set_p2a {
    int32_t status;
};

/*
 * PERFORMANCE_LIMITS_GET
 */

struct scmi_perf_limits_get_a2p {
    uint32_t domain_id;
};

struct scmi_perf_limits_get_p2a {
    int32_t status;
    uint32_t range_max;
    uint32_t range_min;
};

/*
 * PERFORMANCE_LEVEL_SET
 */

struct scmi_perf_level_set_a2p {
    uint32_t domain_id;
    uint32_t performance_level;
};

struct scmi_perf_level_set_p2a {
    int32_t status;
};

/*
 * PERFORMANCE_LEVEL_GET
 */

struct scmi_perf_level_get_a2p {
    uint32_t domain_id;
};

struct scmi_perf_level_get_p2a {
    int32_t status;
    uint32_t performance_level;
};

/*
 * PERFORMANCE_NOTIFY_LIMITS
 */

#define SCMI_PERF_NOTIFY_LIMITS_NOTIFY_ENABLE_MASK UINT32_C(0x1)

struct scmi_perf_notify_limits_a2p {
    uint32_t domain_id;
    uint32_t notify_enable;
};

struct scmi_perf_notify_limits_p2a {
    int32_t status;
};

/*
 * PERFORMANCE_NOTIFY_LEVEL
 */

#define SCMI_PERF_NOTIFY_LEVEL_NOTIFY_ENABLE_MASK UINT32_C(0x1)

struct scmi_perf_notify_level_a2p {
    uint32_t domain_id;
    uint32_t notify_enable;
};

struct scmi_perf_notify_level_p2a {
    int32_t status;
};

/*
 * PERFORMANCE_LEVEL_CHANGED
 */
struct scmi_perf_level_changed {
    uint32_t agent_id;
    uint32_t domain_id;
    uint32_t performance_level;
};

/*
 * PERFORMANCE_LIMITS_CHANGED
 */
struct scmi_perf_limits_changed {
    uint32_t agent_id;
    uint32_t domain_id;
    uint32_t range_max;
    uint32_t range_min;
};

/*
 * PERFORMANCE_DESCRIBE_FASTCHANNEL
 */

struct scmi_perf_describe_fc_a2p {
    uint32_t domain_id;
    uint32_t message_id;
};

struct scmi_perf_describe_fc_p2a {
    int32_t status;
    uint32_t attributes;
    uint32_t rate_limit;
    uint32_t chan_addr_low;
    uint32_t chan_addr_high;
    uint32_t chan_size;
    uint32_t doorbell_addr_low;
    uint32_t doorbell_addr_high;
    uint32_t doorbell_set_mask_low;
    uint32_t doorbell_set_mask_high;
    uint32_t doorbell_preserve_mask_low;
    uint32_t doorbell_preserve_mask_high;
};

fwk_id_t get_dependency_id(unsigned int el_idx);

/*
 * Interface to SCMI Perf FastChannels stub
 */
struct mod_scmi_perf_private_api_perf_stub {
    int (*perf_set_level)(
        fwk_id_t domain_id,
        unsigned int agent_id,
        uint32_t perf_level);

    int (*perf_set_limits)(
        fwk_id_t domain_id,
        unsigned int agent_id,
        const struct mod_scmi_perf_level_limits *limits);

    int (*find_opp_for_level)(
        struct scmi_perf_domain_ctx *domain_ctx,
        uint32_t *level,
        bool use_nearest);

    void (*notify_limits_updated)(
        fwk_id_t domain_id,
        uint32_t range_min,
        uint32_t range_max);
};

void perf_fch_set_fch_get_level(
    const struct mod_scmi_perf_domain_config *domain,
    uint32_t level);

bool perf_fch_prot_msg_attributes_has_fastchannels(
    const struct scmi_protocol_message_attributes_a2p *parameters);

bool perf_fch_domain_attributes_has_fastchannels(
    const struct scmi_perf_domain_attributes_a2p *parameters);

int perf_fch_describe_fast_channels(
    fwk_id_t service_id,
    const uint32_t *payload);

int perf_fch_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data,
    struct mod_scmi_perf_ctx *mod_ctx,
    struct mod_scmi_perf_private_api_perf_stub *api);

int perf_fch_bind(fwk_id_t id, unsigned int round);

int perf_fch_start(fwk_id_t id);

int perf_fch_process_event(const struct fwk_event *event);

/*
 * Interface to SCMI Perf Protocol stub
 */
int perf_prot_ops_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data,
    struct mod_scmi_perf_ctx *mod_ctx,
    struct mod_scmi_perf_private_api_perf_stub *api);

int perf_prot_ops_bind(fwk_id_t id, unsigned int round);

int perf_prot_ops_start(fwk_id_t id);

void perf_prot_ops_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api);

int perf_prot_ops_process_events(
    const struct fwk_event *event,
    struct fwk_event *resp_event);

int perf_prot_ops_update_stats(fwk_id_t domain_id, uint32_t level);

void perf_prot_ops_notify_limits(
    unsigned int domain_idx,
    uint32_t range_min,
    uint32_t range_max);

void perf_prot_ops_notify_level(
    unsigned int domain_idx,
    uint32_t level,
    uint32_t cookie);

#endif /* INTERNAL_SCMI_PERF_H */
