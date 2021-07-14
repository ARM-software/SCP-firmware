/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Voltage Domain Management Protocol Support
 */

#ifndef INTERNAL_SCMI_VOLTAGE_DOMAIN_H
#define INTERNAL_SCMI_VOLTAGE_DOMAIN_H

#include <mod_voltage_domain.h>

#define SCMI_PROTOCOL_VERSION_VOLTD UINT32_C(0x10000)

/*
 * Generic p2a
 */
struct scmi_voltd_generic_p2a {
    int32_t status;
};

/*
 * Protocol Attributes
 */

#define SCMI_VOLTD_PROTOCOL_ATTRIBUTES_VOLTD_COUNT_MASK   UINT32_C(0xFFFF)
#define SCMI_VOLTD_PROTOCOL_ATTRIBUTES_VOLTD_COUNT_MAX \
    (SCMI_VOLTD_PROTOCOL_ATTRIBUTES_VOLTD_COUNT_MASK + 1)
#define SCMI_VOLTD_PROTOCOL_ATTRIBUTES(_domain_count) \
    ((_domain_count) & SCMI_VOLTD_PROTOCOL_ATTRIBUTES_VOLTD_COUNT_MASK)

/*
 * Voltage Domain Attributes
 */

struct scmi_voltd_attributes_a2p {
    uint32_t domain_id;
};

#define SCMI_VOLTD_NAME_LENGTH_MAX 16

struct scmi_voltd_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    char name[SCMI_VOLTD_NAME_LENGTH_MAX];
};

/*
 * Get voltage level of a domain
 */

struct scmi_voltd_level_get_a2p {
    uint32_t domain_id;
};

struct scmi_voltd_level_get_p2a {
    int32_t status;
    int32_t voltage_level;
};

/*
 * Set voltage level of a domain
 */

struct scmi_voltd_level_set_a2p {
    uint32_t domain_id;
    uint32_t flags;
    int32_t voltage_level;
};

struct scmi_voltd_level_set_p2a {
    int32_t status;
};

/*
 * Voltage Domain Config Set
 */
#define SCMI_VOLTD_CONFIG_MODE_TYPE_POS  0x03U
#define SCMI_VOLTD_CONFIG_MODE_TYPE_BIT  (1U << SCMI_VOLTD_CONFIG_MODE_TYPE_POS)
#define SCMI_VOLTD_CONFIG_MODE_TYPE_IMPL SCMI_VOLTD_CONFIG_MODE_TYPE_BIT
#define SCMI_VOLTD_CONFIG_MODE_ID_MASK   (SCMI_VOLTD_CONFIG_MODE_TYPE_BIT - 1U)
#define SCMI_VOLTD_CONFIG_MODE_TYPE_MASK \
    ((uint32_t) ~(SCMI_VOLTD_CONFIG_MODE_ID_MASK))

enum scmi_voltd_mode_type {
    SCMI_VOLTD_MODE_TYPE_ARCH = 0x0U,
    SCMI_VOLTD_MODE_TYPE_IMPL = 0x08U,
};

enum scmi_voltd_mode_id {
    SCMI_VOLTD_MODE_ID_OFF = 0x0U,
    SCMI_VOLTD_MODE_ID_ON = 0x07U,
};

struct scmi_voltd_config_set_a2p {
    uint32_t domain_id;
    uint32_t config;
};

struct scmi_voltd_config_set_p2a {
    int32_t status;
};

struct scmi_voltd_config_get_a2p {
    uint32_t domain_id;
};

struct scmi_voltd_config_get_p2a {
    int32_t status;
    uint32_t config;
};

/*
 * Voltage Domain Describe Levels
 */

#define SCMI_VOLTD_LEVEL_FORMAT_RANGE     1
#define SCMI_VOLTD_LEVEL_FORMAT_LIST      0

#define SCMI_VOLTD_DESCRIBE_LEVELS_REMAINING_POS    16
#define SCMI_VOLTD_DESCRIBE_LEVELS_FORMAT_POS       12
#define SCMI_VOLTD_DESCRIBE_LEVELS_COUNT_POS        0

#define SCMI_VOLTD_DESCRIBE_LEVELS_REMAINING_MASK \
    (UINT32_C(0xFFFF) << SCMI_VOLTD_DESCRIBE_LEVELS_REMAINING_POS)
#define SCMI_VOLTD_DESCRIBE_LEVELS_FORMAT_MASK \
    (UINT32_C(0x1) << SCMI_VOLTD_DESCRIBE_LEVELS_FORMAT_POS)
#define SCMI_VOLTD_DESCRIBE_LEVELS_COUNT_MASK \
    (UINT32_C(0xFFF) << SCMI_VOLTD_DESCRIBE_LEVELS_COUNT_POS)

#define SCMI_VOLTD_NUM_LEVELS_FLAGS(_count, _format, _remaining) \
    ( \
        (((_count) << SCMI_VOLTD_DESCRIBE_LEVELS_COUNT_POS) & \
         SCMI_VOLTD_DESCRIBE_LEVELS_COUNT_MASK) | \
        (((_remaining) << SCMI_VOLTD_DESCRIBE_LEVELS_REMAINING_POS) & \
         SCMI_VOLTD_DESCRIBE_LEVELS_REMAINING_MASK) | \
        (((_format) << SCMI_VOLTD_DESCRIBE_LEVELS_FORMAT_POS) & \
         SCMI_VOLTD_DESCRIBE_LEVELS_FORMAT_MASK) \
    )

#define SCMI_VOLTD_LEVEL_RANGE_FLAGS \
    SCMI_VOLTD_NUM_LEVELS_FLAGS(3, SCMI_VOLTD_LEVEL_FORMAT_RANGE, 0);

#define SCMI_VOLTD_LEVEL_LIST_FLAGS(_cnt, _rem) \
    SCMI_VOLTD_NUM_LEVELS_FLAGS((_cnt), SCMI_VOLTD_LEVEL_FORMAT_LIST, (_rem));

struct scmi_voltd_describe_levels_a2p {
    uint32_t domain_id;
    uint32_t level_index;
};

struct scmi_voltd_describe_levels_p2a {
    int32_t status;
    uint32_t flags;
    int32_t voltage[];
};

#endif /* INTERNAL_SCMI_VOLTAGE_DOMAIN_H */
