/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_PD_PMIC_BD9571H
#define MOD_RCAR_PD_PMIC_BD9571H

#include <config_power_domain.h>

#include <mod_power_domain.h>
#include <mod_rcar_pmic.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * @{
 */

/*!
 * \defgroup GroupRCARPDPMIC PD PMIC
 * @{
 */

/*!
 * \brief APIs provided by the driver.
 */
enum mod_rcar_pd_pmic_api_type {
    MOD_RCAR_PD_PMIC_API_TYPE_PUBLIC,
    MOD_RCAR_PD_PMIC_API_TYPE_SYSTEM,
    MOD_RCAR_PD_PMIC_API_COUNT,
};

/*!
 * \brief Configuration data of a power domain of the PMIC module.
 */
struct mod_rcar_pd_pmic_config {
    /*! Power domain type */
    enum rcar_pd_type pd_type;

    /*! Power supply identifier. */
    fwk_id_t psu_id;

    /*!
     * Flag indicating if this domain should be powered on during element init.
     */
    bool always_on;
};

/*!
 * @cond
 */

/* Power domain context */
struct rcar_pmic_pd_ctx {
    /* Power domain configuration data */
    const struct mod_rcar_pd_pmic_config *config;
    /* Power module driver input API */
    struct mod_rcar_pmic_device_api *api;
    /* Power Domain current state*/
    unsigned int current_state;
    /* Identifier of the entity bound to the power domain driver API */
    fwk_id_t bound_id;
    /* Power module driver input API */
    struct mod_pd_driver_input_api *pd_driver_input_api;
};

/* Module context */
struct rcar_pmic_ctx {
    /* Table of the power domain contexts */
    struct rcar_pmic_pd_ctx *pd_ctx_table;

    /* Log API */
    struct mod_log_api *log_api;
};

/*!
 * @endcond
 */

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_PD_PMIC_BD9571H */
