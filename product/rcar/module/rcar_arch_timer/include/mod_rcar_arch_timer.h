/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_ARCH_TIMER_H
#define MOD_RCAR_ARCH_TIMER_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * @{
 */

/*!
 * \defgroup GroupRCARArchTimer Arch Timer Driver
 *
 * \details Driver module for the arch timer.
 *
 * @{
 */

/*!
 * \brief Arch timer device descriptor
 */
struct mod_arch_timer_dev_config {
    /*! Identifier of the clock that this device depends on */
    fwk_id_t clock_id;
};

/*!
 * \brief Get the framework time driver for a arch timer device.
 *
 * \details This function is intended to be used by a firmware to register a
 *      arch timer as the driver for the framework time component.
 *
 * \param[out] ctx Pointer to storage for the context passed to the driver.
 * \param[in] cfg Arch timer configuration.
 *
 * \return Framework time driver for the given device.
 */
struct fwk_time_driver mod_arch_timer_driver(
    const void **ctx,
    const struct mod_arch_timer_dev_config *cfg);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_ARCH_TIMER_H */
