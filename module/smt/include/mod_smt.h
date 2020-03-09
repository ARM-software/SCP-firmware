/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SMT_H
#define MOD_SMT_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \name Channel policies
 *
 * \details These policies define attributes that affect how the channel is
 * treated by the SMT component.
 *
 * @{
 */

/*! No policies */
#define MOD_SMT_POLICY_NONE         ((uint32_t)0)

/*! This channel is secure */
#define MOD_SMT_POLICY_SECURE       ((uint32_t)(1 << 0))

/*! The mailbox for this channel requires initialization */
#define MOD_SMT_POLICY_INIT_MAILBOX ((uint32_t)(1 << 1))

/*!
 * @}
 */

/*!
 * \brief Channel type
 *
 * \details Defines the role of an entity in a channel
 */
enum mod_smt_channel_type {
    /*! Master channel */
    MOD_SMT_CHANNEL_TYPE_MASTER,

    /*! Slave channel */
    MOD_SMT_CHANNEL_TYPE_SLAVE,

    /*! Channel type count */
    MOD_SMT_CHANNEL_TYPE_COUNT,
};

/*!
 * \brief Channel config.
 */
struct mod_smt_channel_config {
    /*! Channel role (slave or master) */
    enum mod_smt_channel_type type;

    /*! Channel policies */
    uint32_t policies;

    /*! Shared mailbox address */
    uintptr_t mailbox_address;

    /*! Shared mailbox size in bytes */
    size_t mailbox_size;

    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;

    /*! Identifier of the power domain that this channel depends on */
    fwk_id_t pd_source_id;
};

/*!
 * \brief Driver API
 */
struct mod_smt_driver_api {
    /*!
     * \brief Raise an interrupt on the receiver
     *
     * \param device_id Device identifier
     *
     * \retval FWK_SUCCESS The operation succeeded
     * \retval FWK_E_PARAM The device_id parameter is invalid
     * \return One of the standard error codes for implementation-defined
     * errors
     */
    int (*raise_interrupt)(fwk_id_t device_id);
};

/*!
 * \brief Driver input API (Implemented by SMT)
 *
 * \details Interface used for driver -> SMT communication.
 */
struct mod_smt_driver_input_api {
    /*!
     * \brief Signal an incoming message in the mailbox
     *
     * \param device_id Channel identifier
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*signal_message)(fwk_id_t channel_id);
};

/*!
 * \brief Type of the interfaces exposed by the power domain module.
 */
enum mod_smt_api_idx {
    MOD_SMT_API_IDX_DRIVER_INPUT,
    MOD_SMT_API_IDX_SCMI_TRANSPORT,
    MOD_SMT_API_IDX_COUNT,
};

/*!
 * \brief SMT notification indices.
 */
enum mod_smt_notification_idx {
    /*! The SMT channel has been initialized */
    MOD_SMT_NOTIFICATION_IDX_INITIALIZED,

    /*! Number of defined notifications */
    MOD_SMT_NOTIFICATION_IDX_COUNT
};

/*!
 * \brief Identifier for the MOD_SMT_NOTIFICATION_IDX_INITIALIZED
 *     notification.
 */
static const fwk_id_t mod_smt_notification_id_initialized =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_SMT,
        MOD_SMT_NOTIFICATION_IDX_INITIALIZED);

#endif /* MOD_SMT_H */
