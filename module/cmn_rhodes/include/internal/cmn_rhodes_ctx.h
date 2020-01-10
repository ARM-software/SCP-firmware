/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      CMN_RHODES Context structure Interface
 */

#ifndef INTERNAL_CMN_RHODES_CTX_H
#define INTERNAL_CMN_RHODES_CTX_H

#include <cmn_rhodes.h>

#include <mod_cmn_rhodes.h>

#include <stdbool.h>
#include <stdint.h>

static struct cmn_rhodes_ctx {
    const struct mod_cmn_rhodes_config *config;

    struct cmn_rhodes_cfgm_reg *root;

    /* Number of HN-F (system cache) nodes in the system */
    unsigned int hnf_count;
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
    struct cmn_rhodes_rnsam_reg **internal_rnsam_table;

    bool initialized;
} *ctx;


#endif /* INTERNAL_CMN600_CTX_H */
