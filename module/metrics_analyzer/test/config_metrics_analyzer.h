/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fwk_element.h"
#include "fwk_id.h"
#include "fwk_module.h"
#include "fwk_module_idx.h"
#include "mod_metrics_analyzer.h"

/*
 * Providers domain indexes
 */

enum thermal_domain_idx {
    THERMAL_DOMAIN_IDX_GPU,
    THERMAL_DOMAIN_IDX_SOC,
    THERMAL_DOMAIN_IDX_CPU,
    THERMAL_DOMAIN_IDX_COUNT,
};

enum power_capping_domain_idx {
    POWER_CAPPING_DOMAIN_IDX_GPU,
    POWER_CAPPING_DOMAIN_IDX_CPU,
    POWER_CAPPING_DOMAIN_IDX_SOC,
    POWER_CAPPING_DOMAIN_IDX_COUNT,
};

enum hsp_domain_idx {
    HSP_DOMAIN_IDX_SOC,
    HSP_DOMAIN_IDX_GPU,
    HSP_DOMAIN_IDX_CPU,
    HSP_DOMAIN_IDX_COUNT,
};

/*
 * consumer domain indexes
 */
enum consumer_domain_idx {
    CONSUMER_DOMAIN_IDX_SOC,
    CONSUMER_DOMAIN_IDX_GPU,
    CONSUMER_DOMAIN_IDX_CPU,
    CONSUMER_DOMAIN_IDX_COUNT,
};

/* Metrics domain indexes */
enum metrics_domain_idx {
    METRICS_ANALYZER_DOMAIN_IDX_SOC,
    METRICS_ANALYZER_DOMAIN_IDX_GPU,
    METRICS_ANALYZER_DOMAIN_IDX_CPU,
    METRICS_ANALYZER_DOMAIN_IDX_COUNT,
};

enum metric_idx {
    METRICS_ANALYZER_METRIC_IDX_THERMAL,
    METRICS_ANALYZER_METRIC_IDX_POWER_CAPPING,
    METRICS_ANALYZER_METRIC_IDX_HSP,
    METRICS_ANALYZER_METRIC_IDX_COUNT,
};

#define ID(module, domain) \
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_##module, module##_DOMAIN_IDX_##domain)

/* Assume that all providers has their get_limit API index equal to zero */
#define PROVIDER_API_ID(module) FWK_ID_API_INIT(FWK_MODULE_IDX_##module, 0)

/* Assume that the consumer has set_limit API index equal to zero */
#define CONSUMER_API_ID FWK_ID_API_INIT(FWK_MODULE_IDX_CONSUMER, 0)

#define PROVIDER_ENTRY(name, domain) \
    { \
        .api_id = PROVIDER_API_ID(name), .domain_id = ID(name, domain), \
    }

#define CONSUMER_ENTRY(domain) \
    { \
        .api_id = CONSUMER_API_ID, .domain_id = ID(CONSUMER, domain), \
    }

const struct mod_metrics_analyzer_interactor soc_limits_providers[] = {
    {
        .api_id = PROVIDER_API_ID(THERMAL),
        .domain_id = ID(THERMAL, SOC),
    },
    {
        .api_id = PROVIDER_API_ID(POWER_CAPPING),
        .domain_id = ID(POWER_CAPPING, SOC),
    },
};

const struct mod_metrics_analyzer_interactor cpu_limits_providers[] = {
    {
        .api_id = PROVIDER_API_ID(THERMAL),
        .domain_id = ID(THERMAL, CPU),
    },
    {
        .api_id = PROVIDER_API_ID(POWER_CAPPING),
        .domain_id = ID(POWER_CAPPING, CPU),
    },
    {
        .api_id = PROVIDER_API_ID(HSP),
        .domain_id = ID(HSP, SOC),
    }
};

const struct mod_metrics_analyzer_interactor gpu_limits_providers[] = {
    {
        .api_id = PROVIDER_API_ID(THERMAL),
        .domain_id = ID(THERMAL, GPU),
    },
    {
        .api_id = PROVIDER_API_ID(POWER_CAPPING),
        .domain_id = ID(POWER_CAPPING, GPU),
    },
};

static const struct fwk_element metrics_analyzer_domain[] = {
    [METRICS_ANALYZER_DOMAIN_IDX_SOC] = {
        .name = "SOC",
        .sub_element_count = FWK_ARRAY_SIZE(soc_limits_providers),
        .data = &(const struct mod_metrics_analyzer_domain_config) {
            .limit_providers = soc_limits_providers,
            .limit_consumer = CONSUMER_ENTRY(SOC),
        },
    },
    [METRICS_ANALYZER_DOMAIN_IDX_CPU] = {
        .name = "CPU",
        .sub_element_count = FWK_ARRAY_SIZE(cpu_limits_providers),
        .data = &(const struct mod_metrics_analyzer_domain_config) {
            .limit_providers = cpu_limits_providers,
            .limit_consumer = CONSUMER_ENTRY(CPU),
        },
    },
    [METRICS_ANALYZER_DOMAIN_IDX_GPU] = {
        .name = "GPU",
        .sub_element_count = FWK_ARRAY_SIZE(gpu_limits_providers),
        .data = &(const struct mod_metrics_analyzer_domain_config) {
            .limit_providers = gpu_limits_providers,
            .limit_consumer = CONSUMER_ENTRY(GPU),
        },
    },
};

struct fwk_module_config module_config = {
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(metrics_analyzer_domain),
};
