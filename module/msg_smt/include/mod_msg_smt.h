/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     msg buffer device driver.
 */

#ifndef MOD_MSG_SMT_H
#define MOD_MSG_SMT_H

#include <stddef.h>
#include <stdint.h>
#include <fwk_id.h>

/*!
 * \brief Channel type
 *
 * \details Defines the role of an entity in a channel
 */
enum mod_msg_smt_channel_type {
    /*! Requester channel */
    MOD_MSG_SMT_CHANNEL_TYPE_REQUESTER,

    /*! Completer channel */
    MOD_MSG_SMT_CHANNEL_TYPE_COMPLETER,

    /*! Channel type count */
    MOD_MSG_SMT_CHANNEL_TYPE_COUNT,
};

/*!
 * \brief Channel config.
 */
struct mod_msg_smt_channel_config {
    /*! Channel role (requester or completer) */
    enum mod_msg_smt_channel_type type;

    /*! Shared mailbox size in bytes */
    size_t mailbox_size;

    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;
};

/*!
 * \brief Driver input API (Implemented by SMT)
 *
 * \details Interface used for driver -> SMT communication.
 */
struct mod_msg_smt_driver_input_api {
    /*!
     * \brief Signal an incoming message in the mailbox
     *
     * \param device_id Channel identifier
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*signal_message)(fwk_id_t channel_id, void *msg_in, size_t in_len, void *msg_out, size_t out_len);
};

/*!
 * \brief Driver output API (Implemented by MHU)
 *
 * \details Interface used for SMT -> driver communication.
 */
struct mod_msg_smt_driver_ouput_api {
    /*!
     * \brief Signal an incoming message in the mailbox
     *
     * \param device_id Channel identifier
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*raise_notification)(fwk_id_t channel_id, size_t size);
};

/*!
 * \brief Type of the interfaces exposed by the power domain module.
 */
enum mod_msg_smt_api_idx {
    MOD_MSG_SMT_API_IDX_SCMI_TRANSPORT,
    MOD_MSG_SMT_API_IDX_DRIVER_INPUT,
    MOD_MSG_SMT_API_IDX_COUNT,
};

#endif /* MOD_MSG_SMT_H */
