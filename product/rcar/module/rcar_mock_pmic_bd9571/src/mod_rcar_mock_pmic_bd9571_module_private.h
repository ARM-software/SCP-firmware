/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_MOCK_PMIC_BD9571_MODULE_PRIVATE_H
#define MOD_RCAR_MOCK_PMIC_BD9571_MODULE_PRIVATE_H

#include <fwk_id.h>

#include <stdbool.h>

struct mod_rcar_mock_pmic_device_ctx {
    bool enabled; /* Current enabled state */
    uint64_t voltage; /* Current voltage (in mV) */
};

struct mod_rcar_mock_pmic_device_ctx *__mod_rcar_mock_pmic_get_valid_device_ctx(
    fwk_id_t device_id);

#endif /* MOD_RCAR_MOCK_PMIC_BD9571_MODULE_PRIVATE_H */
