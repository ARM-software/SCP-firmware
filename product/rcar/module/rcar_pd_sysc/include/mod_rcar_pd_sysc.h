/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_PD_SYSC_H
#define MOD_RCAR_PD_SYSC_H

#include <config_power_domain.h>

#include <mod_power_domain.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * @{
 */

/*!
 * \defgroup GroupRCARPdSysc SYSC Driver
 * @{
 */

/*!
 * @cond
 */

/* Power domain context */
struct rcar_sysc_pd_ctx {
    /* Power domain configuration data */
    const struct mod_rcar_pd_sysc_config *config;

    /* Identifier of the entity bound to the power domain driver API */
    fwk_id_t bound_id;

    /* Power module driver input API */
    struct mod_pd_driver_input_api *pd_driver_input_api;
    /* Power Domain current state*/
    unsigned int current_state;
};

/* Module context */
struct rcar_sysc_ctx {
    /* Number of pd_sysc domains */
    unsigned int pd_sysc_count;

    /* Table of the power domain contexts */
    struct rcar_sysc_pd_ctx *pd_ctx_table;

    /* Log API */
    struct mod_log_api *log_api;
};

/*!
 * @endcond
 */

/*!
 * \brief Configuration data of a power domain of the SYSC module.
 */
struct mod_rcar_pd_sysc_config {
    /*! Power domain type */
    enum rcar_pd_type pd_type;
    /*! Offset of PWRSR register for this area */
    unsigned int chan_offs;
    /*! Bit in PWR* (except for PWRUP in PWRSR) */
    unsigned char chan_bit;
    /*! Bit in SYSCI*R */
    unsigned char isr_bit;

    /*!
     * Flag indicating if this domain should be powered on during element init.
     */
    bool always_on;
};

/*!
 * \brief API indices
 */
enum mod_rcar_pd_sys_api_type {
    MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC,
    MOD_RCAR_PD_SYSC_API_TYPE_SYSTEM,
    MOD_RCAR_PD_SYSC_API_COUNT,
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_PD_SYSC_H */
