/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Power Requester unit test support.
 */
#include <mod_scmi_system_power_req.h>

/*
 * used as a mock for message handlers
 */
int fake_message_handler(
    fwk_id_t service_id,
    const void *payload,
    size_t payload_size);
