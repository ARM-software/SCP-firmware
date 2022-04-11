/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Header file for FIP module used to parse the firmware
 *     package.
 */

#ifndef MOD_FIP_H
#define MOD_FIP_H

#include <fwk_attributes.h>

#include <stddef.h>
#include <stdint.h>

#define FIP_UUID_ENTRY_SIZE 16
#define FIP_TOC_HEADER_NAME UINT32_C(0xAA640001)

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
    /*! NULL entry at end of fip image */
    MOD_FIP_TOC_ENTRY_NULL,
    /*! SCP Firmware BL2 */
    MOD_FIP_TOC_ENTRY_SCP_BL2,
    /*! TF-A runtime firmware BL31 */
    MOD_FIP_TOC_ENTRY_TFA_BL31,
    /*! Number of default enteries */
    MOD_FIP_TOC_ENTRY_COUNT,
};

/*
 * Standard UUIDs as defined by TF-A.
 */

/*! NULL entry at end of fip image */
#define FIP_UUID_NULL \
    (struct fip_uuid_desc) \
    { \
        .image_type = MOD_FIP_TOC_ENTRY_NULL, .uuid = { \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00, \
            0x00 \
        } \
    }

/*! SCP Firmware BL2 */
#define FIP_UUID_SCP_BL2 \
    (struct fip_uuid_desc) \
    { \
        .image_type = MOD_FIP_TOC_ENTRY_SCP_BL2, .uuid = { \
            0x97, \
            0x66, \
            0xfd, \
            0x3d, \
            0x89, \
            0xbe, \
            0xe8, \
            0x49, \
            0xae, \
            0x5d, \
            0x78, \
            0xa1, \
            0x40, \
            0x60, \
            0x82, \
            0x13 \
        } \
    }

/*! TF-A runtime firmware BL31 */
#define FIP_UUID_TFA_BL31 \
    (struct fip_uuid_desc) \
    { \
        .image_type = MOD_FIP_TOC_ENTRY_TFA_BL31, .uuid = { \
            0x47, \
            0xd4, \
            0x08, \
            0x6d, \
            0x4c, \
            0xfe, \
            0x98, \
            0x46, \
            0x9b, \
            0x95, \
            0x29, \
            0x50, \
            0xcb, \
            0xbd, \
            0x5a, \
            0x00 \
        } \
    }

/*!
 * \}
 */

/*
 * Firmware Image Package (FIP) layout.
 *
 * The FIP layout consists of a table of contents (ToC) followed by payload
 * data.
 *
 * The ToC itself has a header followed by one or more table entries. The ToC
 * is terminated by an end marker entry.
 *
 * All ToC entries describe some payload data that has been appended to the end
 * of the binary package. With the information provided in the ToC entry the
 * corresponding payload data can be retrieved.
 *
 *  +----------------+
 *  | ToC Header     |
 *  +----------------+
 *  | ToC Entry 0    |
 *  +----------------+
 *  | ToC Entry 1    |
 *  +----------------+
 *  | ToC End Marker |
 *  +----------------+
 *  |                |
 *  |     Data 0     |
 *  |                |
 *  +----------------+
 *  |                |
 *  |     Data 1     |
 *  |                |
 *  +----------------+
 */

/*!
 * \brief An enum entry to UUID mapping structure
 */
struct fip_uuid_desc {
    /*! Enum entry for Image type */
    enum mod_fip_toc_entry_type image_type;
    /*! Entry UUID */
    uint8_t uuid[FIP_UUID_ENTRY_SIZE];
};

/*!
 * \brief The ToC header fields structure
 */
struct FWK_PACKED fip_toc_header {
    /*! The name of the ToC */
    uint32_t name;
    /*! A non-zero number provided by the creation tool */
    uint32_t serial_number;
    /*! Flags associated with this data */
    uint64_t flags;
};

/*!
 * \brief A ToC entry fields structure
 */
struct FWK_PACKED fip_toc_entry {
    /*! Entry UUID */
    uint8_t uuid[FIP_UUID_ENTRY_SIZE];
    /*! Offset address of payload data */
    uint64_t offset_address;
    /*! Size of the corresponding payload data in bytes */
    uint64_t size;
    /*! Flags associated with this entry */
    uint64_t flags;
};

/*!
 * \brief Firmware Image Package (FIP) layout
 */
struct FWK_PACKED fip_toc {
    /*! ToC header */
    struct fip_toc_header header;
    /*! ToC table entries */
    struct fip_toc_entry entry[];
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
     * \param image_type FIP ToC entry type.
     * \param[out] entry Updated if the type of entry requested is found.
     * \param base Base address where FIP ToC resides.
     * \param limit Maximum size of the media where FIP ToC resides.

     * \retval ::FWK_SUCCESS Entry found and \p entry updated.
     * \retval ::FWK_E_INIT Underlying storage could not be parsed (during
     *      init).
     * \retval ::FWK_E_RANGE No entry of type \p type could be located.
     * \retval ::FWK_E_DATA FIP ToC corrupted or otherwise not usable on this
     *         platform.
     */
    int (*get_entry)(
        enum mod_fip_toc_entry_type image_type,
        struct mod_fip_entry_data *entry,
        uintptr_t base,
        size_t limit);
};

/*!
 * \brief fip module configuration.
 */
struct mod_fip_module_config {
    /*! Array of custom fip_uuid_desc entries */
    struct fip_uuid_desc *custom_fip_uuid_desc_arr;
    /*! Custom array element count */
    size_t custom_uuid_desc_count;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_FIP_H */
