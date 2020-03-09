/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno PVT Driver header file
 */

#ifndef MOD_JUNO_PVT_H
#define MOD_JUNO_PVT_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \ingroup GroupJunoModule
 * \defgroup GroupJunoPVT Juno PVT
 * \{
 * \brief The Juno SoC provides Power, Voltage, and Temperature (PVT) sensors
 *      that can be used to power down the chip when it exceeds the maximum
 *      operating temperature. These sensors may be located next to the cores,
 *      gpu, or distributed on the SoC, and are grouped together within the same
 *      location.
 *      The PVT sensors are calibrated during board manufacture.
 *
 * \note Only one sensor per group can be read at a time.
 */

/*!
 * \brief Sensor type indices.
 */
enum juno_sensor_pvt_type {
    /*! Temperature type PVT sensor */
    JUNO_PVT_TYPE_TEMP,

    /*! Voltage type PVT sensor */
    JUNO_PVT_TYPE_VOLT,

    /*! Type count of PVT sensors */
    JUNO_PVT_TYPE_COUNT
};

/*!
 * \brief Sensor Group identifiers.
 */
enum juno_group_pvt_id {
    /*! BIG CPU group */
    JUNO_PVT_GROUP_BIG = 0,

    /*! LITTLE CPU group */
    JUNO_PVT_GROUP_LITTLE,

    /*! GPU group */
    JUNO_PVT_GROUP_GPU,

    /*! SoC group */
    JUNO_PVT_GROUP_SOC,

    /*! Standard Cell group */
    JUNO_PVT_GROUP_STDCELL,

    /*! Number of groups */
    JUNO_PVT_GROUP_COUNT
};

/*!
 * \brief Sensor Group Descriptor.
 */
struct juno_group_desc {
    /*! Configuration register */
    struct juno_pvt_reg *regs;

    /*! Interrupt number */
    unsigned int irq;

    /*! Sensor count for the group */
    uint8_t sensor_count;

    /*! Identifier of the Power Domain associated with the sensor group */
    fwk_id_t pd_id;
};

/*!
 * \brief Sensor Descriptor.
 */
struct mod_juno_pvt_dev_config {
    /*! The group the sensor belongs to */
    const struct juno_group_desc *group;

    /*! The index of the sensor within its group */
    uint8_t index;

    /*! Sensor type */
    enum juno_sensor_pvt_type type;

    /*! Sensor information */
    struct mod_sensor_info *info;

    /*! Calibration register, first point */
    uint16_t *cal_reg_a;

    /*! Calibration register, second point */
    uint16_t *cal_reg_b;

    /*! Calibration offset, first point */
    uint16_t offset_cal_reg_a;

    /*! Calibration offset, second point */
    uint16_t offset_cal_reg_b;
};

/*!
 * \}
 */

#endif /* MOD_JUNO_PVT_H */
