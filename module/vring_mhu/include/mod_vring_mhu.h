/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     virtio based transport channel device driver.
 */

#ifndef MOD_VRING_MHU_H
#define MOD_VRING_MHU_H

/*!
 * \brief Channel config.
 */
struct mod_vring_mhu_channel_config {
    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;

    /*! DT node label of the virtio-mmio device */
    const char *device_label;
};

#endif /* MOD_OPTEE_MHU_H */
