/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_scmi_clock.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define FAKE_MODULE_IDX 0x5

enum fake_scmi_agent {
    FAKE_SCMI_AGENT_IDX_PSCI = 1,
    FAKE_SCMI_AGENT_IDX_OSPM0,
    FAKE_SCMI_AGENT_IDX_OSPM1,
    FAKE_SCMI_AGENT_IDX_COUNT,
};

/*!
 * \brief Clock device indexes.
 */
enum clock_dev_idx {
    CLOCK_DEV_IDX_FAKE0,
    CLOCK_DEV_IDX_FAKE1,
    CLOCK_DEV_IDX_FAKE2,
    CLOCK_DEV_IDX_FAKE3,
    CLOCK_DEV_IDX_COUNT
};

/*!
 * \brief OSPM0 SCMI Clock indexes.
 */
enum scmi_clock_ospm0_idx {
    SCMI_CLOCK_OSPM0_IDX0,
    SCMI_CLOCK_OSPM0_IDX1,
    SCMI_CLOCK_OSPM0_IDX2,
    SCMI_CLOCK_OSPM0_IDX3,
    SCMI_CLOCK_OSPM0_COUNT
};

/*!
 * \brief OSPM1 SCMI Clock indexes.
 */
enum scmi_clock_ospm1_idx {
    SCMI_CLOCK_OSPM1_IDX0,
    SCMI_CLOCK_OSPM1_COUNT
};


static const struct mod_scmi_clock_device agent_device_table_ospm0
    [SCMI_CLOCK_OSPM0_COUNT] = {
    [SCMI_CLOCK_OSPM0_IDX0] = {
        /* FAKE0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_FAKE0),
        .starts_enabled = true,
    },
    [SCMI_CLOCK_OSPM0_IDX1] = {
        /* FAKE1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_FAKE1),
        .starts_enabled = true,
    },
    [SCMI_CLOCK_OSPM0_IDX2] = {
        /* FAKE2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_FAKE2),
        .starts_enabled = true,
    },
    [SCMI_CLOCK_OSPM0_IDX3] = {
        /* FAKE3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_FAKE3),
        .starts_enabled = true,
    },
};

static const struct mod_scmi_clock_device agent_device_table_ospm1
    [SCMI_CLOCK_OSPM1_COUNT] = {
    [SCMI_CLOCK_OSPM1_IDX0] = {
        /* FAKE3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_FAKE3),
        .starts_enabled = true,
    },
};

static const struct mod_scmi_clock_agent agent_table
        [FAKE_SCMI_AGENT_IDX_COUNT] = {
    [FAKE_SCMI_AGENT_IDX_PSCI] = { 0 /* No access */ },
    [FAKE_SCMI_AGENT_IDX_OSPM0] = {
        .device_table = agent_device_table_ospm0,
        .device_count = FWK_ARRAY_SIZE(agent_device_table_ospm0),
    },
    [FAKE_SCMI_AGENT_IDX_OSPM1] = {
        .device_table = agent_device_table_ospm1,
        .device_count = FWK_ARRAY_SIZE(agent_device_table_ospm1),
    },
};

struct fwk_module_config config_scmi_clock = {
    .data = &((struct mod_scmi_clock_config) {
        .max_pending_transactions = 0,
        .agent_table = agent_table,
        .agent_count = FWK_ARRAY_SIZE(agent_table),
    }),
};

static struct clock_operations clock_ops_table[CLOCK_DEV_IDX_COUNT];

static uint8_t agent_clock_state_table
    [FAKE_SCMI_AGENT_IDX_COUNT * CLOCK_DEV_IDX_COUNT];

static uint8_t dev_clock_ref_count_table[CLOCK_DEV_IDX_COUNT];

static uint8_t agent_clock_state_table_expected
    [FAKE_SCMI_AGENT_IDX_COUNT * CLOCK_DEV_IDX_COUNT];

static uint8_t dev_clock_ref_count_table_expected[CLOCK_DEV_IDX_COUNT];

static const uint8_t agent_clock_state_table_default
    [FAKE_SCMI_AGENT_IDX_COUNT * CLOCK_DEV_IDX_COUNT] = {
        /* INVALID AGENT */
        MOD_CLOCK_STATE_STOPPED,
        MOD_CLOCK_STATE_STOPPED,
        MOD_CLOCK_STATE_STOPPED,
        MOD_CLOCK_STATE_STOPPED,
        /* FAKE_SCMI_AGENT_IDX_PSCI */
        MOD_CLOCK_STATE_STOPPED,
        MOD_CLOCK_STATE_STOPPED,
        MOD_CLOCK_STATE_STOPPED,
        MOD_CLOCK_STATE_STOPPED,
        /* FAKE_SCMI_AGENT_IDX_OSPM0 */
        MOD_CLOCK_STATE_RUNNING,
        MOD_CLOCK_STATE_RUNNING,
        MOD_CLOCK_STATE_RUNNING,
        MOD_CLOCK_STATE_RUNNING,
        /* FAKE_SCMI_AGENT_IDX_OSPM1 */
        MOD_CLOCK_STATE_RUNNING,
        MOD_CLOCK_STATE_STOPPED,
        MOD_CLOCK_STATE_STOPPED,
        MOD_CLOCK_STATE_STOPPED,
    };

static const uint8_t dev_clock_ref_count_table_default[CLOCK_DEV_IDX_COUNT] = {
    1,
    1,
    1,
    2,
};
