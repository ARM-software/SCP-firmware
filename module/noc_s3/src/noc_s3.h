/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef NOC_S3_H
#define NOC_S3_H

#include <mod_noc_s3.h>

#include <stdbool.h>
#include <stdint.h>

#define MOD_NAME "[NOC_S3] "

/*!
 * \brief NoC S3 component subfeature type enumerations.
 */
enum noc_s3_subfeature_type {
    /*! NoC S3 subfeature Access Protection Unit. */
    NOC_S3_NODE_TYPE_APU,
    /*! NoC S3 subfeature Programable System Address Map. */
    NOC_S3_NODE_TYPE_PSAM,
    /*! NoC S3 subfeature Functional Crossbar Unit. */
    NOC_S3_NODE_TYPE_FCU,
    /*! NoC S3 subfeature Interconnect Device Management. */
    NOC_S3_NODE_TYPE_IDM,
};

/*!
 * \brief Check if the node type is of type domain.
 *
 * \param[in] node_type Node type.
 *
 * \return True if node is a domain.
 * \return False if node is not a domain.
 */
bool noc_s3_is_node_type_domain(enum mod_noc_s3_node_type node_type);

/*!
 * \brief Check if the node type is of type component.
 *
 * \param[in] node_type Node type.
 *
 * \return True if node is a component.
 * \return False if node is not a component.
 */
bool noc_s3_is_node_type_component(enum mod_noc_s3_node_type node_type);

/*!
 * \brief Check if the type is of type subfeature.
 *
 * \param[in] type Subfeature type.
 *
 * \return True if node is a subfeature.
 * \return False if node is not a subfeature.
 */
bool noc_s3_is_type_subfeature(enum noc_s3_subfeature_type type);

/*!
 * \brief Check if the incoming address range is overlapping with the mapped
 *        address.
 *
 * \details Looking at the case below, if there is an overlap, then incoming
 *          address base will less than mapped end and mapped base will be less
 *          than incoming end.
 *
 *                   start|------Region A------|end
 *               start|----------Region B-----------|end
 *
 * \param start_a Region A start address.
 * \param end_a  Region A end address.
 * \param start_b Region B start address.
 * \param end_b  Region B end address.
 *
 * \return true Overlapping regions.
 * \return false Non overlapping region.
 */
bool noc_s3_check_overlap(
    uint64_t start_a,
    uint64_t end_a,
    uint64_t start_b,
    uint64_t end_b);

/*!
 * \brief Calculate the peripheral base address of the subfeature.
 *
 * \details This API validates the discovery table and the parameters and then
 *          calculate the base address of the target subfeature. The component
 *          offset is fetched from the discovery table and the subfeature offset
 *          is then taken from the component header.
 *
 * \param dev NoC S3 Device handler.
 * \param node_type Target node type.
 * \param subfeature_type Subfeature type of the component.
 * \param node_id Node id of the component.
 * \param subfeature_address computed subfeature address.
 *
 * \return FWK_E_PARAM If input arguments are invalid.
 * \return FWK_SUCCESS If the subfeature address calculated successfully.
 */
int noc_s3_get_subfeature_address(
    struct mod_noc_s3_dev *dev,
    enum mod_noc_s3_node_type node_type,
    enum noc_s3_subfeature_type subfeature_type,
    uint32_t node_id,
    uintptr_t *subfeature_address);

#endif /* NOC_S3_H */
