/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_FWK_MODULE_MODULE_IDX_H
#define TEST_FWK_MODULE_MODULE_IDX_H

#include <fwk_id.h>

enum fwk_module_idx {
    FWK_MODULE_IDX_THERMAL_MGMT,
    FWK_MODULE_IDX_DVFS,
    FWK_MODULE_IDX_SCMI_PERF,
    FWK_MODULE_IDX_SENSOR,
    FWK_MODULE_IDX_FAKE_POWER_MODEL,
    FWK_MODULE_IDX_FAKE_THERMAL_PROTECTION,
    FWK_MODULE_IDX_FAKE_ACTIVITY_COUNTER,
    FWK_MODULE_IDX_COUNT,
};

static const fwk_id_t fwk_module_id_thermal_mgmt =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_THERMAL_MGMT);

static const fwk_id_t fwk_module_id_dvfs =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_DVFS);

static const fwk_id_t fwk_module_id_scmi_perf =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_PERF);

static const fwk_id_t fwk_module_id_sensor =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SENSOR);

static const fwk_id_t fwk_module_id_fake_power_model =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FAKE_POWER_MODEL);

static const fwk_id_t fwk_module_id_fake_thermal_protection =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FAKE_THERMAL_PROTECTION);

static const fwk_id_t fwk_module_id_fake_activity_counter =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FAKE_ACTIVITY_COUNTER);

/* Required to mock the existence of a thermal protection implementation */
static const fwk_id_t mod_fake_thermal_protection_api_id =
    FWK_ID_API_INIT(FWK_MODULE_IDX_FAKE_THERMAL_PROTECTION, 0);

#endif /* TEST_FWK_MODULE_MODULE_IDX_H */
