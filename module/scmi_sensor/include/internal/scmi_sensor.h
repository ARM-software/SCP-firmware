/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) support.
 */

#ifndef INTERNAL_SCMI_SENSOR_H
#define INTERNAL_SCMI_SENSOR_H

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupSCMI_PERF SCMI Sensor Management Protocol
 * @{
 */

#define SCMI_PROTOCOL_ID_SENSOR      UINT32_C(0x15)
#define SCMI_PROTOCOL_VERSION_SENSOR UINT32_C(0x10000)

/*
 * Identifiers of the SCMI Sensor Management Protocol commands
 */
enum scmi_sensor_command_id {
    SCMI_SENSOR_DESCRIPTION_GET = 0x003,
    SCMI_SENSOR_CONFIG_SET      = 0x004,
    SCMI_SENSOR_TRIP_POINT_SET  = 0x005,
    SCMI_SENSOR_READING_GET     = 0x006,
};

/*
 * PROTOCOL_ATTRIBUTES
 */

struct __attribute((packed)) scmi_sensor_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    uint32_t sensor_reg_address_low;
    uint32_t sensor_reg_address_high;
    uint32_t sensor_reg_len;
};

/*
 * SENSOR_READING_GET
 */

#define SCMI_SENSOR_PROTOCOL_READING_GET_ASYNC_FLAG_MASK    (1 << 0)

struct __attribute((packed)) scmi_sensor_protocol_reading_get_a2p {
    uint32_t sensor_id;
    uint32_t flags;
};

struct __attribute((packed)) scmi_sensor_protocol_reading_get_p2a {
    int32_t status;
    uint32_t sensor_value_low;
    uint32_t sensor_value_high;
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

#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_TYPE_POS              0
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_POS   11
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_POS 22
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_INTERVAL_POS   27

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
    (int32_t)(SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MASK >> 1)
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MIN \
    (-(SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MAX + 1))

#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_MAX \
    (int32_t)(SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_INTERVAL_MASK >> 1)
#define SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_MIN \
    (-(SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_MAX + 1))

#define SCMI_SENSOR_DESC_ATTRIBUTES_HIGH(SENSOR_TYPE, UNIT_MULTIPLIER, \
                                         UPDATE_MULTIPLIER, UPDATE_INTERVAL) \
    ( \
        (((SENSOR_TYPE) << \
            SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_TYPE_POS) & \
            SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_TYPE_MASK) | \
        (((UNIT_MULTIPLIER) << \
            SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_POS) & \
            SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MASK) | \
        (((UPDATE_MULTIPLIER) << \
            SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_POS) & \
            SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_MASK) | \
        (((UPDATE_INTERVAL) << \
            SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_INTERVAL_POS) & \
            SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_INTERVAL_MASK) \
    )

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

#define SCMI_SENSOR_NAME_LEN    16

struct __attribute((packed)) scmi_sensor_desc {
    uint32_t sensor_id;
    uint32_t sensor_attributes_low;
    uint32_t sensor_attributes_high;
    char sensor_name[SCMI_SENSOR_NAME_LEN];
};

struct __attribute((packed)) scmi_sensor_protocol_description_get_a2p {
    uint32_t desc_index;
};

struct __attribute((packed)) scmi_sensor_protocol_description_get_p2a {
    int32_t status;
    uint32_t num_sensor_flags;
    struct scmi_sensor_desc sensor_desc[];
};

/* Event indices */
enum scmi_sensor_api_idx {
    SCMI_SENSOR_EVENT_IDX_REQUEST,
    SCMI_SENSOR_EVENT_IDX_COUNT,
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* INTERNAL_SCMI_SENSOR_H */
