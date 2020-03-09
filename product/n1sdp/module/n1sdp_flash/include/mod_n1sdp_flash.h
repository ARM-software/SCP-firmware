/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Header file for N1SDP flash driver module used to parse the firmware
 *     package residing in on-board QSPI flash memory.
 */

#ifndef MOD_N1SDP_FLASH_H
#define MOD_N1SDP_FLASH_H

#include <fwk_id.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup GroupModuleN1SDPFlash N1SDP Flash
 *
 * \brief Provides an API to access N1SDP's QSPI flash memory.
 *
 * \details This module provides a table of descriptors that each describe a
 * block of data in the QSPI flash memory.
 *
 * \{
 */

/*!
 * \brief CRC-16-CCITT reversed polynomial used for CRC calculation.
 */
#define CRC16_POLYNOMIAL UINT16_C(0x8408)

/*!
 * \brief Type of flash entry.
 */
enum mod_n1sdp_flash_data_type {
    /*! SCP bypass ROM */
    MOD_N1SDP_FLASH_DATA_TYPE_ROM_BYPASS = 1,

    /*! N1SDP Firmware Image Package (NFIP) */
    MOD_N1SDP_FLASH_DATA_TYPE_NFIP,

    /*! NVM configuration space */
    MOD_N1SDP_FLASH_DATA_TYPE_NVM_CFG,

    /*! NVM log space */
    MOD_N1SDP_FLASH_DATA_TYPE_NVM_LOG,

    /*! Number of flash entry types */
    MOD_N1SDP_FLASH_DATA_TYPE_COUNT,
};

/*!
 * \brief Type of N1SDP FIP entry.
 */
enum mod_n1sdp_fip_data_type {
    /*! SCP BL2 Binary */
    MOD_N1SDP_FIP_TYPE_SCP_BL2,

    /*! MCP BL2 Binary */
    MOD_N1SDP_FIP_TYPE_MCP_BL2,

    /*! EL3 Runtime - BL31 Binary */
    MOD_N1SDP_FIP_TYPE_TF_BL31,

    /*! Secure Payload BL32 Binary */
    MOD_N1SDP_FIP_TYPE_TF_BL32,

    /*! Non-secure BL33 (UEFI) Binary */
    MOD_N1SDP_FIP_TYPE_NS_BL33,

    /*! Number of N1SDP FIP entry types */
    MOD_N1SDP_FIP_TYPE_COUNT,
};

/*!
 * \brief Attribute flags used for flash & NFIP entries.
 */
enum mod_n1sdp_flash_data_flag {
    MOD_N1SDP_FLASH_DATA_FLAG_ACTIVE         = (1 << 0),
    MOD_N1SDP_FLASH_DATA_FLAG_ENABLED        = (1 << 1),
    MOD_N1SDP_FLASH_DATA_FLAG_CRC16_ENABLED  = (1 << 2),
    MOD_N1SDP_FLASH_DATA_FLAG_SECURE         = (1 << 3),
};

/*!
 * \brief Descriptor for a flash table entry.
 */
struct mod_n1sdp_flash_descriptor {
    /*! The type of the flash entry */
    enum mod_n1sdp_flash_data_type type;

    /*!
     * The absolute address of the start of the data described by this
     * entry.
     */
    uintptr_t address;

    /*! The size of the data described by this entry */
    size_t size;

    /*! Attribute flags of the data described by this entry */
    enum mod_n1sdp_flash_data_flag flags;
};

/*!
 * \brief Descriptor for a NFIP table entry.
 */
struct mod_n1sdp_fip_descriptor {
    /*! The type of the NFIP table entry */
    enum mod_n1sdp_fip_data_type type;

    /*!
     * The absolute address of the start of the data described by this
     * entry.
     */
    uintptr_t address;

    /*! The size of the data described by this entry */
    size_t size;

    /*! Attribute flags of the data described by this entry */
    enum mod_n1sdp_flash_data_flag flags;
};

/*!
 * \brief N1SDP Flash Module configuration
 */
struct mod_n1sdp_flash_config {
    /*! Base address of the memory-mapped flash memory. */
    uintptr_t flash_base_address;

    /*! Alternate base address of the memory-mapped flash memory. */
    uintptr_t flash_base_address_alt;

    /*! Offset of the Flash Table of Contents (TOC) from flash base address. */
    uintptr_t toc_offset;
};

/*!
 * \brief APIs to access the descriptors in the flash memory.
 */
struct mod_n1sdp_flash_api {
    /*!
     * \brief Get the total number of flash descriptors.
     *
     * \param id Module identifier.
     * \param[out] count The descriptor count.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_PARAM The \p count parameter was invalid.
     * \return One of the other specific error codes described by framework.
     */
    int (*get_flash_descriptor_count)(fwk_id_t id, unsigned int *count);

    /*!
     * \brief Get a pointer to the table of descriptors that describe the flash
     *     memory.
     *
     * \param id Module identifier.
     * \param[out] table The descriptor table.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_PARAM The \p table parameter was invalid.
     * \return One of the other specific error codes described by framework.
     */
    int (*get_flash_descriptor_table)(
        fwk_id_t id,
        struct mod_n1sdp_flash_descriptor **table);
    /*!
     * \brief Get the total number of NFIP descriptors.
     *
     * \param id Module identifier.
     * \param[out] count The descriptor count.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_PARAM The \p count parameter was invalid.
     * \return One of the other specific error codes described by framework.
     */
    int (*get_n1sdp_fip_descriptor_count)(fwk_id_t id, unsigned int *count);

    /*!
     * \brief Get a pointer to the table of descriptors that describe the FIP
     *     memory.
     *
     * \param id Module identifier.
     * \param[out] table The descriptor table.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_PARAM The \p table parameter was invalid.
     * \return One of the other specific error codes described by framework.
     */
    int (*get_n1sdp_fip_descriptor_table)(
        fwk_id_t id,
        struct mod_n1sdp_fip_descriptor **table);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_N1SDP_FLASH_H */
