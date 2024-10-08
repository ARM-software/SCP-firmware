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
/*
 *                  +---------------------------------------+
 *                  |                NoC S3                 |
 *                  |                                       |
 *                  |                                       |
 *                  |                                       |
 *                  |   Completer Node      Requester Node  |
 *                  |   for Requester       For completer   |
 * +------------+   |   +----------+        +-----------+   |    +-----------+
 * |            |   |   |          |        |           |   |    |           |
 * |  Requester +-->+-->|xSNI Node +-- -- --+ xMNI Node +-->+--->| Completer |
 * |            |   |   |          |        |           |   |    |           |
 * +------------+   |   | PSAM/APU |        | APU       |   |    +-----------+
 *                  |   +----------+        +-----------+   |
 *                  |                                       |
 *                  |                                       |
 *                  |                                       |
 *                  |                                       |
 *                  |                                       |
 *                  +---------------------------------------+
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
    /*! Identifier of the notification id. */
    const fwk_id_t notification_id;
    /*! Identifier of the module sending the notification. */
    const fwk_id_t source_id;
};

/*!
 * \brief Data to configure carveout in PSAM of an xSNI port.
 */
struct mod_noc_s3_psam_region {
    /*! Base address of the carveout. */
    uint64_t base_address;
    /*! Size of the carveout. */
    uint64_t size;
    /*! Target xMNI node ID. */
    uint32_t target_id;
};

/*!
 * \brief Component configuration info data.
 */
struct mod_noc_s3_comp_config {
    /*! Type of the component. */
    enum mod_noc_s3_node_type type;
    /*! ID of the component port. */
    uint32_t id;
    /*! Info of carveouts to be mapped in PSAM. */
    struct mod_noc_s3_psam_region *psam_regions;
    /*! Number of carveouts for PSAM. */
    uint32_t psam_region_count;
};

/*!
 * \brief Info to configure ports in the NoC S3 block.
 */
struct mod_noc_s3_element_config {
    /*! NoC S3 periphbase address, same as CFGNI0 address. */
    uintptr_t periphbase;
    /*! List of component nodes to be configured. */
    struct mod_noc_s3_comp_config *component_config;
    /*! Number of component nodes. */
    uint32_t component_count;
    /*! Platform notification source and notification id. */
    struct mod_noc_s3_platform_notification plat_notification;
};

/*!
 * \brief NoC S3 module API indices.
 */
enum mod_noc_s3_api_idx {
    /*! Interface to configure carveouts in a PSAM. */
    MOD_NOC_S3_API_SETUP_PSAM,
    /*! Total API count. */
    MOD_NOC_S3_API_COUNT
};

/*!
 * \brief Module interface to manage mappings.
 */
struct mod_noc_s3_memmap_api {
    /*!
     * \brief Program a region in the xSNI node of the target NoC S3. This API
     *        maps a region and returns the index where the region is mapped.
     *
     * \param[in]  dev              Device handler containing base address of
     *                              the registers to configure NoC S3.
     * \param[in]  component_config xSNI configuration information for
     *                              configuring a region into the target xSNI
     *                              node's PSAM.
     * \param[out] region_idx[out]  Index of the mapped region.
     *
     * \return FWK_E_DATA if mapping region is invalid.
     * \return FWK_E_PARAM if an invalid parameter was encountered.
     * \return FWK_E_SUCESS if regions are mapped succesfully.
     */
    int (*map_region_in_psam)(
        struct mod_noc_s3_dev *dev,
        struct mod_noc_s3_comp_config *component_config,
        uint8_t *region_idx);

    /*!
     * \brief Remove the mapping from the region in PSAM.
     *
     * \param[in]  dev              Device handler containing base address of
     *                              the registers to configure NoC S3.
     * \param[in]  component_config xSNI configuration information for
     *                              configuring a region into the target xSNI
     *                              node's PSAM.
     * \param[out] region_idx       Index of the mapped region.
     *
     * \return FWK_E_DATA if mapping region is invalid.
     * \return FWK_E_PARAM if an invalid parameter was encountered.
     * \return FWK_E_SUCESS if regions are unmapped succesfully.
     */
    int (*unmap_region_in_psam)(
        struct mod_noc_s3_dev *dev,
        struct mod_noc_s3_comp_config *component_config,
        uint8_t region_idx);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_NOC_S3_H */
