/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) support.
 */

#ifndef INTERNAL_SCMI_SENSOR_H
#define INTERNAL_SCMI_SENSOR_H

#include <stdint.h>
#ifdef BUILD_HAS_SCMI_SENSOR_V2
#    include <mod_sensor.h>

#    include <fwk_id.h>
#    include <fwk_macros.h>

#    include <stddef.h>
#    include <stdint.h>
#endif

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMI_SENSOR SCMI Sensor Management Protocol
 * \{
 */
#ifdef BUILD_HAS_SCMI_SENSOR_V2
#    define SCMI_PROTOCOL_VERSION_SENSOR UINT32_C(0x20000)
#else
#    define SCMI_PROTOCOL_VERSION_SENSOR UINT32_C(0x10000)
#endif

/*
 * PROTOCOL_ATTRIBUTES
 */

struct scmi_sensor_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    uint32_t sensor_reg_address_low;
    uint32_t sensor_reg_address_high;
    uint32_t sensor_reg_len;
};

/*
 * SENSOR_READING_GET
 */

#define SCMI_SENSOR_PROTOCOL_READING_GET_ASYNC_FLAG_MASK (1U << 0)

struct scmi_sensor_trip_point_notify_a2p {
    uint32_t sensor_id;
    uint32_t flags;
};

struct scmi_sensor_trip_point_notify_p2a {
    int32_t status;
};

struct scmi_sensor_trip_point_config_a2p {
    uint32_t sensor_id;
    uint32_t flags;
    uint32_t sensor_value_low;
    uint32_t sensor_value_high;
};

struct scmi_sensor_trip_point_config_p2a {
    int32_t status;
};

struct scmi_sensor_protocol_reading_get_a2p {
    uint32_t sensor_id;
    uint32_t flags;
};
#ifdef BUILD_HAS_SCMI_SENSOR_V2
struct scmi_sensor_protocol_reading_get_data {
    int32_t sensor_value_low;
    int32_t sensor_value_high;
    uint32_t timestamp_low;
    uint32_t timestamp_high;
};
#endif

struct scmi_sensor_protocol_reading_get_p2a {
    int32_t status;
#ifdef BUILD_HAS_SCMI_SENSOR_V2
    struct scmi_sensor_protocol_reading_get_data data[];
#else
    uint32_t sensor_value_low;
    uint32_t sensor_value_high;
#endif
};

/*
 * SENSOR TRIP POINT EVENT
 */
struct scmi_sensor_trip_point_event_p2a {
    uint32_t agent_id;
    uint32_t sensor_id;
    uint32_t trip_point_desc;
};

/*
 * SENSOR_DESCRIPTION_GET
 */

 #define SCMI_SENSOR_DESCS_MAX(MAILBOX_SIZE) \
    ((sizeof(struct scmi_sensor_protocol_description_get_p2a) < MAILBOX_SIZE) \
        ? ((MAILBOX_SIZE - \
            sizeof(struct scmi_sensor_protocol_description_get_p2a)) \
                / sizeof(struct scmi_sensor_desc)) \
        : 0)

/*
 * SENSOR_AXIS_DESCRIPTION_GET
 */

#define SCMI_SENSOR_AXIS_DESCS_MAX(MAILBOX_SIZE) \
    ((sizeof(struct scmi_sensor_axis_description_get_p2a) < MAILBOX_SIZE) ? \
         ((MAILBOX_SIZE - \
           sizeof(struct scmi_sensor_axis_description_get_p2a)) / \
          sizeof(struct scmi_sensor_axis_desc)) : \
         0)

/*
 * SENSOR_READ_GET_AXIS_VALUE
 */

#define SCMI_SENSOR_READ_GET_VALUES_MAX(MAILBOX_SIZE) \
    ((sizeof(struct scmi_sensor_protocol_reading_get_p2a) < MAILBOX_SIZE) ? \
         ((MAILBOX_SIZE - \
           sizeof(struct scmi_sensor_protocol_reading_get_p2a)) / \
          sizeof(struct scmi_sensor_protocol_reading_get_data)) : \
         0)

#define SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_ASYNC_READING_POS 31
#define SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_EXTENDED_ATTRIBS_POS 8
#define SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_NUM_TRIP_POINTS_POS 0

#define SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_ASYNC_READING_MASK \
    (UINT32_C(0x1) << SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_ASYNC_READING_POS)
#define SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_EXTENDED_ATTRIBS_MASK \
    (UINT32_C(0x1) << SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_EXTENDED_ATTRIBS_POS)
#define SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_NUM_TRIP_POINTS_MASK \
    (UINT32_C(0xFF) << SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_NUM_TRIP_POINTS_POS)

#ifdef BUILD_HAS_SCMI_SENSOR_V2
#    define SCMI_SENSOR_DESC_ATTRIBUTES_LOW( \
        ASYNC_READING, EXTENDED_ATTRIBUTES, NUM_TRIP_POINTS) \
        ( \
            (((ASYNC_READING) \
              << SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_ASYNC_READING_POS) & \
             SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_ASYNC_READING_MASK) | \
            (((EXTENDED_ATTRIBUTES) \
              << SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_EXTENDED_ATTRIBS_POS) & \
             SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_EXTENDED_ATTRIBS_MASK) | \
            (((NUM_TRIP_POINTS) \
              << SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_NUM_TRIP_POINTS_POS) & \
             SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_NUM_TRIP_POINTS_MASK))
#else
#    define SCMI_SENSOR_DESC_ATTRIBUTES_LOW(ASYNC_READING, NUM_TRIP_POINTS) \
        ( \
            (((ASYNC_READING) \
              << SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_ASYNC_READING_POS) & \
             SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_ASYNC_READING_MASK) | \
            (((NUM_TRIP_POINTS) \
              << SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_NUM_TRIP_POINTS_POS) & \
             SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_NUM_TRIP_POINTS_MASK))
#endif

#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_TYPE_POS              0U
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_POS   11U
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_POS 22U
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_INTERVAL_POS   27U

#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_TYPE_MASK \
    (UINT32_C(0xFF) << SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_TYPE_POS)
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MASK \
    (UINT32_C(0x1F) << SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_POS)
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_MASK \
    (UINT32_C(0x1F) \
        << SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_POS)
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_INTERVAL_MASK \
    (UINT32_C(0x1F) << SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_INTERVAL_POS)

#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MAX \
    (int32_t)(SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MASK >> 1U)
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MIN \
    (-(SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MAX + 1))

#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_MAX \
    (int32_t)(SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_INTERVAL_MASK >> 1U)
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_MIN \
    (-(SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_MAX + 1))

#define SCMI_SENSOR_DESC_ATTRIBUTES_HIGH( \
    SENSOR_TYPE, UNIT_MULTIPLIER, UPDATE_MULTIPLIER, UPDATE_INTERVAL) \
    (((((unsigned int)SENSOR_TYPE) \
       << SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_TYPE_POS) & \
      SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_TYPE_MASK) | \
     ((((unsigned int)UNIT_MULTIPLIER) \
       << SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_POS) & \
      SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MASK) | \
     ((((unsigned int)UPDATE_MULTIPLIER) \
       << SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_POS) & \
      SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_MASK) | \
     ((((unsigned int)UPDATE_INTERVAL) \
       << SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_INTERVAL_POS) & \
      SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_INTERVAL_MASK))

#define SCMI_SENSOR_NUM_SENSOR_FLAGS_NUM_DESCS_POS              0
#define SCMI_SENSOR_NUM_SENSOR_FLAGS_NUM_REMAINING_DESCS_POS   16

#define SCMI_SENSOR_NUM_SENSOR_FLAGS_NUM_DESCS_MASK \
    (UINT32_C(0xFFF) << SCMI_SENSOR_NUM_SENSOR_FLAGS_NUM_DESCS_POS)
#define SCMI_SENSOR_NUM_SENSOR_FLAGS_NUM_REMAINING_DESCS_MASK \
    (UINT32_C(0xFFFF) << SCMI_SENSOR_NUM_SENSOR_FLAGS_NUM_REMAINING_DESCS_POS)

#define SCMI_SENSOR_NUM_SENSOR_FLAGS(NUM_DESCS, NUM_REMAINING_DESCS) \
    ( \
        (((NUM_DESCS) << \
            SCMI_SENSOR_NUM_SENSOR_FLAGS_NUM_DESCS_POS) & \
            SCMI_SENSOR_NUM_SENSOR_FLAGS_NUM_DESCS_MASK) | \
        (((NUM_REMAINING_DESCS) << \
            SCMI_SENSOR_NUM_SENSOR_FLAGS_NUM_REMAINING_DESCS_POS) & \
            SCMI_SENSOR_NUM_SENSOR_FLAGS_NUM_REMAINING_DESCS_MASK) \
    )

#define SCMI_SENSOR_CONFIG_FLAGS_EVENT_CONTROL_POS 0
#define SCMI_SENSOR_CONFIG_FLAGS_EVENT_CONTROL_MASK \
    (UINT32_C(0x1) << SCMI_SENSOR_CONFIG_FLAGS_EVENT_CONTROL_POS)

#define SCMI_SENSOR_NAME_LEN    16
#define SCMI_SENSOR_AXIS_NAME_LEN 16

#define SCMI_SENSOR_TRIP_POINT_FLAGS_RESERVED1_POS 12
#define SCMI_SENSOR_TRIP_POINT_FLAGS_ID_POS 4
#define SCMI_SENSOR_TRIP_POINT_FLAGS_RESERVED2_POS 2
#define SCMI_SENSOR_TRIP_POINT_FLAGS_EV_CTRL_POS 0

#define SCMI_SENSOR_TRIP_POINT_FLAGS_RESERVED1_MASK \
    (UINT32_C(0xFFFFF) << SCMI_SENSOR_TRIP_POINT_FLAGS_RESERVED1_POS)
#define SCMI_SENSOR_TRIP_POINT_FLAGS_ID_MASK \
    (UINT32_C(0xFF) << SCMI_SENSOR_TRIP_POINT_FLAGS_ID_POS)
#define SCMI_SENSOR_TRIP_POINT_FLAGS_RESERVED2_MASK \
    (UINT32_C(0x3) << SCMI_SENSOR_TRIP_POINT_FLAGS_RESERVED2_POS)
#define SCMI_SENSOR_TRIP_POINT_FLAGS_EV_CTRL_MASK \
    (UINT32_C(0x3) << SCMI_SENSOR_TRIP_POINT_FLAGS_EV_CTRL_POS)

#define SCMI_SENSOR_TRIP_POINT_EVENT_DESC_DIRECTION_POS 16
#define SCMI_SENSOR_TRIP_POINT_EVENT_DESC_ID_POS 0

#define SCMI_SENSOR_TRIP_POINT_EVENT_DESC_DIRECTION_MASK \
    (UINT32_C(0x1) << SCMI_SENSOR_TRIP_POINT_EVENT_DESC_DIRECTION_POS)
#define SCMI_SENSOR_TRIP_POINT_EVENT_DESC_ID_MASK \
    (UINT32_C(0xFF) << SCMI_SENSOR_TRIP_POINT_EVENT_DESC_ID_POS)

#define SCMI_SENSOR_TRIP_POINT_EVENT_DESC(DIRECTION, ID) \
    ((((DIRECTION) << SCMI_SENSOR_TRIP_POINT_EVENT_DESC_DIRECTION_POS) & \
      SCMI_SENSOR_TRIP_POINT_EVENT_DESC_DIRECTION_MASK) | \
     (((ID) << SCMI_SENSOR_TRIP_POINT_EVENT_DESC_ID_POS) & \
      SCMI_SENSOR_TRIP_POINT_EVENT_DESC_ID_MASK))

#ifdef BUILD_HAS_SCMI_SENSOR_V2
#    define SCMI_SENSOR_AXIS_DESC_ATTRIBUTES_LOW(EXTENDED_ATTRIBUTES) \
        (((EXTENDED_ATTRIBUTES) \
          << SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_EXTENDED_ATTRIBS_POS) & \
         SCMI_SENSOR_DESC_ATTRS_LOW_SENSOR_EXTENDED_ATTRIBS_MASK)
#endif

#define SCMI_SENSOR_AXIS_DESC_ATTRIBUTES_HIGH(SENSOR_TYPE, UNIT_MULTIPLIER) \
    (((((unsigned int)SENSOR_TYPE) \
       << SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_TYPE_POS) & \
      SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_TYPE_MASK) | \
     ((((unsigned int)UNIT_MULTIPLIER) \
       << SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_POS) & \
      SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MASK))

struct scmi_sensor_desc {
    uint32_t sensor_id;
    uint32_t sensor_attributes_low;
    uint32_t sensor_attributes_high;
    char sensor_name[SCMI_SENSOR_NAME_LEN];
#ifdef BUILD_HAS_SCMI_SENSOR_V2
    uint32_t sensor_power;
    uint32_t sensor_resolution;
    int32_t sensor_min_range_low;
    int32_t sensor_min_range_high;
    int32_t sensor_max_range_low;
    int32_t sensor_max_range_high;
#endif
};

struct scmi_sensor_protocol_description_get_a2p {
    uint32_t desc_index;
};

struct scmi_sensor_protocol_description_get_p2a {
    int32_t status;
    uint32_t num_sensor_flags;
    struct scmi_sensor_desc sensor_desc[];
};

struct scmi_sensor_axis_desc {
    uint32_t axis_idx;
    uint32_t axis_attributes_low;
    uint32_t axis_attributes_high;
    char axis_name[SCMI_SENSOR_AXIS_NAME_LEN];
#ifdef BUILD_HAS_SCMI_SENSOR_V2
    uint32_t axis_resolution;
    int32_t axis_min_range_low;
    int32_t axis_min_range_high;
    int32_t axis_max_range_low;
    int32_t axis_max_range_high;
#endif
};

struct scmi_sensor_axis_description_get_a2p {
    uint32_t sensor_idx;
    uint32_t axis_desc_index;
};

struct scmi_sensor_axis_description_get_p2a {
    int32_t status;
    uint32_t num_axis_flags;
    struct scmi_sensor_axis_desc axis_desc[];
};

/* Event indices */
enum scmi_sensor_event_idx {
    SCMI_SENSOR_EVENT_IDX_REQUEST,
    SCMI_SENSOR_EVENT_IDX_COUNT,
};

/* SCMI sensor notifications indices */
enum scmi_sensor_notification_id { SCMI_SENSOR_TRIP_POINT_EVENT = 0x0 };

#ifdef BUILD_HAS_SCMI_SENSOR_V2
void scmi_sensor_prop_set(
    struct mod_sensor_complete_info *info,
    struct scmi_sensor_desc *desc);

void scmi_sensor_axis_prop_set(
    struct mod_sensor_axis_info *axis_values,
    struct scmi_sensor_axis_desc *desc);

/* These values are the defaults taken from the SCMI spec V3.0 */
#    define SCMI_SENSOR_EXT_ATTR_POWER          0
#    define SCMI_SENSOR_EXT_ATTR_RESOLUTION_VAL 0
#    define SCMI_SENSOR_EXT_ATTR_MIN_RANGE_LOW  0
#    define SCMI_SENSOR_EXT_ATTR_MIN_RANGE_HIGH 0x80000000
#    define SCMI_SENSOR_EXT_ATTR_MAX_RANGE_LOW  0xFFFFFFFF
#    define SCMI_SENSOR_EXT_ATTR_MAX_RANGE_HIGH 0x7FFFFFFF
#endif

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* INTERNAL_SCMI_SENSOR_H */
