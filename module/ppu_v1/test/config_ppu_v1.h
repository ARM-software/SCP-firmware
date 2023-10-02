/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Power unit test configuration.
 */

#include <mod_ppu_v1.h>

static struct mod_ppu_v1_config ppu_v1_config_data_ut = {
    .num_of_cores_in_cluster = 2,
};
