/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) support.
 */

#ifndef INTERNAL_SCMI_VENDOR_EXT_H
#define INTERNAL_SCMI_VENDOR_EXT_H

#include <stdint.h>

/*!
 * \addtogroup GroupSYNQUACERModule SYNQUACER Product Modules
 * @{
 */

/*!
 * \defgroup GroupSCMI_VendorEXT SCMI vendor extension protocol
 *
 * \brief SCMI vendor extension protocol.
 *
 * \details This module implements a SCMI vendor extension protocol.
 *
 * @{
 */

/*! SCMI protocol id for vendor extension message. */
#define SCMI_PROTOCOL_ID_VENDOR_EXT UINT32_C(0x80)
/*! SCMI version. */
#define SCMI_PROTOCOL_VERSION_VENDOR_EXT UINT32_C(0x10000)

/*!
 * \brief Identifiers of the SCMI Vendor extension Protocol commands.
 */
enum scmi_vendor_ext_command_id {
    /*! Retrieve DRAM mapping information */
    SCMI_VENDOR_EXT_MEMORY_INFO_GET = 0x003,
};

/*!
 * \brief PROTOCOL_ATTRIBUTES
 */
struct __attribute((packed)) scmi_vendor_ext_protocol_attributes_p2a {
    /*! SCMI status. */
    int32_t status;
    /*! attributes. */
    uint32_t attributes;
};

/*
 * Memory Info get structure
 */

/*! array size of memory information array. */
#define SCMI_MEMORY_INFO_ARRAY_NUM_MAX (3)

/*!
 * \brief detailed memory mapping information.
 */
struct memory_info_array {
    /*! dram region start address of lower 32-bits. */
    uint32_t start_offset_low;
    /*! dram region start address of upper 32-bits. */
    uint32_t start_offset_high;
    /*! dram size of lower 32-bits. */
    uint32_t size_low;
    /*! dram size of upper 32-bits. */
    uint32_t size_high;
};

/*!
 * \brief memory mapping information structure.
 */
struct __attribute((packed)) synquacer_memory_info {
    /*! number of memory regions. */
    uint32_t array_num;
    /*! reserved. */
    uint32_t reserved;
    /*! array of memory regions. */
    struct memory_info_array memory_info_array[SCMI_MEMORY_INFO_ARRAY_NUM_MAX];
};

/*!
 * \brief memory mapping information response.
 */
struct scmi_vendor_ext_memory_info_get_resp {
    /*! SCMI status. */
    int status;
    /*! reserved. */
    int reserved;
    /*! pointer to the momory information array. */
    struct synquacer_memory_info meminfo;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* INTERNAL_SCMI_VENDOR_EXT_H */
