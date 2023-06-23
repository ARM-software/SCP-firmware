/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) protocol independent
 *      definitions.
 */

#ifndef MOD_SCMI_POWER_CAPPING_UNIT_TEST_H
#define MOD_SCMI_POWER_CAPPING_UNIT_TEST_H

#define FAKE_SCMI_MODULE_ID          0x5
#define FAKE_POWER_CAPPING_MODULE_ID 0x6

enum fake_services {
    FAKE_SERVICE_IDX_1,
    FAKE_SERVICE_IDX_2,
    FAKE_SERVICE_IDX_COUNT,
};

enum fake_power_capping_domains {
    FAKE_POWER_CAPPING_IDX_1,
    FAKE_POWER_CAPPING_IDX_2,
    FAKE_POWER_CAPPING_IDX_COUNT,
};

#define FAKE_API_IDX_SCMI_TRANSPORT 0x3
#define FAKE_SCMI_AGENT_IDX_PSCI    0x6
#define FAKE_SCMI_AGENT_IDX_OSPM    0x7

#define POWER_CAPPING_NOTIF_SUP_POS 31
#define POWER_MEAS_NOTIF_SUP_POS    30
#define ASYNC_POWER_CAP_POS         29
#define POWER_CAP_CONF_SUP_POS      27
#define POWER_CAP_MON_SUP_POS       26
#define PAI_CONF_SUP_POS            25
#define POWER_UNIT_POS              24
#define FAST_CHANNEL_SUP_POS        22

#define ASYNC_FLAG(x)        (x << 1)
#define IGN_DEL_RESP_FLAG(x) (x)

#define MIN_DEFAULT_PAI (10u)
#define MAX_DEFAULT_PAI (MIN_DEFAULT_PAI * 10u)

#define MIN_DEFAULT_POWER_CAP (10u)
#define MAX_DEFAULT_POWER_CAP (MIN_DEFAULT_POWER_CAP * 10u)

#define DISABLE_CAP_VALUE ((uint32_t)0)

#define POWER_CAP_NOTIFY_ENABLE  (1)
#define POWER_CAP_NOTIFY_DISABLE (0)

#define MEASUREMENTS_NOTIFY_ENABLE  (1)
#define MEASUREMENTS_NOTIFY_DISABLE (0)

#endif /* MOD_SCMI_POWER_CAPPING_UNIT_TEST_H */
