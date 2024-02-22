/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef NOC_S3_DISCOVERY_H
#define NOC_S3_DISCOVERY_H

#include <noc_s3.h>
#include <noc_s3_reg.h>

#include <mod_noc_s3.h>

#include <stdint.h>

/**
 * \brief Parse the component header to find the target subfeature.
 *
 * \details Component header contains a list of subfeatures that it supports.
 *          This API parses the list and then records the offset of the
 *          subfeature.
 *
 * \param[in]  component_hdr   Component header address.
 * \param[in]  subfeature_type Type of the subfeature.
 * \param[out] ret_off_addr    Offset of the subfeature.
 *
 * \return FWK_E_PARAM   If the component header is not of type component.
 * \return FWK_E_SUPPORT If the type of subfeature is not supported.
 * \return FWK_SUCCESS   If the subfeature offset is found.
 */
int noc_s3_get_subfeature_offset(
    struct noc_s3_component_cfg_hdr *component_hdr,
    enum noc_s3_subfeature_type subfeature_type,
    uint32_t *ret_off_addr);

/**
 * \brief Performs a discovery flow on the given NoC S3 instance.
 *
 * \details In NoC S3, "Discovery" is the process of finding the offset of all
 *          the nodes present on the discovery tree. This API performs two stage
 *          walk of the tree. In the first stage, a depth first search is
 *          performed on the tree to find the number of nodes for each node
 *          type. In the second stage, another depth first search is performed
 *          to record the node offset in the table.
 *
 * \param[in, out] dev NoC S3 Device handler struct \ref noc_s3.h
 *
 * \return FWK_SUCCESS If the discovery is successful.
 * \return FWK_E_PARAM If any of the input parameter is invalid.
 */
int noc_s3_discovery(struct mod_noc_s3_dev *dev);

#endif /* NOC_S3_DISCOVERY_H */
