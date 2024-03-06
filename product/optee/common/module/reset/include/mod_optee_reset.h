/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_OPTEE_RESET_H
#define MOD_OPTEE_RESET_H

/*!
 * \brief Platform reset domain configuration.
 */
struct mod_optee_reset_dev_config {
    /*! Optee reset reference */
    struct rstctrl *rstctrl;
    /*! Reset line name */
    const char *name;
};

#endif /* MOD_OPTEE_RESET_H */
