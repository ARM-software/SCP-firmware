/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Performance, Monitor, and Instrumentation HAL Interface.
 */

#ifndef MOD_PMI_H
#define MOD_PMI_H

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupPmi Performance and Instrumentation
 *      module
 * \{
 */

/*!
 * \defgroup GroupPmiTypes Types
 * \{
 */

/*!
 * \brief PMI configuration data.
 */
struct mod_pmi_driver_config {
    /*! Identifier of the PMI driver */
    fwk_id_t driver_id;

    /*! Identifier of the PMI driver api*/
    fwk_id_t driver_api_id;
};

/*!
 * \}
 */

/*!
 * \defgroup GroupPmiApis APIs
 * \{
 */

/*!
 * \brief PMI API.
 */
struct mod_pmi_hal_api {
    /*!
     * \brief Start cycle counting
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*start_cycle_count)(void);

    /*!
     * \brief Stop cycle counting
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*stop_cycle_count)(void);

    /*!
     * \brief Get the current cycle count.
     *
     * \param[out] cycle_count  Value of the current cycle count
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */

    int (*get_cycle_count)(uint64_t *cycle_count);

    /*!
     * \brief Set the cycle count to cycle_count if supported
     *
     * \param[in] cycle_count Reset/Set value of the cycle count.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*set_cycle_count)(uint64_t cycle_count);

    /*!
     * \brief Returns cycle count diff for 'end' and 'start'
     *
     * \details Since cycle count register can wrap around, it is important
     *          to take care of this while calculating the difference between
     *          'end' and 'start'. This call assumes 'end' wrapped around but
     *          still less than the start value.
     *
     * \param[in] start start value of the cycle count
     *
     * \param[in] end end value of the cycle count
     *
     * \retval Difference between cycle count 'end' - 'start'
     */
    uint64_t (*cycle_count_diff)(uint64_t start, uint64_t end);

    /*!
     * \brief Get current timestamp
     *
     * \retval Value of the current time
     */
    uint64_t (*get_current_time)(void);
};

/*!
 * \brief PMI Driver API.
 */
struct mod_pmi_driver_api {
    /*!
     * \brief Start cycle counting
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*start_cycle_count)(void);

    /*!
     * \brief Stop cycle counting
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval One of the standard framework error codes.
     */
    int (*stop_cycle_count)(void);

    /*!
     * \brief Get the current cycle count.
     *
     * \param[out] cycle_count  Value of the current cycle count
     * \return One of the standard framework error codes.
     */
    int (*get_cycle_count)(uint64_t *cycle_count);

    /*!
     * \brief Set the cycle count to cycle_count if supported
     *
     * \param[in] cycle_count Reset/Set value of the cycle count.
     * \return One of the standard framework error codes.
     */
    int (*set_cycle_count)(uint64_t cycle_count);

    /*!
     * \brief Returns cycle count diff for 'end' and 'start'
     *
     * \details Since cycle count register can wrap around, it is important
     *          to take care of this while calculating the difference between
     *          'end' and 'start'. This call assumes 'end' wrapped around but
     *          still less than the start value.
     *
     * \param[in] start start value of the cycle count
     *
     * \param[in] end end value of the cycle count
     */
    uint64_t (*cycle_count_diff)(uint64_t start, uint64_t end);

    /*!
     * \brief Get current timestamp
     *
     * \retval Value of the current time
     */
    uint64_t (*get_current_time)(void);
};

/*!
 * \}
 */

/*!
 * \defgroup GroupPmiIds Identifiers
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_pmi_api_idx {
    MOD_PMI_API_IDX_HAL, /*! API index for mod_pmi_api_id_pmi() */
    MOD_PMI_API_IDX_COUNT /*! Number of defined APIs */
};

/*! Module API identifier */
static const fwk_id_t mod_pmi_api_id_hal =
    FWK_ID_API_INIT(FWK_MODULE_IDX_PMI, MOD_PMI_API_IDX_HAL);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_PMI_H */
