/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_DVFS_UTIL_PRIVATE_H
#define MOD_DVFS_UTIL_PRIVATE_H

#include <mod_dvfs.h>
#include <mod_dvfs_domain_api_private.h>

int __mod_dvfs_set_opp(
    const struct mod_dvfs_domain_ctx *ctx,
    const struct mod_dvfs_opp *new_opp);

int __mod_dvfs_get_current_opp(
    const struct mod_dvfs_domain_ctx *ctx,
    struct mod_dvfs_opp *opp);

#endif /* MOD_DVFS_PRIVATE_H */
