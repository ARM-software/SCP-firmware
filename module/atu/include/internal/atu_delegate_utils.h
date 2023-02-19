/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for the delegating ATU requests.
 */

#ifndef ATU_DELEGATE_UTILS_INTERNAL_H
#define ATU_DELEGATE_UTILS_INTERNAL_H

#include <mod_atu.h>

#include <stddef.h>
#include <stdint.h>

extern const struct mod_transport_firmware_signal_api signal_api;

/*! \brief ATU message identifiers */
enum atu_msg_id {
    /*! Reserved ATU message identifier */
    ATU_MSG_ID_RESERVED,
    /*! Request to configure a translation region */
    ATU_MSG_ID_REQ_ADD_REGION,
    /*! Request to disable a translation region */
    ATU_MSG_ID_REQ_REMOVE_REGION,
    /*! ATU message identifier count */
    ATU_MSG_ID_COUNT,
};

/*! \brief ATU message response */
enum atu_msg_response {
    /*! Reserved ATU message response */
    ATU_MSG_RESPONSE_RESERVED,
    /*! Requested operation succeeded */
    ATU_MSG_RESPONSE_SUCCESS,
    /*! General error */
    ATU_MSG_RESPONSE_GENERAL_ERROR,
    /*! Requested region exceeds the number of regions supported by the ATU */
    ATU_MSG_RESPONSE_INVALID_REGION_ERROR,
    /*! Requested operation is not allowed */
    ATU_MSG_RESPONSE_REGION_NOT_ALLOWED_ERROR,
    /*! Invalid translation region */
    ATU_MSG_RESPONSE_REGION_INVALID_ADDRESS_ERROR,
    /*! Invalid request */
    ATU_MSG_RESPONSE_INVALID_REQUEST_ERROR,
    /* Invalid message payload */
    ATU_MSG_RESPONSE_INVALID_PAYLOAD_ERROR,
    /*! Requested region is owned by a different entity */
    ATU_MSG_RESPONSE_REGION_OWNERSHIP_ERROR,
    /* Region overlaps with an existing ATU region */
    ATU_MSG_RESPONSE_REGION_OVERLAP_ERROR,
    /*! ATU message response count */
    ATU_MSG_RESPONSE_COUNT,
};

/*!
 * \brief Buffer to hold the most recently received message
 */
struct atu_msg_buffer {
    /*! \brief Message header */
    uint32_t header;
    /*! \brief Message payload */
    uint32_t *payload;
    /*! \brief Size of the message payload */
    size_t payload_size;
};

/*!
 * \brief Payload to be sent for \ref ATU_MSG_ID_REQ_REMOVE_REGION message
 */
struct atu_msg_remove_region_payload {
    /*! \brief Index of the region to be removed */
    uint32_t region_idx;
    /*! \brief Identifier of the entity that's requesting the operation */
    uint32_t requester_id;
};

/*!
 * \brief Response to be received for \ref ATU_MSG_ID_REQ_ADD_REGION message
 */
struct atu_msg_add_region_response {
    /*! \brief Status of the requested operation */
    int32_t status;
    /*!
     * \brief Index of the ATU region.
     * \note Only valid when status is ATU_MSG_RESPONSE_SUCCESS
     */
    uint32_t region_idx;
};

/*!
 * \brief Response to be received for \ref ATU_MSG_ID_REQ_REMOVE_REGION message
 */
struct atu_msg_remove_region_response {
    /*! \brief Status of the requested operation */
    int32_t status;
};

/*!
 * \brief Send message to configure a translation region.
 *
 * \param region Pointer to the translation region map info.
 * \param device_ctx Pointer to the ATU device context.
 * \param[out] region_idx Index of the translation region that has been mapped.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int atu_send_msg_add_region(
    const struct atu_region_map *region,
    void *device_ctx,
    uint8_t *region_idx);

/*!
 * \brief Send message to remove a translation region.
 *
 * \param region_idx Index of the translation region to be removed.
 * \param device_ctx Pointer to the ATU device context.
 * \param requester_id identifier of the entity that's requesting the operation.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int atu_send_msg_remove_region(
    const uint8_t region_idx,
    void *device_ctx,
    fwk_id_t requester_id);

#endif /* ATU_DELEGATE_UTILS_INTERNAL_H */
