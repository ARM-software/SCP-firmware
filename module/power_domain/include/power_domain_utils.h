/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power domain utilities.
 */

#ifndef POWER_DOMAIN_UTILS_H
#define POWER_DOMAIN_UTILS_H

/*!
 * \addtogroup GroupPowerDomain
 * \{
 */

#include <fwk_element.h>
#include <fwk_id.h>

/*!
 * \brief Create power domain element table.
 *
 * \details This function will create power domain element table for an SoC
 *      that contains the same number of cores for each cluster. Also, all cores
 *      supports the same state table and all clusters support the same state
 *      table. Devices power domain configuration other than cores and clusters
 *      as well as systop should be provided in the static table.
 *
 * \param core_count Number of cores in the system.
 * \param cluster_count Number of clusters in the system.
 * \param driver_idx The index of the driver which controls the cores domain.
 * \param api_idx The driver api index.
 * \param core_state_table The cores state mask table.
 * \param core_state_table_size The cores state table size.
 * \param cluster_state_table The cluster state mask table.
 * \param cluster_state_table_size The cluster state table size.
 * \param static_table Power domain static elements for devices and systop.
 * \param static_table_size Power domain static table size.
 *
 * \retval A pointer to the newly created power domain elements table.
 * \retval NULL if the table creation failed.
 */
const struct fwk_element *create_power_domain_element_table(
    unsigned int core_count,
    unsigned int cluster_count,
    unsigned int driver_idx,
    unsigned int api_idx,
    const uint32_t *core_state_table,
    size_t core_state_table_size,
    const uint32_t *cluster_state_table,
    size_t cluster_state_table_size,
    struct fwk_element *static_table,
    size_t static_table_size);

/*!
 * \}
 */

#endif /* POWER_DOMAIN_UTILS_H */
