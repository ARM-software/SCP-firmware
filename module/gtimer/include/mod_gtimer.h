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
