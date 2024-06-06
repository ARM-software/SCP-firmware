/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_PERF_CONTROLLER_H_
#define MOD_PERF_CONTROLLER_H_

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupPERF_CONTROLLER performance controller
 * \{
 */

/*!
 * \brief Core performance API.
 */
struct mod_perf_controller_perf_api {
    /*!
     * \brief Set performance level for a controller.
     *
     * \param cluster_id Cluster identifier.
     * \param performance_level Desirable performance level.
     *
     * \retval ::FWK_E_ACCESS Wrong id.
     * \retval ::FWK_SUCCESS If the call is successful.
     * \return One of the standard framework error codes.
     */
    int (*set_performance_level)(
        fwk_id_t cluster_id,
        uintptr_t cookie,
        uint32_t performance_level);
};

/*!
 * \brief Performance driver interface.
 */
struct mod_perf_controller_drv_api {
    /*! Name of the driver */
    const char *name;

    /*!
     * \brief Set performance level for a performance domain.
     *
     * \param domain_id Domain identifier.
     * \param performance_level Desirable performance level.
     *
     * \retval ::FWK_E_ACCESS Wrong id.
     * \retval ::FWK_SUCCESS If the call is successful.
     * \return One of the standard framework error codes.
     */
    int (*set_performance_level)(
        fwk_id_t domain_id,
        uintptr_t cookie,
        uint32_t performance_level);
};

/*!
 * \brief Cluster update API.
 */
struct mod_perf_controller_cluster_update_api {
    /*!
     * \brief Provides the means to update the output performance level after
     *      setting the power limits.
     *
     * \param cluster_id Cluster identifier.
     *
     * \retval ::FWK_E_ACCESS Wrong id.
     * \retval ::FWK_SUCCESS If the call is successful.
     * \return One of the standard framework error codes.
     */
    int (*update)(fwk_id_t cluster_id);
};

struct mod_perf_controller_cluster_config {
    /*! Module or element identifier of the driver */
    fwk_id_t driver_id;

    /*! API identifier of the driver */
    fwk_id_t driver_api_id;
};

/*!
 * \brief Performance controller API IDs
 */
enum mod_perf_controller_api_idx {
    /*! Index for the core performance adjustments API */
    MOD_PERF_CONTROLLER_CLUSTER_PERF_API = 0U,

    /*! Index for the cluster power adjustments API */
    MOD_PERF_CONTROLLER_CORE_POWER_API,

    /*! Index for the controller update API*/
    MOD_PERF_CONTROLLER_UPDATE_API,

    /*! Number of APIs */
    MOD_PERF_CONTROLLER_API_COUNT
};

/*!
 * \}
 */

#endif /* MOD_PERF_CONTROLLER_H_ */
