/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_PMIC_DEVICE_CTX_PRIVATE_H
#define MOD_RCAR_PMIC_DEVICE_CTX_PRIVATE_H

#include <mod_rcar_pmic.h>
#include <mod_psu.h>

#include <fwk_id.h>

/* Device context */
struct mod_rcar_pmic_device_ctx {
    /* Device configuration */
    const struct mod_rcar_pmic_device_config *config;

    struct {
        /* Driver API */
        const struct mod_rcar_pmic_driver_api *driver;
    } apis;
};

struct mod_rcar_pmic_device_ctx *__mod_rcar_pmic_get_device_ctx(
    fwk_id_t device_id);
struct mod_rcar_pmic_device_ctx *__mod_rcar_pmic_get_valid_device_ctx(
    fwk_id_t device_id);

#endif /* MOD_RCAR_PMIC_DEVICE_CTX_PRIVATE_H */
