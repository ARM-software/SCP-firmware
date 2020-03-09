/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI performance domain management protocol support.
 */

#ifndef INTERNAL_SCMI_PERF_H
#define INTERNAL_SCMI_PERF_H

#include <stdint.h>

#define SCMI_PROTOCOL_ID_PERF      UINT32_C(0x13)
#define SCMI_PROTOCOL_VERSION_PERF UINT32_C(0x10000)

#define SCMI_PERF_SUPPORTS_STATS_SHARED_MEM_REGION  0
#define SCMI_PERF_STATS_SHARED_MEM_REGION_ADDR_LOW  0
#define SCMI_PERF_STATS_SHARED_MEM_REGION_ADDR_HIGH 0
#define SCMI_PERF_STATS_SHARED_MEM_REGION_LENGTH    0

/*
 * Identifier of the SCMI Performance Domain Management Protocol commands
 */

enum scmi_perf_command_id {
    SCMI_PERF_DOMAIN_ATTRIBUTES = 0x003,
    SCMI_PERF_DESCRIBE_LEVELS   = 0x004,
    SCMI_PERF_LIMITS_SET        = 0x005,
    SCMI_PERF_LIMITS_GET        = 0x006,
    SCMI_PERF_LEVEL_SET         = 0x007,
    SCMI_PERF_LEVEL_GET         = 0x008,
    SCMI_PERF_NOTIFY_LIMITS     = 0x009,
    SCMI_PERF_NOTIFY_LEVEL      = 0x00A
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

struct __attribute((packed)) scmi_perf_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    uint32_t statistics_address_low;
    uint32_t statistics_address_high;
    uint32_t statistics_len;
};

/*
 * PERFORMANCE_DOMAIN_ATTRIBUTES
 */

#define SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LIMITS_POS 31
#define SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LEVEL_POS  30
#define SCMI_PERF_DOMAIN_ATTRIBUTES_LIMITS_NOTIFY_POS  29
#define SCMI_PERF_DOMAIN_ATTRIBUTES_LEVEL_NOTIFY_POS   28

#define SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LIMITS_MASK \
    (UINT32_C(0x1) << SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LIMITS_POS)
#define SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LEVEL_MASK \
    (UINT32_C(0x1) << SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LEVEL_POS)
#define SCMI_PERF_DOMAIN_ATTRIBUTES_LIMITS_NOTIFY_MASK \
    (UINT32_C(0x1) << SCMI_PERF_DOMAIN_ATTRIBUTES_LIMITS_NOTIFY_POS)
#define SCMI_PERF_DOMAIN_ATTRIBUTES_LEVEL_NOTIFY_MASK \
    (UINT32_C(0x1) << SCMI_PERF_DOMAIN_ATTRIBUTES_LEVEL_NOTIFY_POS)

#define SCMI_PERF_DOMAIN_ATTRIBUTES(LEVEL_NOTIFY, LIMITS_NOTIFY, \
                                    CAN_SET_LEVEL, CAN_SET_LIMITS) \
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
            SCMI_PERF_DOMAIN_ATTRIBUTES_CAN_SET_LIMITS_MASK) \
    )

struct __attribute((packed)) scmi_perf_domain_attributes_a2p {
    uint32_t domain_id;
};

#define SCMI_PERF_DOMAIN_RATE_LIMIT_POS  0
#define SCMI_PERF_DOMAIN_RATE_LIMIT_MASK \
    (UINT32_C(0xFFFFF) << SCMI_PERF_DOMAIN_RATE_LIMIT_POS)

struct __attribute((packed)) scmi_perf_domain_attributes_p2a {
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

struct __attribute((packed)) scmi_perf_level {
    uint32_t performance_level;
    uint32_t power_cost;
    uint32_t attributes;
};

struct __attribute((packed)) scmi_perf_describe_levels_a2p {
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

struct __attribute((packed)) scmi_perf_describe_levels_p2a {
    int32_t status;
    uint32_t num_levels;

    struct scmi_perf_level perf_levels[];
};

/*
 * PERFORMANCE_LIMITS_SET
 */

struct __attribute((packed)) scmi_perf_limits_set_a2p {
    uint32_t domain_id;
    uint32_t range_max;
    uint32_t range_min;
};

struct __attribute((packed)) scmi_perf_limits_set_p2a {
    int32_t status;
};

/*
 * PERFORMANCE_LIMITS_GET
 */

struct __attribute((packed)) scmi_perf_limits_get_a2p {
    uint32_t domain_id;
};

struct __attribute((packed)) scmi_perf_limits_get_p2a {
    int32_t status;
    uint32_t range_max;
    uint32_t range_min;
};

/*
 * PERFORMANCE_LEVEL_SET
 */

struct __attribute((packed)) scmi_perf_level_set_a2p {
    uint32_t domain_id;
    uint32_t performance_level;
};

struct __attribute((packed)) scmi_perf_level_set_p2a {
    int32_t status;
};

/*
 * PERFORMANCE_LEVEL_GET
 */

struct __attribute((packed)) scmi_perf_level_get_a2p {
    uint32_t domain_id;
};

struct __attribute((packed)) scmi_perf_level_get_p2a {
    int32_t status;
    uint32_t performance_level;
};

/*
 * PERFORMANCE_NOTIFY_LIMITS
 */

struct __attribute((packed)) scmi_perf_notify_limits_a2p {
    uint32_t domain_id;
    uint32_t notify_enable;
};

struct __attribute((packed)) scmi_perf_notify_limits_p2a {
    int32_t status;
};

/*
 * PERFORMANCE_NOTIFY_LEVEL
 */

struct __attribute((packed)) scmi_perf_notify_level_a2p {
    uint32_t domain_id;
    uint32_t notify_enable;
};

struct __attribute((packed)) scmi_perf_notify_level_p2a {
    int32_t status;
};

#endif /* INTERNAL_SCMI_PERF_H */
