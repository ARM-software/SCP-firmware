/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_OPTEE_CLOCK_H
#define MOD_OPTEE_CLOCK_H

#include <fwk_element.h>
#include <fwk_macros.h>

#include <drivers/clk.h>

#include <stdint.h>
#include <stdbool.h>

/*!
 * \brief Platform clocks configuration.
 */
struct mod_optee_clock_config {
    /*! Clock name */
    const char *name;
    /*! Optee clock reference */
    struct clk *clk;
    /*! default state of the clock */
    bool default_enabled;
};

#endif /* MOD_OPTEE_CLOCK_H */
