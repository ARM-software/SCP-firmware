/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_ATU_H
#define MOD_ATU_H

#include <fwk_attributes.h>
#include <fwk_id.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*! Mask to read an attribute from ATUROBA register */
#define ATU_ROBA_ATTRIBUTE_MASK (0x3u)

/*! ATU ROBA Register AxNSE bit field offset */
#define ATU_ATUROBA_AXNSE_OFFSET (14u)

/*! ATU ROBA Register AxCACHE3 bit field offset */
#define ATU_ATUROBA_AXCACHE3_OFFSET (12u)

/*! ATU ROBA Register AxCACHE2 bit field offset */
#define ATU_ATUROBA_AXCACHE2_OFFSET (10u)

/*! ATU ROBA Register AxCACHE1 bit field offset */
#define ATU_ATUROBA_AXCACHE1_OFFSET (8u)

/*! ATU ROBA Register AxCACHE0 bit field offset */
#define ATU_ATUROBA_AXCACHE0_OFFSET (6u)

/*! ATU ROBA Register AxPROT2 bit field offset */
#define ATU_ATUROBA_AXPROT2_OFFSET (4u)

/*! ATU ROBA Register AxPROT1 bit field offset */
#define ATU_ATUROBA_AXPROT1_OFFSET (2u)

/*! ATU ROBA Register AxPROT0 bit field offset */
#define ATU_ATUROBA_AXPROT0_OFFSET (0u)

/*! Macro to encode the output bus attributes */
#define ATU_ENCODE_ATTRIBUTES( \
    AXNSE_VAL, \
    AXCACHE3_VAL, \
    AXCACHE2_VAL, \
    AXCACHE1_VAL, \
    AXCACHE0_VAL, \
    AXPROT2_VAL, \
    AXPROT1_VAL, \
    AXPROT0_VAL) \
    (AXPROT0_VAL | (AXPROT1_VAL << ATU_ATUROBA_AXPROT1_OFFSET) | \
     (AXPROT2_VAL << ATU_ATUROBA_AXPROT2_OFFSET) | \
     (AXCACHE0_VAL << ATU_ATUROBA_AXCACHE0_OFFSET) | \
     (AXCACHE1_VAL << ATU_ATUROBA_AXCACHE1_OFFSET) | \
     (AXCACHE2_VAL << ATU_ATUROBA_AXCACHE2_OFFSET) | \
     (AXCACHE3_VAL << ATU_ATUROBA_AXCACHE3_OFFSET) | \
     (AXNSE_VAL << ATU_ATUROBA_AXNSE_OFFSET))

/*
 * Encode ATU region output bus attributes for accessing Root PAS
 */
#define ATU_ENCODE_ATTRIBUTES_ROOT_PAS \
    (ATU_ENCODE_ATTRIBUTES( \
        MOD_ATU_ROBA_SET_1, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0))

/*
 * Encode ATU region output bus attributes for accessing Secure PAS
 */
#define ATU_ENCODE_ATTRIBUTES_SECURE_PAS \
    (ATU_ENCODE_ATTRIBUTES( \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0))

/*
 * Encode ATU region output bus attributes for accessing Non-Secure PAS
 */
#define ATU_ENCODE_ATTRIBUTES_NON_SECURE_PAS \
    (ATU_ENCODE_ATTRIBUTES( \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_1, \
        MOD_ATU_ROBA_SET_0))

/*
 * Encode ATU region output bus attributes for accessing Realm PAS
 */
#define ATU_ENCODE_ATTRIBUTES_REALM_PAS \
    (ATU_ENCODE_ATTRIBUTES( \
        MOD_ATU_ROBA_SET_1, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_0, \
        MOD_ATU_ROBA_SET_1, \
        MOD_ATU_ROBA_SET_0))

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupATU Address Translation Unit Driver
 * \{
 */

/*!
 * \brief ATU Region Output Bus Attributes
 */
enum mod_atu_roba {
    /*! AxPROT[0] attribute */
    MOD_ATU_ROBA_AXPROT0,
    /*! AxPROT[1] attribute */
    MOD_ATU_ROBA_AXPROT1,
    /*! AxPROT[2] attribute */
    MOD_ATU_ROBA_AXPROT2,
    /*! AxCACHE[0] attribute*/
    MOD_ATU_ROBA_AXCACHE0,
    /*! AxCACHE[1] attribute*/
    MOD_ATU_ROBA_AXCACHE1,
    /*! AxCACHE[2] attribute*/
    MOD_ATU_ROBA_AXCACHE2,
    /*! AxCACHE[3] attribute*/
    MOD_ATU_ROBA_AXCACHE3,
    /*! AxNSE attribute */
    MOD_ATU_ROBA_AXNSE,
    /*! Output bus attribute count */
    MOD_ATU_ROBA_COUNT,
};

/*!
 * \brief ATU Output Bus Attributes Values
 */
enum mod_atu_roba_value {
    /*! The attribute is set to passthrough */
    MOD_ATU_ROBA_PASSTHROUGH,
    /*! Reserved */
    MOD_ATU_ROBA_RESERVED,
    /*! The attribute is set to 0 */
    MOD_ATU_ROBA_SET_0,
    /*! The attribute is set to 1 */
    MOD_ATU_ROBA_SET_1,
    /*! The attribute value count */
    MOD_ATU_ROBA_VALUE_COUNT,
};

/*!
 * ATU translation region info data.
 */
struct FWK_PACKED atu_region_map {
    /*! Identifier of the entity that owns the ATU region */
    fwk_id_t region_owner_id;
    /*! Start logical address of the region to be mapped */
    uint32_t log_addr_base;
    /*! Start physical address of the region to be mapped */
    uint64_t phy_addr_base;
    /*! Size of the region to be mapped */
    size_t region_size;
    /*! Output bus attributes to be configured for the region */
    uint32_t attributes;
};

/*!
 * \brief ATU module configuration data.
 */
struct mod_atu_device_config {
#if defined(BUILD_HAS_ATU_MANAGE)
    /*! Base address of the ATU register */
    uintptr_t atu_base;

    /* Table of regions to be configured in the ATU */
    const struct atu_region_map *atu_region_config_table;

    /* Number of entries in the \ref atu_region_config_table */
    size_t atu_region_count;
#endif
#if defined(BUILD_HAS_ATU_DELEGATE)
    /*!
     * Transport channel identifier
     *
     * Note: ATU_DELEGATE mode requires the transport module for sending
     * and receiving messages. So, transport module must be included in the
     * firmware build to use this mode.
     */
    fwk_id_t transport_id;

    /*! Timer identifier */
    fwk_id_t timer_id;

    /*!
     * Response message wait timeout in microseconds.
     * A valid non-zero value must be specified.
     */
    uint32_t response_wait_timeout_us;
#endif
    /*! Flag to indicate whether ATU configuration is delegated */
    bool is_atu_delegated;
};

/*!
 * \brief ATU API
 *
 * \details Interface used by other modules to configure translation regions.
 */
struct mod_atu_api {
    /*!
     * \brief Configure translation region.
     *
     * \note The requested ATU region must be algined with the ATU page size.
     *
     * \param region Info of the translation region to be configured.
     * \param atu_device_id Identifier of the ATU device.
     * \param[out] region_idx Index of the mapped translation region.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered.
     *      - The requested ATU region is not aligned with the ATU page size.
     *      - Invalid attributes configuration data was provided in the `region`
     *      parameter.
     *      - The `region_idx` parameter was null pointer value.
     *      - The`region` parameter was null pointer value.
     *      - The size of the payload to be sent exceeds the size of the
     *      payload buffer.
     * \retval ::FWK_E_TIMEOUT No response received.
     * \retval ::FWK_E_SUPPORT Operation not supported.
     *      - The region exceeds the total number of active translation regions
     *      supported by the ATU.
     *      - The requested translation region overlaps an existing translation
     *      region.
     * \retval ::FWK_E_BUSY The previous message has not been read.
     * \retval ::FWK_E_DATA Error in the response message received.
     */
    int (*add_region)(
        const struct atu_region_map *region,
        fwk_id_t atu_device_id,
        uint8_t *region_idx);

    /*!
     * \brief Remove translation region.
     *
     * \param region_idx Index of the translation region to be removed.
     * \param atu_device_id Identifier of the ATU device.
     * \param requester_id Identifier of the entity that's requesting the
     *      removal of the ATU region.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered.
     * \retval ::FWK_E_ACCESS The requester does not have the ownership of the
     *      ATU region.
     * \retval ::FWK_E_TIMEOUT No response received.
     * \retval ::FWK_E_SUPPORT Operation not supported.
     *      - The requested region number exceeds the total number of active
     *      translation regions supported by the ATU.
     * \retval ::FWK_E_BUSY The previous message has not been read.
     * \retval ::FWK_E_DATA Error in the response message received.
     */
    int (*remove_region)(
        uint8_t region_idx,
        fwk_id_t atu_device_id,
        fwk_id_t requester_id);
};

/*!
 * \brief ATU module API indices.
 */
enum mod_atu_api_idx {
    /*! Interface to configure translation regions in ATU */
    MOD_ATU_API_IDX_ATU,
#if defined(BUILD_HAS_ATU_DELEGATE)
    /*! Interface to transport module */
    MOD_ATU_API_IDX_TRANSPORT_SIGNAL,
#endif
    /*! Total API count */
    MOD_ATU_API_IDX_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_ATU_H */
