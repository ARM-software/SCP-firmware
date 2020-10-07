/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      CMN700 Context structure Interface
 */

#ifndef INTERNAL_CMN700_CTX_H
#define INTERNAL_CMN700_CTX_H

#include <cmn700.h>

#include <mod_cmn700.h>

#include <stdbool.h>
#include <stdint.h>

struct cmn700_device_ctx {
    const struct mod_cmn700_config *config;

    struct cmn700_cfgm_reg *root;

    /* Number of HN-F (system cache) nodes in the system */
    unsigned int hnf_count;

    /* Pointer to list of HN-F nodes for use in CCIX programming */
    uintptr_t *hnf_node;

    uint64_t *hnf_cache_group;
    uint64_t *sn_nodeid_group;

    /*
     * External RN-SAMs. The driver keeps a list of tuples (node identifier and
     * node pointers). The configuration of these nodes is via the SAM API.
     */
    unsigned int external_rnsam_count;
    struct external_rnsam_tuple *external_rnsam_table;

    /*
     * Internal RN-SAMs. The driver keeps a list of RN-SAM pointers to
     * configure them once the system has been fully discovered and all
     * parameters are known
     */
    unsigned int internal_rnsam_count;
    struct cmn700_rnsam_reg **internal_rnsam_table;

    /* Count of RN Nodes for the use in CCIX programming */
    unsigned int rnd_count;
    unsigned int rnf_count;
    unsigned int rni_count;

    /* Timer module API */
    struct mod_timer_api *timer_api;

    bool initialized;
};

#endif /* INTERNAL_CMN700_CTX_H */
