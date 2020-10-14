/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      CMN650 Context structure Interface
 */

#ifndef INTERNAL_CMN650_CTX_H
#define INTERNAL_CMN650_CTX_H

#include <cmn650.h>

#include <mod_cmn650.h>

#include <stdbool.h>
#include <stdint.h>

struct cmn650_device_ctx {
    const struct mod_cmn650_config *config;

    struct cmn650_cfgm_reg *root;

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
    struct cmn650_rnsam_reg **internal_rnsam_table;

    /* Count of RN Nodes for the use in CCIX programming */
    unsigned int rnd_count;
    unsigned int rnf_count;
    unsigned int rni_count;

    /* CCIX specific members */

    /* Node count of CXG_RA, CXG_HA, CXLA nodes each. */
    size_t ccix_node_count;

    /*
     * CXG_RA register and node_id pairs. The driver keeps a list of tuples of
     * pointers to the CXG_RA registers.
     */
    struct cxg_ra_reg_tuple *cxg_ra_reg_table;

    /*
     * CXG_HA register and node_id pairs. The driver keeps a list of tuples of
     * pointers to the CXG_HA registers.
     */
    struct cxg_ha_reg_tuple *cxg_ha_reg_table;

    /*
     * CXLA register and node_id pairs. The driver keeps a list of tuples of
     * pointers to the CXLA registers.
     */
    struct cxla_reg_tuple *cxla_reg_table;

    /*
     * remote_rnf_ldid_value keeps track of the ldid of the remote RNF agents
     * which are to be programmed on the HNF's RN_PHYS_ID registers.
     */
    unsigned int remote_rnf_ldid_value;

    /* Timer module API */
    struct mod_timer_api *timer_api;

    bool initialized;
};

#endif /* INTERNAL_CMN650_CTX_H */
