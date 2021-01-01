/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_RESOURCE_PERMS_H
#define CONFIG_RESOURCE_PERMS_H

enum juno_res_perms_devices {
    JUNO_RES_PERMS_DEVICES_CPU = 0,
    JUNO_RES_PERMS_DEVICES_GPU,
    JUNO_RES_PERMS_DEVICES_IO,
    JUNO_RES_PERMS_DEVICES_COUNT
};

#endif /* CONFIG_RESOURCE_PERMS_H */
