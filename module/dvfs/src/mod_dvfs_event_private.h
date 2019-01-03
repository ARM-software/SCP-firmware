/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_DVFS_EVENT_PRIVATE_H
#define MOD_DVFS_EVENT_PRIVATE_H

#include <fwk_event.h>

/* Event handler */
int __mod_dvfs_process_event(
    const struct fwk_event *event,
    struct fwk_event *response);

#endif /* MOD_DVFS_EVENT_PRIVATE_H */
