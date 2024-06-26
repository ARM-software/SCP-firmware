/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <Mockfwk_module.h>

#include <mod_perf_controller.h>

#include <fwk_element.h>
#include <fwk_module_idx.h>

#define MAX_CORE_PER_CLUSTER 4U

enum test_perf_controller_cluster_idx {
    TEST_BIG_CLUSTER,
    TEST_LITTLE_CLUSTER,
    TEST_CLUSTER_COUNT,
};

struct fwk_element cluster_config[TEST_CLUSTER_COUNT] = {
    [TEST_BIG_CLUSTER] = {
        .sub_element_count = MAX_CORE_PER_CLUSTER,
        .data = &(struct mod_perf_controller_cluster_config) {
            .performance_driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TEST_PERF_CONT_DRVR,TEST_BIG_CLUSTER),
            .performance_driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_TEST_PERF_CONT_DRVR,TEST_BIG_CLUSTER),
            .power_model_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TEST_PERF_MODEL_DRVR,TEST_BIG_CLUSTER),
            .power_model_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_TEST_PERF_MODEL_DRVR,TEST_BIG_CLUSTER),
        },
    },
    [TEST_LITTLE_CLUSTER] = {
        .sub_element_count = 1U,
        .data = &(struct mod_perf_controller_cluster_config) {
            .performance_driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TEST_PERF_CONT_DRVR,TEST_LITTLE_CLUSTER),
            .performance_driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_TEST_PERF_CONT_DRVR,TEST_LITTLE_CLUSTER),
            .power_model_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TEST_PERF_MODEL_DRVR,TEST_LITTLE_CLUSTER),
            .power_model_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_TEST_PERF_MODEL_DRVR,TEST_LITTLE_CLUSTER),
        },
    },
};
