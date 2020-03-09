/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Management Protocol Support.
 */

#ifndef MOD_SCMI_MANAGEMENT_H
#define MOD_SCMI_MANAGEMENT_H

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup GroupN1SDPSCMI_MANAGEMENT SCMI Management Protocol
 * @{
 */

/*!
 * \brief Macro defining the SCMI Management Protocol's ID.
 */
#define SCMI_PROTOCOL_ID_MANAGEMENT      UINT32_C(0x89)

/*!
 * \brief Macro defining the SCMI Management Protocol's version.
 */
#define SCMI_PROTOCOL_VERSION_MANAGEMENT UINT32_C(0x10000)

/*!
 * \brief Identifiers of the SCMI Management Protocol commands.
 */
enum scmi_management_command_id {
    /*! Command ID for getting clock status */
    SCMI_MANAGEMENT_CLOCK_STATUS_GET = 0x3,
    /*! Commmand ID for chip ID information */
    SCMI_MANAGEMENT_CHIPID_INFO_GET  = 0x4,
};

/*!
 * \brief Protocol handler for getting clock status from SCP.
 */
struct scmi_management_clock_status_get_p2a {
    /*! SCMI status */
    int32_t status;
    /*! Clock status */
    uint32_t clock_status;
};

/*!
 * \brief Protocol handler for chip ID information.
 */
struct scmi_management_chipid_info_get_p2a {
    /*! SCMI status */
    int32_t status;
    /*! Multi chip mode identifier */
    uint32_t multi_chip_mode;
    /*! Chip ID of the chip if multi_chip_mode is set */
    uint32_t chipid;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SCMI_MANAGEMENT_H */
