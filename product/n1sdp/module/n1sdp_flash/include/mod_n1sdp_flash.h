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
 * \defgroup GroupModuleN1SDPFlashEntries Flash partition identifiers.
 * \{
 */
struct mod_n1sdp_flash_entry_id;

/*! MCP BL2 Binary */
extern const struct mod_n1sdp_flash_entry_id mod_n1sdp_flash_entry_mcp_bl2;
/*! SCP BL2 Binary */
extern const struct mod_n1sdp_flash_entry_id mod_n1sdp_flash_entry_scp_bl2;
/*! EL3 Runtime - BL31 Binary */
extern const struct mod_n1sdp_flash_entry_id mod_n1sdp_flash_entry_tf_bl31;

/*!
 * \}
 */


/*!
 * \brief N1SDP Flash Module configuration
 */
struct mod_n1sdp_flash_config {
    /*! Base address of the memory-mapped flash memory. */
    uintptr_t flash_base_address;

    /*! Alternate base address of the memory-mapped flash memory. */
    uintptr_t flash_base_address_alt;
};

/*!
 * \brief A memory-mapped flash entry
 */
struct mod_n1sdp_flash_entry {
    /*! Start of entry data */
    void * p;
    /*! Size */
    size_t size;
    /*! Flags */
    uint64_t flags;
};

/*!
 * \brief APIs to access the descriptors in the flash memory.
 */
struct mod_n1sdp_flash_api {
    /*!
     * \brief Retrieve a flash entry.
     *
     * \param id Flash entry identifiery.
     * \param[out] entry Updated if requested entry if found.

     * \retval FWK_SUCCESS Flash entry found and \p entry have been updated.
     * \retval FWK_E_INIT Flash could not be parsed (during init).
     * \retval FWK_E_RANGE Not entry matching \p id could be located.
     * \retval FWK_E_DATA Flash TOC corrupted or otherwise not useable on this
     *         platform.
     */
    int (*get_entry)(
        const struct mod_n1sdp_flash_entry_id * id,
        struct mod_n1sdp_flash_entry *entry);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_N1SDP_FLASH_H */
