/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef BUILD_HAS_SCMI_SENSOR_V2

#    include <internal/scmi_sensor.h>

#    include <mod_scmi.h>
#    include <mod_scmi_sensor.h>
#    include <mod_sensor.h>

#    include <fwk_assert.h>
#    include <fwk_id.h>
#    include <fwk_status.h>

#    include <stdbool.h>
#    include <stddef.h>
#    include <stdint.h>

void scmi_sensor_prop_set(
    struct mod_sensor_complete_info *info,
    struct scmi_sensor_desc *desc)
{
    if (info->hal_info.ext_attributes && !info->multi_axis.support) {
        desc->sensor_power = info->hal_info.sensor_properties.sensor_power;
        desc->sensor_resolution = info->hal_info.sensor_properties
                                      .sensor_property_vals.axis_resolution;
        desc->sensor_min_range_low = (int32_t)(
            info->hal_info.sensor_properties.sensor_property_vals
                .axis_min_range &
            0xffffffff);
        desc->sensor_min_range_high = (int32_t)(
            info->hal_info.sensor_properties.sensor_property_vals
                .axis_min_range >>
            32);
        desc->sensor_max_range_low = (int32_t)(
            info->hal_info.sensor_properties.sensor_property_vals
                .axis_max_range &
            0xffffffff);
        desc->sensor_min_range_high = (int32_t)(
            info->hal_info.sensor_properties.sensor_property_vals
                .axis_max_range >>
            32);
    } else {
        /* Populate with the default values from the SCMI specification V3.0 */
        desc->sensor_power = SCMI_SENSOR_EXT_ATTR_POWER;
        desc->sensor_resolution = SCMI_SENSOR_EXT_ATTR_RESOLUTION_VAL;
        desc->sensor_min_range_low = SCMI_SENSOR_EXT_ATTR_MIN_RANGE_LOW;
        desc->sensor_min_range_high = SCMI_SENSOR_EXT_ATTR_MIN_RANGE_HIGH;
        desc->sensor_max_range_low = SCMI_SENSOR_EXT_ATTR_MAX_RANGE_LOW;
        desc->sensor_max_range_high = SCMI_SENSOR_EXT_ATTR_MAX_RANGE_HIGH;
    }
}

void scmi_sensor_axis_prop_set(
    struct mod_sensor_axis_info *axis_values,
    struct scmi_sensor_axis_desc *desc)
{
    if (axis_values->extended_attribs) {
        desc->axis_resolution =
            axis_values->multi_axis_properties.axis_resolution;
        desc->axis_min_range_low = (int32_t)(
            axis_values->multi_axis_properties.axis_min_range & 0xffffffff);
        desc->axis_min_range_high =
            (int32_t)(axis_values->multi_axis_properties.axis_min_range >> 32);
        desc->axis_max_range_low = (int32_t)(
            axis_values->multi_axis_properties.axis_max_range & 0xffffffff);
        desc->axis_max_range_high =
            (int32_t)(axis_values->multi_axis_properties.axis_max_range >> 32);
    } else {
        /* Populate with the default values from the SCMI specification V3.0 */
        desc->axis_resolution = SCMI_SENSOR_EXT_ATTR_RESOLUTION_VAL;
        desc->axis_min_range_low = SCMI_SENSOR_EXT_ATTR_MIN_RANGE_LOW;
        desc->axis_min_range_high = SCMI_SENSOR_EXT_ATTR_MIN_RANGE_HIGH;
        desc->axis_max_range_low = SCMI_SENSOR_EXT_ATTR_MAX_RANGE_LOW;
        desc->axis_max_range_high = SCMI_SENSOR_EXT_ATTR_MAX_RANGE_HIGH;
    }
}

#endif
