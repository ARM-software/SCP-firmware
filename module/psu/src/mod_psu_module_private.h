/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_PSU_DEVICE_CTX_PRIVATE_H
#define MOD_PSU_DEVICE_CTX_PRIVATE_H

#include <fwk_id.h>
#include <mod_psu.h>

/* Device context */
struct mod_psu_device_ctx {
    /* Device configuration */
    const struct mod_psu_device_config *config;

    struct {
        /* Driver API */
        const struct mod_psu_driver_api *driver;
    } apis;
};

struct mod_psu_device_ctx *__mod_psu_get_device_ctx(fwk_id_t device_id);
struct mod_psu_device_ctx *__mod_psu_get_valid_device_ctx(fwk_id_t device_id);

#endif /* MOD_PSU_DEVICE_CTX_PRIVATE_H */
