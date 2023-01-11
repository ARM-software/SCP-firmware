/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
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

    /* CCG specific members */

    /* Node count of CCG_RA, CCG_HA, CCLA nodes each. */
    size_t ccg_node_count;

    /*
     * CCG_RA register and node_id pairs. The driver keeps a list of tuples of
     * pointers to the CCG_RA registers.
     */
    struct ccg_ra_reg_tuple *ccg_ra_reg_table;

    /*
     * CCG_HA register and node_id pairs. The driver keeps a list of tuples of
     * pointers to the CCG_HA registers.
     */
    struct ccg_ha_reg_tuple *ccg_ha_reg_table;

    /*
     * CCLA register and node_id pairs. The driver keeps a list of tuples of
     * pointers to the CXLA registers.
     */
    struct ccla_reg_tuple *ccla_reg_table;

    /*
     * remote_rnf_ldid_value keeps track of the ldid of the remote RNF agents
     * which are to be programmed on the HNF's RN_PHYS_ID registers.
     */
    unsigned int remote_rnf_ldid_value;

    /* Timer module API */
    struct mod_timer_api *timer_api;

    bool initialized;

    /* Count of IO regions programmed in SAM table */
    unsigned int region_io_count;

    /* Count of system cache regions programmed in SAM table */
    unsigned int region_sys_count;

    /* Flags to indicate SCG region init status. */
    unsigned int scg_regions_enabled[MAX_SCG_COUNT];
};

#endif /* INTERNAL_CMN700_CTX_H */
