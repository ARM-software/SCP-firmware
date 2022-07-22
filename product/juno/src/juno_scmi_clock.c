/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Clock Management Protocol Support.
 */
#include <mod_scmi.h>
#include <mod_scmi_clock.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_status.h>

#include <inttypes.h>

int mod_scmi_clock_rate_set_policy(
    enum mod_scmi_clock_policy_status *policy_status,
    enum mod_clock_round_mode *round_mode,
    uint64_t *rate,
    enum mod_scmi_clock_policy_commit policy_commit,
    fwk_id_t service_id,
    uint32_t clock_dev_id)
{
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_DEBUG
    FWK_LOG_DEBUG(
        "[SCMI-CLK] Set Clock Rate Policy Handler agent: %u clock: %" PRIu32
        "\n",
        fwk_id_get_element_idx(service_id),
        clock_dev_id);
#endif

    *policy_status = MOD_SCMI_CLOCK_EXECUTE_MESSAGE_HANDLER;

    return FWK_SUCCESS;
}
