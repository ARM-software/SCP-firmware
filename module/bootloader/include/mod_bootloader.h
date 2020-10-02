/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_BOOTLOADER_H
#define MOD_BOOTLOADER_H

#include <fwk_element.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupBoot Bootloader
 *
 * \details A service module providing support for loading firmware images.
 *
 * \{
 */

/*!
 * \brief Module configuration.
 */
struct mod_bootloader_config {
    /*!
     * Base address of the memory region that the image will be copied from.
     * Note that this is not the base address of the image to be loaded.
     *
     * Note that if Shared Data Storage (SDS) is used for the booting process
     * then this is not the base address of the image to be loaded. In this
     * situation the base address is combined with an offset value, provided by
     * the application processor firmware, via a Shared Data Storage region.
     */
    uintptr_t source_base;

    /*!
     * The number of bytes of storage available at the source location.
     * This value implicitly limits the maximum size of the image that can be
     * copied.
     */
    uint32_t source_size;

    /*! Base address of the location that the image will be copied to. */
    uintptr_t destination_base;

    /*!
     * The number of bytes of storage available at the destination location.
     * This value implicitly limits the maximum size of the image that can be
     * copied.
     */
    uint32_t destination_size;

#ifdef BUILD_HAS_MOD_SDS
    /*!
     * Identifier of the SDS structure containing image metadata, such as the
     * size of the image and its offset from source_base.
     */
    uint32_t sds_struct_id;
#endif
};

/*!
 * \brief Bootloader interface.
 */
struct mod_bootloader_api {
    /*!
     * \brief Copy a RAM Firmware image from a source location to a destination
     *      (which is expected to be the SCP SRAM).
     *
     * \param config Pointer to an scp_bootloader_config structure containing
     *      settings that control where the image is copied from and to.
     *
     * \retval ::FWK_SUCCESS The RAM Firmware image was copied successfully.
     * \retval ::FWK_E_ALIGN The given image offset is not properly aligned.
     * \retval ::FWK_E_DATA The image did not pass checksum validation.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered:
     *      - The image source address is invalid.
     *      - The image destination address is invalid.
     *      - The image size is invalid.
     * \retval ::FWK_E_SIZE A size-related issue was encountered:
     *      - The given image size is below the minimum possible size.
     *      - The image is too large for the destination memory area.
     *      - The given image offset is beyond the source memory area.
     *      - The header's size field does not match the given size.
     *      - Part of the image lies outside of the source memory area.
     */
    int (*load_image)(void);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_BOOTLOADER_H */
