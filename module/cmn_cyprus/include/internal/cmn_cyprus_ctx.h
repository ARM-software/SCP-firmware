/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      CMN Cyprus driver module context structure.
 */

#ifndef INTERNAL_CMN_CYPRUS_CTX_H
#define INTERNAL_CMN_CYPRUS_CTX_H

#include <internal/cmn_cyprus_reg.h>

#include <mod_cmn_cyprus.h>

#include <stdbool.h>

#define MOD_NAME "[CMN_CYPRUS] "

/*!
 * \brief Structure to store the HN-S node info.
 */
struct cmn_cyprus_hns_info {
    /*! Pointer to the HN-S register */
    struct cmn_cyprus_hns_reg *hns;

    /*! Pointer to HN-S node position structure */
    struct cmn_cyprus_node_pos node_pos;

    /*! Pointer to the connected MXP register */
    struct cmn_cyprus_mxp_reg *mxp;
};

/*!
 * \brief CMN Cyprus driver context.
 */
struct cmn_cyprus_ctx {
    /*! CMN Cyprus driver configuration data */
    const struct mod_cmn_cyprus_config *config;

    /*! Base address of the CMN Cyprus configuration register */
    struct cmn_cyprus_cfgm_reg *cfgm;

    /*! Count of HN-S nodes */
    unsigned int hns_count;

    /*! Pointer to the table of HN-S nodes */
    struct cmn_cyprus_hns_info *hns_info_table;

    /*! Count of RNSAM nodes */
    unsigned int rnsam_count;

    /*! List of RNSAM node pointers */
    struct cmn_cyprus_rnsam_reg **rnsam_table;

    /*! Count of RN-D nodes */
    unsigned int rnd_count;

    /*! Count of RN-F nodes */
    unsigned int rnf_count;

    /*! Count of RN-I nodes */
    unsigned int rni_count;

    /*! Count of CCG RA nodes */
    unsigned int ccg_ra_reg_count;

    /*! Count of CCG HA nodes */
    unsigned int ccg_ha_reg_count;

    /*! Count of CCLA nodes */
    unsigned int ccla_reg_count;
};

#endif /* INTERNAL_CMN_CYPRUS_CTX_H */
