/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_EXT_CLOCK_H
#define MOD_RCAR_EXT_CLOCK_H

#include <rcar_mmap.h>

#include <mod_clock.h>
#include <mod_rcar_clock.h>

#include <fwk_element.h>

#include <stdint.h>

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * @{
 */

/*!
 * \defgroup GroupRCARExtClock External Clock
 * @{
 */

/*!
 * \brief Subsystem external clock device configuration.
 */
struct mod_rcar_ext_clock_dev_config {
    /*! If true, the driver will provide a default clock supply. */
    uint64_t clock_rate;
    /*! The clock state. */
    enum mod_clock_state clock_state;
};

/*!
 * @cond
 */

/* Device context */
struct rcar_ext_clock_dev_ctx {
    bool ext_clock_initialized;
    uint64_t clock_rate;
    const struct mod_rcar_ext_clock_dev_config *config;
    struct mod_rcar_clock_drv_api *api;
};

/* Module context */
struct rcar_ext_clock_ctx {
    struct rcar_ext_clock_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
};

/*!
 * @endcond
 */

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_EXT_CLOCK_H */
