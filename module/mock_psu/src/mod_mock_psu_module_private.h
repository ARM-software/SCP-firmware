/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MOCK_PSU_MODULE_PRIVATE_H
#define MOD_MOCK_PSU_MODULE_PRIVATE_H

#include <stdbool.h>
#include <fwk_id.h>

struct mod_mock_psu_device_ctx {
    bool enabled; /* Current enabled state */
    uint64_t voltage; /* Current voltage (in mV) */
};

struct mod_mock_psu_device_ctx *__mod_mock_psu_get_valid_device_ctx(
    fwk_id_t device_id);

#endif /* MOD_MOCK_PSU_MODULE_PRIVATE_H */
