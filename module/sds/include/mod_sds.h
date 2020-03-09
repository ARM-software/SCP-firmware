/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SDS_H
#define MOD_SDS_H

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupSDS Shared Data Storage
 *
 * \details Provides a framework for the structured storage of data that is
 *      shared between the SCP Firmware and application processor firmware.
 *
 * @{
 */

/*
 * Bit positions for the fields that make up a structure's unique,
 * 32-bit identifier. This value consists of the 16-bit ID, an 8-bit major
 * version number and an 8-bit minor version number.
 */

/*! Position of the identifier field. */
#define MOD_SDS_ID_IDENTIFIER_POS    0
/*! Position of the minor version field. */
#define MOD_SDS_ID_VERSION_MINOR_POS 16
/*! Position of the major version field. */
#define MOD_SDS_ID_VERSION_MAJOR_POS 24

/*
 * Masks for the structure identifier fields.
 */

/*! Mask for the identifier field. */
#define MOD_SDS_ID_IDENTIFIER_MASK    0x0000FFFF
/*! Mask for the minor version field. */
#define MOD_SDS_ID_VERSION_MINOR_MASK 0x00FF0000
/*! Mask for the major version field. */
#define MOD_SDS_ID_VERSION_MAJOR_MASK 0xFF000000

/*!
 * \brief Element descriptor that describes an SDS structure that will be
 *      automatically created during element initialization.
 */
struct mod_sds_structure_desc {
    /*! Identifier of the structure to be created. */
    uint32_t id;
    /*! Size, in bytes, of the structure. */
    size_t size;
    /*!
     *  Payload of the structure. If not equal to NULL, as part of the
     *  initialization of the module's elements, the payload of the structure
     *  identified by 'id' is initalized/updated to the value pointed to by
     *  'payload'.
     */
    const void *payload;
    /*! Set the valid flag in the structure if true. */
    bool finalize;
};

/*!
 * \brief Module configuration.
 */
struct mod_sds_config {
    /*! Base address of the region used for shared data storage */
    uintptr_t region_base_address;

    /*!
     * Size, in bytes, of the SDS Memory Region. If the SDS module is
     *      initialized and a region already exists then the region will be
     *      resized to match the size given here. The existing region may be
     *      grown or shrunk, as required. In the latter case the new size must
     *      be sufficient to hold the existing contents and if it is not then
     *      the initialization process will fail.
     */
    size_t region_size;

#if BUILD_HAS_MOD_CLOCK
    /*! Identifier of the clock that this module depends on */
    fwk_id_t clock_id;
#endif
};

/*!
 * \brief SDS notification indices.
 */
enum mod_sds_notification_idx {
    /*! The SDS region has been initialized */
    MOD_SDS_NOTIFICATION_IDX_INITIALIZED,

    /*! Number of defined notifications */
    MOD_SDS_NOTIFICATION_IDX_COUNT
};

/*!
 * \brief Identifier for the ::MOD_SDS_NOTIFICATION_IDX_INITIALIZED
 *     notification.
 */
static const fwk_id_t mod_sds_notification_id_initialized =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_SDS,
        MOD_SDS_NOTIFICATION_IDX_INITIALIZED);

/*!
 * \brief Module interface.
 */
struct mod_sds_api {
    /*!
     * \brief Write data within a Shared Data Structure.
     *
     * \details Write data to a given offset within a Shared Data Structure. The
     *      SDS module ensures that the data being written is properly
     *      contained within the structure but it is the caller's responsibility
     *      to ensure that the correct field or fields are being updated.
     *
     * \note A single write may update a portion of a field, a complete
     *      field, or multiple fields, depending on the definition of the
     *      structure and the size of the data being written.
     *
     * \param structure_id The identifier of the Shared Data Structure into
     *      which data will be written.
     *
     * \param offset The offset, in bytes, of the field within the Shared Data
     *      Structure.
     *
     * \param data Pointer to the data that will be written into the Shared Data
     *      Structure's field.
     *
     * \param size Size, in bytes, of the data to be written.
     *
     * \retval FWK_SUCCESS Data was successfully written to the structure.
     * \retval FWK_E_PARAM The data pointer parameter was NULL.
     * \retval FWK_E_PARAM An invalid structure identifier was provided.
     * \retval FWK_E_RANGE The field extends outside of the structure bounds.
     */
    int (*struct_write)(uint32_t structure_id, unsigned int offset,
                        const void *data, size_t size);

    /*!
     * \brief Read data from within a Shared Data Structure.
     *
     * \details Read data from a given offset within a Shared Data Structure.
     *      The SDS module ensures that the data being read is contained
     *      completely within the structure but it is the caller's
     *      responsibility to ensure that the correct field(s) are being read.
     *
     *      A single read may return only a portion of a field, a complete
     *      field, or multiple fields, depending on the definition of the
     *      structure and the value of the size parameter.
     *
     * \note Reading from a structure that has not been finalized is permitted
     *      but discouraged.
     *
     * \param structure_id The identifier of the Shared Data Structure from
     *      which data will be read.
     *
     * \param offset The offset, in bytes, of the field within the Shared Data
     *      Structure.
     *
     * \param[out] data The field data that will be read.
     *
     * \param size Size, in bytes, of the storage pointed to by the data
     *       parameter.
     *
     * \retval FWK_SUCCESS Data was successfully read from the structure.
     * \retval FWK_E_PARAM The data pointer parameter was NULL.
     * \retval FWK_E_PARAM An invalid structure identifier was provided.
     * \retval FWK_E_RANGE The field extends outside of the structure bounds.
     */
    int (*struct_read)(uint32_t structure_id, unsigned int offset, void *data,
                       size_t size);

    /*!
     * \brief Mark a Shared Data Structure as valid and ready for use by
     *      application processor firmware.
     *
     * \param structure_id The identifier of the Shared Data Structure to
     *      finalize.
     *
     * \retval FWK_SUCCESS The structure was successfully finalized.
     * \retval FWK_E_PARAM An invalid structure identifier was provided.
     * \retval FWK_E_STATE The structure has already been finalized.
     */
    int (*struct_finalize)(uint32_t structure_id);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SDS_H */
