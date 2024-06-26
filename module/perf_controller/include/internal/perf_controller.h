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
    const struct mod_perf_controller_power_model_api *power_model_api;

    /*! Performance driver API. */
    const struct mod_perf_controller_drv_api *perf_driver_api;

    /*! Performance limit for the cluster. */
    uint32_t performance_limit;

    /*! Requested performance details for the cluster. */
    struct {
        /*! Requested performance level. */
        uint32_t level;
        /*! Cookie assosiated with the request. */
        uintptr_t cookie;
    } performance_request_details;

    /*! Cluster configuration. */
    const struct mod_perf_controller_cluster_config *config;

    /*! Context table of cores. */
    struct mod_perf_controller_core_ctx *core_ctx_table;

    /*! Number of cores in the cluster. */
    unsigned int core_count;
};

/*!
 * \brief Performance controller module list of internal functions pointers.
 *
 * \details Internal functions are used via a function pointer call to make the
 *          unit testing of each function easier.
 */
struct mod_perf_controller_internal_api {
    /*! Memeber function to return minimum power limit. */
    uint32_t (*get_cores_min_power_limit)(
        struct mod_perf_controller_cluster_ctx *);

    /*! Memeber function to apply performance granted to the cluster. */
    int (*cluster_apply_performance_granted)(
        struct mod_perf_controller_cluster_ctx *cluster_ctx);
};

/*!
 * \brief Module context
 */
struct mod_perf_controller_ctx {
    /*! Context table of clusters. */
    struct mod_perf_controller_cluster_ctx *cluster_ctx_table;

    /*! Number of clusters in the module. */
    unsigned int cluster_count;
};

#endif /* PERF_CONTROLLER_H_ */
