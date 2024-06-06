/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PERF_CONTROLLER_H_
#define PERF_CONTROLLER_H_

#include <mod_perf_controller.h>

#include <fwk_id.h>

/*!
 * \brief Performance controller core context.
 *
 * \details The performance controller core context is responsible for storing
 *      the power limit for a corresponding core.
 */
struct mod_perf_controller_core_ctx {
    uint32_t power_limit;
};

/*!
 * \brief Performance controller cluster context.
 *
 * \details The performance controller cluster is responsible for storing the
 *      aggregate information about the cores.
 */
struct mod_perf_controller_cluster_ctx {
    /*! Power model that converts from a power quantity to performance level. */
    const struct mod_perf_controller_power_model_driver_api *model_api;

    /*! Performance driver API. */
    const struct mod_perf_controller_drv_api *perf_driver_api;

    /*! Performance limit for the cluster. */
    uint32_t performance_limit;

    /*! Requested performance level for the cluster. */
    uint32_t req_performance_level;

    /*! Cluster configuration */
    const struct mod_perf_controller_cluster_config *config;

    /*! Context table of cores */
    struct mod_perf_controller_core_ctx *core_ctx_table;
};

#endif /* PERF_CONTROLLER_H_ */
