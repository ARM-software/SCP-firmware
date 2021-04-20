/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     OP-TEE mailbox buffer layer
 */

#ifndef MOD_OPTEE_MHU_H
#define MOD_OPTEE_MHU_H

/* OPTEE_MHU currently uses a static shared memory buffer size */
#define OPTEE_MHU_SHMEM_SIZE 128

/*!
 * \brief Channel config.
 */
struct mod_optee_mhu_channel_config {
    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;
};

/*!
 * \brief Interface to exchange message with OP-TEE
 */

/* Return number of MHU device registered in the paltform */
int optee_mhu_get_devices_count(void);

/* Retrun framwork ID related to MHU device identifed by @id */
fwk_id_t optee_mhu_get_device(unsigned int id);

/* Signal an incoming SCMI message in SMT device memory mailbox */
void optee_mhu_signal_smt_message(fwk_id_t device_id);

/* Signal an incoming SCMI message in a MSG OP-TEE shared memory mailbox */
void optee_mhu_signal_msg_message(fwk_id_t device_id,
                                  void *in_buf, size_t in_size,
                                  void *out_buf, size_t *out_size);

#endif /* MOD_OPTEE_MHU_H */
