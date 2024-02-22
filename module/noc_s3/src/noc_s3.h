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

#endif /* NOC_S3_H */
