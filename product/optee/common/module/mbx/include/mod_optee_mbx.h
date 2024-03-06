/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     OP-TEE mailbox buffer layer
 */

#ifndef MOD_OPTEE_MBX_H
#define MOD_OPTEE_MBX_H

/*!
 * \brief Channel config.
 */
struct mod_optee_mbx_channel_config {
    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;
};

/*!
 * \brief Interface to exchange message with OP-TEE
 */

/*!
 * \brief Return the number of MBX devices registered in the paltform.
 *
 * \return Return the number of devices.
 */
int optee_mbx_get_devices_count(void);

/*!
 * \brief Return the framwork ID related to the MBX device identifed by @id.
 *
 * \param id Device index.
 *
 * \return Return the framwork ID
 */
fwk_id_t optee_mbx_get_device(unsigned int id);

/*!
 * \brief Signal an incoming SCMI message in a static shared memory.
 *
 * \param device_id MBX device ID.
 */
void optee_mbx_signal_smt_message(fwk_id_t device_id);

/*  */
/*!
 * \brief Signal an incoming SCMI message in an OP-TEE dynamic shared memory.
 *
 * \param device_id MBX device ID.
 * \param in_buf Pointer to the request buffer.
 * \param in_size Size of the request buffer.
 * \param out_buf Pointer to the response buffer.
 * \param out_size Size of the response buffer.
 */
void optee_mbx_signal_msg_message(
    fwk_id_t device_id,
    void *in_buf,
    size_t in_size,
    void *out_buf,
    size_t *out_size);

#endif /* MOD_OPTEE_MBX_H */
