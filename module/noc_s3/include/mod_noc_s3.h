/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef MOD_NOC_S3_H
#define MOD_NOC_S3_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupNoCS3 Network on Chip S3
 * \{
 */

/*!
 * \brief NoC S3 node type enumerations.
 */
enum mod_noc_s3_node_type {
    /* Domains */
    /*! NoC S3 node type Global CFGNI Domain. */
    MOD_NOC_S3_NODE_TYPE_GLOBAL_CFGNI,
    /*! NoC S3 node type Voltage Domain. */
    MOD_NOC_S3_NODE_TYPE_VD,
    /*! NoC S3 node type Power Domain. */
    MOD_NOC_S3_NODE_TYPE_PD,
    /*! NoC S3 node type Clock Domain. */
    MOD_NOC_S3_NODE_TYPE_CD,
    /* Components */
    /*! NoC S3 node type ASNI. */
    MOD_NOC_S3_NODE_TYPE_ASNI,
    /*! NoC S3 node type AMNI. */
    MOD_NOC_S3_NODE_TYPE_AMNI,
    /*! NoC S3 node type PMU. */
    MOD_NOC_S3_NODE_TYPE_PMU,
    /*! NoC S3 node type HSNI. */
    MOD_NOC_S3_NODE_TYPE_HSNI,
    /*! NoC S3 node type HMNI. */
    MOD_NOC_S3_NODE_TYPE_HMNI,
    /*! NoC S3 node type PMNI. */
    MOD_NOC_S3_NODE_TYPE_PMNI,
    /*! Maximum number of nodes. */
    MOD_NOC_S3_NODE_TYPE_COUNT
};

/*!
 * \brief NoC S3 configuration node granularity enumeration
 */
enum noc_s3_granularity {
    /*! NoC S3 Node granularity 4K. */
    NOC_S3_4KB_CONFIG_NODES,
    /*! NoC S3 Node granularity 64K. */
    NOC_S3_64KB_CONFIG_NODES,
};

/*!
 * \brief NoC S3 discovery data.
 */
struct noc_s3_discovery_data {
    /* Offset table. */
    uintptr_t *table[MOD_NOC_S3_NODE_TYPE_COUNT];
    /* Size of each row, for each node type in the offset table. */
    uint8_t max_node_size[MOD_NOC_S3_NODE_TYPE_COUNT];
};

/*!
 * \brief NoC S3 device structure
 */
struct mod_noc_s3_dev {
    /*! NoC S3 periphbase address, same as CFGNI0 address. */
    uintptr_t periphbase;
    /*!
     * The memory-mapped registers of NoC S3 are organized in a series of 4KB or
     * 64KB regions. Specify whether it has 4KB or 64KB config nodes.
     */
    enum noc_s3_granularity node_granularity;
    /*!
     * Points to the table generated during discovery.
     */
    struct noc_s3_discovery_data discovery_data;
    /*!
     * Flag to indicate that discovery is performed and the table is
     * initialized.
     */
    bool discovery_completed;
};

/*!
 * \brief Platform notification source and notification id.
 *
 * \details If the module is dependant on notification from other modules, then
 *          the module will subscribe to its notification and start only after
 *          receiving it.
 */
struct mod_noc_s3_platform_notification {
    /*! Identifier of the notification id */
    const fwk_id_t notification_id;
    /*! Identifier of the module sending the notification */
    const fwk_id_t source_id;
};

/*!
 * \brief Info to configure ports in the NoC S3 block.
 */
struct mod_noc_s3_element_config {
    /*! NoC S3 periphbase address, same as CFGNI0 address. */
    uintptr_t periphbase;
    /*! Platform notification source and notification id */
    struct mod_noc_s3_platform_notification plat_notification;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_NOC_S3_H */
