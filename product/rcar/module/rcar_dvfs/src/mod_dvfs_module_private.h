/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_DVFS_MODULE_PRIVATE_H
#define MOD_DVFS_MODULE_PRIVATE_H

#include <mod_clock.h>
#include <mod_rcar_pmic.h>

#include <fwk_id.h>

/* Domain context */
struct mod_dvfs_domain_ctx {
    /* Domain configuration */
    const struct mod_dvfs_domain_config *config;

    struct {
        /* Power supply API */
        const struct mod_rcar_pmic_device_api *psu;

        /* Clock API */
        const struct mod_clock_api *clock;
    } apis;

    /* Number of operating points */
    size_t opp_count;

    /* Operating point prior to domain suspension */
    struct mod_dvfs_opp suspended_opp;

    /* Current operating point limits */
    struct mod_dvfs_frequency_limits frequency_limits;
};

struct mod_dvfs_domain_ctx *__mod_dvfs_get_valid_domain_ctx(fwk_id_t domain_id);

#endif /* MOD_DVFS_MODULE_PRIVATE_H */
