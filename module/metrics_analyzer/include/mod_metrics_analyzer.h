/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Metrics Analyzer
 */

#ifndef MOD_METRICS_ANALYZER_H
#define MOD_METRICS_ANALYZER_H

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupMetricsAnalyzer System Metrics Analyzer
 * \{
 */

/*!
 * \brief API indices
 */
enum mod_metrics_analyzer_api_idx {
    /*! Metrics_Analyzer API count */
    MOD_METRICS_ANALYZER_API_IDX_COUNT,
};

/*!
 * \brief Metrics analyzer interactor, which can be a provider or a consumer.
 *        The interactor is defined by the API ID and the corresponding domain
 *        ID from the interactor perspective.
 */
struct mod_metrics_analyzer_interactor {
    /*!
     * ID of the API which information is transfered through.
     */
    const fwk_id_t api_id;
    /*!
     * The ID to be used when calling the API with.
       The domain under interaction.
     */
    const fwk_id_t domain_id;
};

/*!
 * \brief Metrics Analyzer module domain configuration data.
 */
struct mod_metrics_analyzer_domain_config {
    /*!
     * List of the power limit providers for the metrics analyzer domain.
     */
    const struct mod_metrics_analyzer_interactor *limit_providers;
    /*!
     * The power limit consumer for the metrics analyzer domain.
     */
    const struct mod_metrics_analyzer_interactor limit_consumer;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_METRICS_ANALYZER_H */
