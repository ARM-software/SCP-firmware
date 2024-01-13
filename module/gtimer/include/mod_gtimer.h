/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Generic Timer device driver module and definitions.
 */

#ifndef MOD_GTIMER_H
#define MOD_GTIMER_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupModuleGtimer Generic Timer Driver
 *
 * \details Driver module for the generic timer.
 *
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_gtimer_api_idx {
    /*! Interface for timer driver */
    MOD_GTIMER_API_IDX_DRIVER,
    /*! Number of defined interfaces */
    MOD_GTIMER_API_IDX_COUNT,
};

/*!
 * \brief Implementation specific system counter register configuration
 *
 * \details The system counter register frame includes implementation specific
 *      registers from offset 0xC0 to 0xFC as defined in the Arm Architecture
 *      Reference Manual. This data structure specifies an offset/value pair for
 *      the impdef registers and can be instantiated multiple times as a
 *      sequential table. This table is iterated each time the system counter
 *      is enabled and the values are written at the specified offsets. Each
 *      impdef register in this offset range is treated as a 32-bit register.
 */
struct mod_gtimer_syscounter_impdef_config {
    /*! Offset of the impdef register in system counter register frame */
    const uint8_t offset;

    /*! Value to be written to that impdef register offset */
    const uint32_t value;
};

/*!
 * \brief Generic timer device descriptor
 */
struct mod_gtimer_dev_config {
    /*! Address of the device's timer register */
    uintptr_t hw_timer;

    /*! Address of the device's counter register */
    uintptr_t hw_counter;

    /*! Address of the device's control register */
    uintptr_t control;

    /*! The frequency in Hertz that the timer ticks at */
    const uint32_t frequency;

    /*! Identifier of the clock that this device depends on */
    fwk_id_t clock_id;

    /*! Skip initialisation of CNTCONTROL register */
    bool skip_cntcontrol_init;

    /*! List of implementation specific registers to be configured */
    struct mod_gtimer_syscounter_impdef_config *syscnt_impdef_cfg;

    /*! Number of entries in the system counter impdef register config table */
    uint8_t syscnt_impdef_cfg_cnt;
};

/*!
 * \brief Get the framework time driver for a generic timer device.
 *
 * \details This function is intended to be used by a firmware to register a
 *      generic timer as the driver for the framework time component.
 *
 * \param[out] ctx Pointer to storage for the context passed to the driver.
 * \param[in] cfg Generic timer configuration.
 *
 * \return Framework time driver for the given device.
 */
struct fwk_time_driver mod_gtimer_driver(
    const void **ctx,
    const struct mod_gtimer_dev_config *cfg);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_GTIMER_H */
