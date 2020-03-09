/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
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
 * @{
 */

/*!
 * \defgroup GroupModuleGtimer Generic Timer Driver
 *
 * \details Driver module for the generic timer.
 *
 * @{
 */

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
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_GTIMER_H */
