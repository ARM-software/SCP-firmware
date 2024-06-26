/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Performance controller unit test stub for internal functions.
 */
#include <perf_controller.h>

#include <stdint.h>

uint32_t get_cores_min_power_limit_stub(
    struct mod_perf_controller_cluster_ctx *cluster_ctx);

int power_to_performance(
    fwk_id_t model_id,
    uint32_t power,
    uint32_t *performance_level);

int driver_set_performance_level(
    fwk_id_t domain_id,
    uintptr_t cookie,
    uint32_t performance_level);

int cluster_apply_performance_granted_stub(
    struct mod_perf_controller_cluster_ctx *cluster_ctx);
