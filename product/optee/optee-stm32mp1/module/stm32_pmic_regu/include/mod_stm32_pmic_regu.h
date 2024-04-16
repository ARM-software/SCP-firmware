/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_STPMIC1_REGU_H
#define MOD_STPMIC1_REGU_H

#include <fwk_element.h>
#include <fwk_macros.h>

#include <stdbool.h>

/*!
 * \brief Platform regulator configuration.
 */
struct mod_stm32_pmic_regu_dev_config {
    const char *regu_name;
    bool read_only;
};

#endif /* MOD_STPMIC1_REGU_H */
