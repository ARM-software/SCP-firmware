/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Framework status code helpers.
 */

#include <fwk_macros.h>
#include <fwk_status.h>

static const char *const error_string[19] = {
    [-FWK_E_PARAM] = "E_PARAM",
    [-FWK_E_ALIGN] = "E_ALIGN",
    [-FWK_E_SIZE] = "E_SIZE",
    [-FWK_E_HANDLER] = "E_HANDLER",
    [-FWK_E_ACCESS] = "E_ACCESS",
    [-FWK_E_RANGE] = "E_RANGE",
    [-FWK_E_TIMEOUT] = "E_TIMEOUT",
    [-FWK_E_NOMEM] = "E_NOMEM",
    [-FWK_E_PWRSTATE] = "E_PWRSTATE",
    [-FWK_E_SUPPORT] = "E_SUPPORT",
    [-FWK_E_DEVICE] = "E_DEVICE",
    [-FWK_E_BUSY] = "E_BUSY",
    [-FWK_E_OS] = "E_OS",
    [-FWK_E_DATA] = "E_DATA",
    [-FWK_E_STATE] = "E_STATE",
    [-FWK_E_INIT] = "E_INIT",
    [-FWK_E_OVERWRITTEN] = "E_OVERWRITTEN",
    [-FWK_E_PANIC] = "E_PANIC",
};

static const char *const status_string[2] = {
    [FWK_SUCCESS] = "SUCCESS",
    [FWK_PENDING] = "PENDING",
};

const char *fwk_status_str(int status)
{
    static const char unknown[] = "unknown";

    unsigned int error_idx = (unsigned int)(-status);

    if ((status < 0) && (error_idx < FWK_ARRAY_SIZE(error_string))) {
        return error_string[error_idx];
    } else if ((status >= 0) && (status < (int)FWK_ARRAY_SIZE(status_string))) {
        return status_string[status];
    }

    return unknown;
}
