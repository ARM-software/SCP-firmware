/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Clock Management Protocol Support
 */

#ifndef INTERNAL_SCMI_CLOCK_H
#define INTERNAL_SCMI_CLOCK_H

#include <mod_clock.h>

#define SCMI_PROTOCOL_VERSION_CLOCK UINT32_C(0x10000)

/*
 * Identifiers for the type of request being processed
 */
enum scmi_clock_request_type {
    SCMI_CLOCK_REQUEST_GET_STATE,
    SCMI_CLOCK_REQUEST_GET_RATE,
    SCMI_CLOCK_REQUEST_SET_RATE,
    SCMI_CLOCK_REQUEST_SET_STATE,
    SCMI_CLOCK_REQUEST_COUNT,
};

/*
 * Identifiers of the internal events
 */
enum scmi_clock_event_idx {
    SCMI_CLOCK_EVENT_IDX_GET_STATE,
    SCMI_CLOCK_EVENT_IDX_GET_RATE,
    SCMI_CLOCK_EVENT_IDX_SET_RATE,
    SCMI_CLOCK_EVENT_IDX_SET_STATE,
    SCMI_CLOCK_EVENT_IDX_COUNT,
};

/*
 * Container for the data when 'set_rate' operation is requested
 */
struct event_set_rate_request_data {
    uint32_t rate[2];
    enum mod_clock_round_mode round_mode;
};

/*
 * Container for the data when 'set_state' operation is requested
 */
struct event_set_state_request_data {
    enum mod_clock_state state;
};

/*
 * Container for the data when 'set_' operation is requested
 */
union event_request_data {
    struct event_set_rate_request_data set_rate_data;
    struct event_set_state_request_data set_state_data;
};

/*
 * Parameters of the event being processed
 */
struct scmi_clock_event_request_params {
    fwk_id_t clock_dev_id;
    union event_request_data request_data;
};

/*
 * Generic p2a
 */
struct scmi_clock_generic_p2a {
    int32_t status;
};

/*
 * Protocol Attributes
 */

#define SCMI_CLOCK_PROTOCOL_ATTRIBUTES_MAX_PENDING_TRANSITIONS_POS  16
#define SCMI_CLOCK_PROTOCOL_ATTRIBUTES_CLOCK_COUNT_POS              0

#define SCMI_CLOCK_PROTOCOL_ATTRIBUTES_MAX_PENDING_TRANSITIONS_MASK \
    (UINT32_C(0xFF) << \
    SCMI_CLOCK_PROTOCOL_ATTRIBUTES_MAX_PENDING_TRANSITIONS_POS)
#define SCMI_CLOCK_PROTOCOL_ATTRIBUTES_CLOCK_COUNT_MASK   \
    (UINT32_C(0xFFFF) << SCMI_CLOCK_PROTOCOL_ATTRIBUTES_CLOCK_COUNT_POS)

#define SCMI_CLOCK_PROTOCOL_ATTRIBUTES(MAX_PENDING_TRANSACTIONS, CLOCK_COUNT) \
    ( \
        ((MAX_PENDING_TRANSACTIONS << \
            SCMI_CLOCK_PROTOCOL_ATTRIBUTES_MAX_PENDING_TRANSITIONS_POS) & \
            SCMI_CLOCK_PROTOCOL_ATTRIBUTES_MAX_PENDING_TRANSITIONS_MASK) | \
        (((CLOCK_COUNT) << SCMI_CLOCK_PROTOCOL_ATTRIBUTES_CLOCK_COUNT_POS) & \
            SCMI_CLOCK_PROTOCOL_ATTRIBUTES_CLOCK_COUNT_MASK) \
    )

/*
 * Clock Attributes
 */

#define SCMI_CLOCK_ATTRIBUTES_ENABLED_POS    0

#define SCMI_CLOCK_ATTRIBUTES_ENABLED_MASK   \
    (UINT32_C(0x1) << SCMI_CLOCK_ATTRIBUTES_ENABLED_POS)

#define SCMI_CLOCK_ATTRIBUTES(ENABLED) \
    ( \
        (((ENABLED) << SCMI_CLOCK_ATTRIBUTES_ENABLED_POS) & \
            SCMI_CLOCK_ATTRIBUTES_ENABLED_MASK) \
    )

struct scmi_clock_attributes_a2p {
    uint32_t clock_id;
};

#define SCMI_CLOCK_NAME_LENGTH_MAX 16

struct scmi_clock_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    char clock_name[SCMI_CLOCK_NAME_LENGTH_MAX];
};

/*
 * Clock Rate Get
 */

struct scmi_clock_rate_get_a2p {
    uint32_t clock_id;
};

struct scmi_clock_rate_get_p2a {
    int32_t status;
    uint32_t rate[2];
};

/*
 * Clock Rate Set
 */

/* If set, set the new clock rate asynchronously */
#define SCMI_CLOCK_RATE_SET_ASYNC_POS 0U
/* If set, do not send a delayed asynchronous response */
#define SCMI_CLOCK_RATE_SET_NO_DELAYED_RESPONSE_POS 1U
/* Round up, if set, otherwise round down */
#define SCMI_CLOCK_RATE_SET_ROUND_UP_POS 2U
/* If set, the platform chooses the appropriate rounding mode */
#define SCMI_CLOCK_RATE_SET_ROUND_AUTO_POS 3U

#define SCMI_CLOCK_RATE_SET_ASYNC_MASK \
    (UINT32_C(0x1) << SCMI_CLOCK_RATE_SET_ASYNC_POS)
#define SCMI_CLOCK_RATE_SET_NO_DELAYED_RESPONSE_MASK \
    (UINT32_C(0x1) << SCMI_CLOCK_RATE_SET_NO_DELAYED_RESPONSE_POS)
#define SCMI_CLOCK_RATE_SET_ROUND_UP_MASK \
    (UINT32_C(0x1) << SCMI_CLOCK_RATE_SET_ROUND_UP_POS)
#define SCMI_CLOCK_RATE_SET_ROUND_AUTO_MASK \
    (UINT32_C(0x1) << SCMI_CLOCK_RATE_SET_ROUND_AUTO_POS)
#define SCMI_CLOCK_RATE_SET_FLAGS_MASK \
    (SCMI_CLOCK_RATE_SET_ASYNC_MASK | \
     SCMI_CLOCK_RATE_SET_NO_DELAYED_RESPONSE_MASK | \
     SCMI_CLOCK_RATE_SET_ROUND_UP_MASK | SCMI_CLOCK_RATE_SET_ROUND_AUTO_MASK)

struct scmi_clock_rate_set_a2p {
    uint32_t flags;
    uint32_t clock_id;
    uint32_t rate[2];
};

struct scmi_clock_rate_set_p2a {
    int32_t status;
};

/*
 * Clock Config Set
 */

#define SCMI_CLOCK_CONFIG_SET_ENABLE_POS 0U

#define SCMI_CLOCK_CONFIG_SET_ENABLE_MASK \
    (UINT32_C(0x1) << SCMI_CLOCK_CONFIG_SET_ENABLE_POS)

struct scmi_clock_config_set_a2p {
    uint32_t clock_id;
    uint32_t attributes;
};

struct scmi_clock_config_set_p2a {
    int32_t status;
};

/*
 * Clock Describe Rates
 */

#define SCMI_CLOCK_RATE_FORMAT_RANGE 1U
#define SCMI_CLOCK_RATE_FORMAT_LIST  0U

#define SCMI_CLOCK_NUM_OF_RATES_RANGE 3U

#define SCMI_CLOCK_DESCRIBE_RATES_REMAINING_POS 16U
#define SCMI_CLOCK_DESCRIBE_RATES_FORMAT_POS    12U
#define SCMI_CLOCK_DESCRIBE_RATES_COUNT_POS     0U

#define SCMI_CLOCK_DESCRIBE_RATES_NUM_RATES_FLAGS( \
    RATE_COUNT, RETURN_FORMAT, REMAINING_RATES) \
    ( \
        ((RATE_COUNT << \
            SCMI_CLOCK_DESCRIBE_RATES_COUNT_POS) & \
            SCMI_CLOCK_DESCRIBE_RATES_COUNT_MASK) | \
        ((REMAINING_RATES << SCMI_CLOCK_DESCRIBE_RATES_REMAINING_POS) & \
            SCMI_CLOCK_DESCRIBE_RATES_REMAINING_MASK) | \
        ((RETURN_FORMAT << SCMI_CLOCK_DESCRIBE_RATES_FORMAT_POS) & \
            SCMI_CLOCK_DESCRIBE_RATES_FORMAT_MASK) \
    )

#define SCMI_CLOCK_DESCRIBE_RATES_REMAINING_MASK \
    (UINT32_C(0xFFFF) << SCMI_CLOCK_DESCRIBE_RATES_REMAINING_POS)
#define SCMI_CLOCK_DESCRIBE_RATES_FORMAT_MASK \
    (UINT32_C(0x1) << SCMI_CLOCK_DESCRIBE_RATES_FORMAT_POS)
#define SCMI_CLOCK_DESCRIBE_RATES_COUNT_MASK \
    (UINT32_C(0xFFF) << SCMI_CLOCK_DESCRIBE_RATES_COUNT_POS)

#define SCMI_CLOCK_RATES_MAX(MAILBOX_SIZE) \
    ((sizeof(struct scmi_clock_describe_rates_p2a) < (MAILBOX_SIZE))  ? \
     (((MAILBOX_SIZE) - sizeof(struct scmi_clock_describe_rates_p2a))   \
        / sizeof(struct scmi_clock_rate)) : 0)

struct scmi_clock_rate {
    uint32_t low;
    uint32_t high;
};

struct scmi_clock_describe_rates_a2p {
    uint32_t clock_id;
    uint32_t rate_index;
};

struct scmi_clock_describe_rates_p2a {
    int32_t status;
    uint32_t num_rates_flags;
    struct scmi_clock_rate rates[];
};

#endif /* INTERNAL_SCMI_CLOCK_H */
