/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Header file for FIP module used to parse the firmware
 *     package.
 */

#ifndef MOD_FIP_H
#define MOD_FIP_H

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupFIP Firmware Image Package
 *
 * \brief Provides an API to access firmware image partitions.
 *
 * \details This module provides a table of descriptors that each describe a
 *      block of firmware image entry in the underlying storage.
 *
 * \{
 */

/*!
 * \defgroup GroupModuleFIPEntries FIP entries' identifiers.
 * \{
 */

/*!
 * \brief FIP TOC entry types.
 */
enum mod_fip_toc_entry_type {
    /*! SCP Firmware BL2 */
    MOD_FIP_TOC_ENTRY_SCP_BL2,
    /*! MCP Firmware BL2 */
    MOD_FIP_TOC_ENTRY_MCP_BL2,
    /*! TF-A runtime firmware BL31 */
    MOD_FIP_TOC_ENTRY_TFA_BL31,
};
/*!
 * \}
 */

/*!
 * \brief FIP Module configuration.
 */
struct mod_fip_config {
    /*!
     * Base address of memory-mapped non-volatile platform storage; FIP ToC
     * starts at this address.
     */
    uintptr_t fip_base_address;
    /*! Size of underlying memory-mapped non-volatile platform storage. */
    size_t fip_nvm_size;
};

/*!
 * \brief A memory-mapped FIP entry data (desribes a firmware image).
 */
struct mod_fip_entry_data {
    /*! Start of entry data */
    void *base;
    /*! Size */
    size_t size;
    /*! Flags */
    uint64_t flags;
};

/*!
 * \brief APIs to access the FIP entry data.
 */
struct mod_fip_api {
    /*!
     * \brief Retrieve a FIP entry's data.
     *
     * \param type FIP ToC entry type.
     * \param[out] entry Updated if the type of entry requested is found.

     * \retval ::FWK_SUCCESS Entry found and \p entry updated.
     * \retval ::FWK_E_INIT Underlying storage could not be parsed (during
     *      init).
     * \retval ::FWK_E_RANGE No entry of type \p type could be located.
     * \retval ::FWK_E_DATA FIP ToC corrupted or otherwise not usable on this
     *         platform.
     */
    int (*get_entry)(
        enum mod_fip_toc_entry_type type,
        struct mod_fip_entry_data *entry);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_FIP_H */
