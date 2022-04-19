/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWU_MDATA_H
#define FWU_MDATA_H

#include <efi.h>

/*!
 * \brief firmware image information
 *
 * \details The structure contains image specific fields which are
 * used to identify the image and to specify the image's
 * acceptance status
 */
struct fwu_image_bank_info {
    /*! Guid value of the image in this bank */
    efi_guid_t  image_uuid;

    /*! Acceptance status of the image */
    uint32_t accepted;

    /*! Reserved */
    uint32_t reserved;
} __attribute__((__packed__));

/*!
 * \brief information for a particular type of image
 *
 * \details This structure contains information on various types of updatable
 * firmware images. Each image type then contains an array of image
 * information per bank.
 */
struct fwu_image_entry {
    /*! Guid value for identifying the image type */
    efi_guid_t image_type_uuid;

    /*! Guid of the storage volume where the image is located */
    efi_guid_t location_uuid;

    /*! Array containing properties of images */
    struct fwu_image_bank_info img_bank_info[CONFIG_FWU_NUM_BANKS];
} __attribute__((__packed__));

/*!
 * \brief FWU metadata structure for multi-bank updates
 *
 * \details This structure is used to store all the needed information for
 * performing multi bank updates on the platform. This contains info on the
 * bank being used to boot along with the information needed for
 * identification of individual images.
 */
struct fwu_mdata {
    /*! crc32 value for the FWU metadata */
    uint32_t crc32;

    /*! FWU metadata version */
    uint32_t version;

    /*! Index of the bank currently used for booting images */
    uint32_t active_index;

    /*! Index of the bank used before the current bank being used for booting */
    uint32_t previous_active_index;

    /*! Array of information on various firmware images that can be updated */
    struct fwu_image_entry img_entry[CONFIG_FWU_NUM_IMAGES_PER_BANK];
} __attribute__((__packed__));

#endif /* FWU_MDATA_H */
