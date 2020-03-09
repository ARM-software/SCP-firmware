/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_N1SDP_PCIE_H
#define MOD_N1SDP_PCIE_H

#include <fwk_id.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup GroupModuleN1SDPPcie N1SDP PCIe Driver
 *
 * \brief Driver support for N1SDP PCIe Root Complex & End Point devices.
 *
 * \details This module provides driver support for enabling and configuring
 * the PCIe peripheral either in root complex mode or in end point mode.
 *
 * \{
 */

/*!
 * \brief N1SDP PCIe instance configuration
 */
struct n1sdp_pcie_dev_config {
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
     * Base address of the PCIe AXI slave memory region (within 32-bit address
     * space). This region holds the ECAM space, MMIO32 & IO space.
     */
    uint32_t axi_slave_base32;

    /*!
     * Base address of the PCIe AXI slave memory region (in 64-bit address
     * space). This region holds the MMIO64 space.
     */
    uint64_t axi_slave_base64;

    /*! Identifier to indicate if the PCIe controller is CCIX capable */
    bool ccix_capable;
};

/*!
 * \brief Module API indices
 */
enum n1sdp_pcie_api_idx {
    /*! Index of the PCIe initialization API */
    N1SDP_PCIE_API_IDX_PCIE_INIT,

    /*! Index of the CCIX config API */
    N1SDP_PCIE_API_IDX_CCIX_CONFIG,

    /*! Number of APIs */
    N1SDP_PCIE_API_COUNT
};

/*!
 * \brief N1SDP PCIe initialization api
 */
struct n1sdp_pcie_init_api {
   /*!
    * \brief API to power ON the PCIe controller
    *
    * \param id Identifier of the PCIe instance
    *
    * \retval FWK_SUCCESS The operation succeeded.
    * \return One of the standard error codes.
    */
   int (*power_on)(fwk_id_t id);

   /*!
    * \brief API to initialize the PHY layer
    *
    * \param id Identifier of the PCIe instance
    *
    * \retval FWK_SUCCESS The operation succeeded.
    * \return One of the standard error codes.
    */
   int (*phy_init)(fwk_id_t id);

   /*!
    * \brief API to initialize the PCIe controller
    *
    * \param id Identifier of the PCIe instance
    * \param ep_mode Identifier to configure the controller
    * in root port or endpoint mode
    *
    * \retval FWK_SUCCESS The operation succeeded.
    * \return One of the standard error codes.
    */
   int (*controller_init)(fwk_id_t id, bool ep_mode);

   /*!
    * \brief API to perform the link training process
    *
    * \param id Identifier of the PCIe instance
    *
    * \retval FWK_SUCCESS The operation succeeded.
    * \return One of the standard error codes.
    */
   int (*link_training)(fwk_id_t id, bool ep_mode);

   /*!
    * \brief API to setup the root complex
    *
    * \param id Identifier of the PCIe instance
    *
    * \retval FWK_SUCCESS The operation succeeded.
    * \return One of the standard error codes.
    */
   int (*rc_setup)(fwk_id_t id);

   /*!
    * \brief API to enable Virtual Channel 1 and map to
    * specified Traffic class. This API is used in multichip mode.
    *
    * \param id Identifier of the PCIe instance
    * \param vc1_tc Traffic class to be mapped to VC1
    *
    * \retval FWK_SUCCESS The operation succeeded.
    * \return One of the standard error codes.
    */
   int (*vc1_setup)(fwk_id_t id, uint8_t vc1_tc);

};

/*!
 * \brief N1SDP PCIe ccix configuration api
 */
struct n1sdp_pcie_ccix_config_api {
   /*!
    * \brief Enable the optimized tlp (Transaction Layer Packet)
    * for the ccix root complex
    *
    * \param enable Enable optimized tlp (true) or disable it (false)
    * and thus enable pcie compatible header
    *
    * \retval FWK_SUCCESS The operation succeeded.
    * \return One of the standard error codes.
    */
   int (*enable_opt_tlp)(bool enable);

};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_N1SDP_PCIE_H */
