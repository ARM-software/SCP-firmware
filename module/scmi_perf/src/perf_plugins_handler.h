/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Performance plugins handler.
 *     Please refer to the architecture documentation for further details.
 */

#ifndef PERF_PLUGINS_HANDLER_H
#define PERF_PLUGINS_HANDLER_H

/*!
 * \addtogroup GroupSCMI_PERF SCMI Performance Domain Management Protocol
 * @{
 */

#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_id.h>

int perf_plugins_handler_bind(void);

int perf_plugins_handler_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api);

/*!
 * \brief FastChannel Performance Update data container
 *
 * \note This data structure is passed between the plugins handler and the SCMI
 *      performance module. It contains:
 *      - limits that SCMI-Perf provides to the plugins handler for its logic.
 *      - adjusted limits that the plugins handler returns to SCMI-Perf ready
 *          for the DVFS module.
 *      These values come from the FastChannels and the plugins handler executes
 *      its logic in order to aggregate them before they can be forwarded to the
 *      DVFS module.
 */
struct fc_perf_update {
    /*! DVFS domain identifier */
    fwk_id_t domain_id;

    /*! Performance level */
    uint32_t level;

    /*! Performance limit max to the plugins handler */
    uint32_t max_limit;
    /*! Performance limit min to the plugins handler */
    uint32_t min_limit;

    /*! Adjusted (by the plugins hanlder) performance limit max */
    uint32_t adj_max_limit;
    /*! Adjusted (by the plugins hanlder) performance limit min */
    uint32_t adj_min_limit;
};

void perf_plugins_handler_update(struct fc_perf_update *freq_update);
void perf_plugins_handler_report(struct perf_plugins_perf_report *data);

fwk_id_t perf_plugins_get_dependency_id(unsigned int dom_idx);

int perf_plugins_handler_init(const struct mod_scmi_perf_config *config);

/*!
 * @}
 */

#endif /* PERF_PLUGINS_HANDLER_H */
