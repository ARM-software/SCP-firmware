/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      CMN600 Context structure Interface
 */

#ifndef INTERNAL_CMN600_CTX_H
#define INTERNAL_CMN600_CTX_H

#include <cmn600.h>

#include <mod_cmn600.h>
#include <mod_timer.h>

#include <stdbool.h>
#include <stdint.h>

/* External nodes that require RN-SAM mapping during run-time */
struct external_rnsam_tuple {
    unsigned int node_id;
    struct cmn600_rnsam_reg *node;
};

/* Max Node Counts */
#define MAX_HNF_COUNT 32
#define MAX_RND_COUNT 32
#define MAX_RNF_COUNT 32
#define MAX_RNI_COUNT 32

struct cmn600_ctx {
    const struct mod_cmn600_config *config;

    struct cmn600_cfgm_reg *root;

    /* Number of HN-F (system cache) nodes in the system */
    unsigned int hnf_count;
    uint32_t hnf_offset[MAX_HNF_COUNT];
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
    struct cmn600_rnsam_reg **internal_rnsam_table;

    /*
     * RN-D nodes. The driver keeps a list of RN-D pointers to
     * configure them once the system has been fully discovered and all
     * parameters are known
     */
    unsigned int rnd_count;
    uint8_t rnd_ldid[MAX_RND_COUNT];

    /*
     * RN-F nodes. The driver keeps track of the total number of the RN-F nodes.
     */
    unsigned int rnf_count;

    /*
     * RN-I nodes. The driver keeps a list of RN-I pointers to
     * configure them once the system has been fully discovered and all
     * parameters are known
     */
    unsigned int rni_count;
    uint8_t rni_ldid[MAX_RNI_COUNT];

   /* CCIX specific registers */
    unsigned int cxg_ha_id;
    unsigned int cxg_ha_node_id;
    unsigned int cxg_ha_id_remote;
    uint8_t raid_value;
    struct cmn600_cxg_ra_reg *cxg_ra_reg;
    struct cmn600_cxg_ha_reg *cxg_ha_reg;
    struct cmn600_cxla_reg *cxla_reg;

    /* CCIX host parameters to be sent to upper level firmware */
    struct mod_cmn600_ccix_host_node_config ccix_host_info;

    /* Timer module API */
    struct mod_timer_api *timer_api;

    /* Chip information */
    const struct mod_system_info *system_info;

    bool initialized;

    /* Chip ID value */
    uint8_t chip_id;
};

int cmn600_setup_sam(struct cmn600_rnsam_reg *rnsam);

#endif /* INTERNAL_CMN600_CTX_H */
