/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_n1sdp_remote_pd.h>
#include <mod_power_domain.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

static const struct fwk_element remote_pd_element_table[] = {
    [0] = {
        .name = "SLV-CLUS0CORE0",
        .data = &((struct mod_n1sdp_remote_pd_config) {
            .pd_type = MOD_PD_TYPE_CORE,
        }),
    },
    [1] = {
        .name = "SLV-CLUS0CORE1",
        .data = &((struct mod_n1sdp_remote_pd_config) {
            .pd_type = MOD_PD_TYPE_CORE,
        }),
    },
    [2] = {
        .name = "SLV-CLUS1CORE0",
        .data = &((struct mod_n1sdp_remote_pd_config) {
            .pd_type = MOD_PD_TYPE_CORE,
        }),
    },
    [3] = {
        .name = "SLV-CLUS1CORE1",
        .data = &((struct mod_n1sdp_remote_pd_config) {
            .pd_type = MOD_PD_TYPE_CORE,
        }),
    },
    [4] = {
        .name = "SLV-CLUS0",
        .data = &((struct mod_n1sdp_remote_pd_config) {
            .pd_type = MOD_PD_TYPE_CLUSTER,
        }),
    },
    [5] = {
        .name = "SLV-CLUS1",
        .data = &((struct mod_n1sdp_remote_pd_config) {
            .pd_type = MOD_PD_TYPE_CLUSTER,
        }),
    },
    [6] = {
        .name = "SLV-DBGTOP",
        .data = &((struct mod_n1sdp_remote_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
        }),
    },
    [7] = {
        .name = "SLV-SYSTOP",
        .data = &((struct mod_n1sdp_remote_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
        }),
    },
    [8] = {
        .name = "SYSTOP-LOGICAL",
        .data = &((struct mod_n1sdp_remote_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
        }),
    },
    [9] = { 0 },
};

static const struct fwk_element *remote_pd_get_element_table(fwk_id_t id)
{
    return remote_pd_element_table;
}

const struct fwk_module_config config_n1sdp_remote_pd = {
    .get_element_table = remote_pd_get_element_table,
};
