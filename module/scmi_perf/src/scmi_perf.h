/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SCMI_PERF_H
#define SCMI_PERF_H

/*!
 * \addtogroup GroupSCMI_PERF SCMI Performance Domain Management Protocol
 * @{
 */

int perf_set_limits(
    fwk_id_t domain_id,
    unsigned int agent_id,
    const struct mod_scmi_perf_level_limits *limits);

/*!
 * @}
 */

#endif /* SCMI_PERF_H */
