/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_PMIC_EVENT_PRIVATE_H
#define MOD_RCAR_PMIC_EVENT_PRIVATE_H

#include <fwk_event.h>
#include <fwk_id.h>

/* "Set enabled" event */
struct mod_rcar_pmic_event_params_set_enabled {
    bool enable;
};

/* "Set voltage" event */
struct mod_rcar_pmic_event_params_set_voltage {
    uint64_t voltage;
};

/* Event handler */
int __mod_rcar_pmic_process_event(
    const struct fwk_event *event,
    struct fwk_event *response);

#endif /* MOD_RCAR_PMIC_EVENT_PRIVATE_H */
