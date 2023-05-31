/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) unit test support.
 */
#include <mod_dvfs.h>
#include <mod_scmi.h>
#include <mod_scmi_perf.h>

/*!
 * \brief Plugin's implementation of the 'update' API.
 */
int plugin_update(struct perf_plugins_perf_update *data);

/*!
 * \brief Plugin's implementation of the 'report' API.
 */
int plugin_report(struct perf_plugins_perf_report *data);
