/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MORELLO_PCIE_H
#define MOD_MORELLO_PCIE_H

#include <fwk_id.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupMorelloModule Morello Product Modules
 * @{
 */

/*!
 * \defgroup GroupModuleMorelloPcie Morello PCIe Driver
 *
 * \brief Driver support for Morello PCIe Root Complex & End Point devices.
 *
 * \details This module provides driver support for enabling and configuring
 * the PCIe peripheral either in root complex mode or in end point mode.
 *
 * \{
 */

/*!
 * \brief PCIe AXI outbound region types
 */
enum ob_region_type {
    PCIE_AXI_OB_REGION_TYPE_MMIO = 0x2,
    PCIE_AXI_OB_REGION_TYPE_IO = 0x6,
    PCIE_AXI_OB_REGION_TYPE_ECAM = 0xA,
    PCIE_AXI_OB_REGION_TYPE_MSG = 0xC,
    PCIE_AXI_OB_REGION_TYPE_VDM = 0xD,
};

/*!
 * \brief PCIe AXI outbound region map descriptor
 */
struct morello_pcie_axi_ob_region_map {
    /*! Base address */
    uint64_t base;

    /*! Region size in bytes */
    uint64_t size;

    /*! Region type */
    enum ob_region_type type;
};

/*!
 * \brief Morello PCIe instance configuration
 */
struct morello_pcie_dev_config {
    /*!
     * Base address of the PCIe Controller. This includes the PHY configuration
     * and PCIe IP level configuration registers.
     */
    uintptr_t ctrl_base;

    /*!
     * Base address of the PCIe functional configuration registers. This
     * region includes registers for configuring the IP in both RC and
     * EP modes.
     */
    uintptr_t global_config_base;

    /*! Base address of the PCIe message registers. */
    uintptr_t msg_base;

    /*!
     * Base address of the PCIe AXI subordinate memory region (within 32-bit
     * address space). This region holds the ECAM space, MMIO32 & IO space.
     */
    uint32_t axi_subordinate_base32;

    /*!
     * Base address of the PCIe AXI subordinate memory region (in 64-bit address
     * space). This region holds the MMIO64 space.
     */
    uint64_t axi_subordinate_base64;

    /*! Identifier to indicate if the PCIe controller is CCIX capable */
    bool ccix_capable;

    /*! Table of AXI outbound region entries */
    struct morello_pcie_axi_ob_region_map *axi_ob_table;

    /*! Number of entries in the \ref axi_ob_table */
    unsigned int axi_ob_count;

    /*! Primary bus number for the RP */
    uint8_t pri_bus_num;
};

/*!
 * \brief Module API indices
 */
enum morello_pcie_api_idx {
    /*! Index of the PCIe initialization API */
    MORELLO_PCIE_API_IDX_PCIE_INIT,

    /*! Index of the CCIX config API */
    MORELLO_PCIE_API_IDX_CCIX_CONFIG,

    /*! Number of APIs */
    MORELLO_PCIE_API_COUNT
};

/*!
 * \brief Morello PCIe initialization api
 */
struct morello_pcie_init_api {
    /*!
     * \brief API to power ON the PCIe controller
     *
     * \param id Identifier of the PCIe instance
     *
     * \retval ::FWK_SUCCESS The operation succeeded
     * \retval ::FWK_E_PARAM Element data is NULL
     * \retval ::FWK_E_TIMEOUT PCIe power on timed out
     * \return One of the standard error codes for implementation-defined
     *      errors
     */
    int (*power_on)(fwk_id_t id);

    /*!
     * \brief API to initialize the PHY layer
     *
     * \param id Identifier of the PCIe instance
     *
     * \retval ::FWK_SUCCESS The operation succeeded
     * \retval ::FWK_E_PARAM Element data is NULL
     * \retval ::FWK_E_TIMEOUT PCIe initialisation timed out
     * \return One of the standard error codes for implementation-defined
     *      errors
     */
    int (*phy_init)(fwk_id_t id);

    /*!
     * \brief API to initialize the PCIe controller
     *
     * \param id Identifier of the PCIe instance
     * \param ep_mode Identifier to configure the controller
     * in root port or endpoint mode
     *
     * \retval ::FWK_SUCCESS The operation succeeded
     * \retval ::FWK_E_PARAM Element data is NULL
     * \retval ::FWK_E_TIMEOUT PCIe Controller initialisation timed out
     * \return One of the standard error codes for implementation-defined
     *      errors
     */
    int (*controller_init)(fwk_id_t id, bool ep_mode);

    /*!
     * \brief API to perform the link training process
     *
     * \param id Identifier of the PCIe instance
     *
     * \retval ::FWK_SUCCESS The operation succeeded
     * \retval ::FWK_E_PARAM Element data is NULL
     * \retval ::FWK_E_TIMEOUT PCIe Link training timed out
     * \return One of the standard error codes for implementation-defined
     *      errors
     */
    int (*link_training)(fwk_id_t id, bool ep_mode);

    /*!
     * \brief API to setup the root complex
     *
     * \param id Identifier of the PCIe instance
     *
     * \retval ::FWK_SUCCESS The operation succeeded
     * \retval ::FWK_E_PARAM Element data is NULL
     * \retval ::FWK_E_DATA AXI region base not aligned with region size
     * \retval ::FWK_E_RANGE Invalid AXI region
     * \return One of the standard error codes for implementation-defined
     *      errors
     */
    int (*rc_setup)(fwk_id_t id);

    /*!
     * \brief API to enable Virtual Channel 1 and map to
     * specified Traffic class. This API is used in multichip mode.
     *
     * \param id Identifier of the PCIe instance
     * \param vc1_tc Traffic class to be mapped to VC1
     *
     * \retval ::FWK_SUCCESS The operation succeeded
     * \retval ::FWK_E_PARAM Element data is NULL or PCIe VC setup failed
     * \return One of the standard error codes for implementation-defined
     *      errors
     */
    int (*vc1_setup)(fwk_id_t id, uint8_t vc1_tc);
};

/*!
 * \brief Morello PCIe ccix configuration api
 */
struct morello_pcie_ccix_config_api {
    /*!
     * \brief Enable the optimized tlp (Transaction Layer Packet)
     * for the ccix root complex
     *
     * \param enable Enable optimized tlp (true) or disable it (false)
     * and thus enable pcie compatible header
     *
     * \retval ::FWK_SUCCESS The operation succeeded
     * \retval ::FWK_E_DATA PCIe/CCIX config data is NULL
     * \return One of the standard error codes for implementation-defined
     *      errors
     */
    int (*enable_opt_tlp)(bool enable);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_MORELLO_PCIE_H */
