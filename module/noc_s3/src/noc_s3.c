/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <noc_s3.h>
#include <noc_s3_discovery.h>
#include <noc_s3_reg.h>

#include <mod_noc_s3.h>

#include <fwk_log.h>
#include <fwk_status.h>

#include <inttypes.h>

/* Check if the node_type is of node_type domain. */
bool noc_s3_is_node_type_domain(enum mod_noc_s3_node_type node_type)
{
    bool is_domain;

    switch (node_type) {
    case MOD_NOC_S3_NODE_TYPE_GLOBAL_CFGNI:
    case MOD_NOC_S3_NODE_TYPE_VD:
    case MOD_NOC_S3_NODE_TYPE_PD:
    case MOD_NOC_S3_NODE_TYPE_CD:
        is_domain = true;
        break;
    default:
        is_domain = false;
        break;
    };

    return is_domain;
}

/* Check if the node_type is of node_type component. */
bool noc_s3_is_node_type_component(enum mod_noc_s3_node_type node_type)
{
    bool is_component;

    switch (node_type) {
    case MOD_NOC_S3_NODE_TYPE_ASNI:
    case MOD_NOC_S3_NODE_TYPE_AMNI:
    case MOD_NOC_S3_NODE_TYPE_PMU:
    case MOD_NOC_S3_NODE_TYPE_HSNI:
    case MOD_NOC_S3_NODE_TYPE_HMNI:
    case MOD_NOC_S3_NODE_TYPE_PMNI:
        is_component = true;
        break;
    default:
        is_component = false;
        break;
    };

    return is_component;
}
