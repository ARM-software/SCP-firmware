/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
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

/* Maximum number of SCG regions supported by the driver */
#define MAX_SCG_COUNT 8

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

    /*! Node Identifier */
    unsigned int node_id;

    /*! SCG that this HN-F node belongs to */
    uint8_t scg_idx;
};

/*! CCG Request Agent (CCG RA) register and its node id */
struct ccg_ra_info {
    unsigned int node_id;
    struct cmn_cyprus_ccg_ra_reg *ccg_ra;
};

/*! CCG Home Agent (CCG HA) register and its node id */
struct ccg_ha_info {
    unsigned int node_id;
    struct cmn_cyprus_ccg_ha_reg *ccg_ha;
};

/*! CCG Link Agent (CCLA) register and its node id */
struct ccla_info {
    unsigned int node_id;
    struct cmn_cyprus_ccla_reg *ccla;
};

/*!
 * \brief CMN Cyprus driver context.
 */
struct cmn_cyprus_ctx {
    /*! Driver configuration table */
    const struct mod_cmn_cyprus_config_table *config_table;

    /*! Chip specific CMN configuration data */
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

    /*! Count of the I/O memory regions mapped in the RNSAM */
    unsigned int io_region_count;

    /*! Count of the syscache memory regions(SCG) mapped in the RNSAM */
    unsigned int scg_count;

    /*! Count of HN-S nodes per SCG */
    uint8_t scg_hns_count[MAX_SCG_COUNT];

    /* Count of isolated HN-S nodes */
    unsigned int isolated_hns_count;

    /*! CCG RA info table */
    struct ccg_ra_info *ccg_ra_info_table;

    /*! CCG HA info table */
    struct ccg_ha_info *ccg_ha_info_table;

    /*! CCLA info table */
    struct ccla_info *ccla_info_table;

    /*! System Info module API */
    struct mod_system_info_get_info_api *system_info_api;

    /*! Multichip mode */
    bool multichip_mode;

    /*! Chip identifier */
    uint8_t chip_id;
};

#endif /* INTERNAL_CMN_CYPRUS_CTX_H */
