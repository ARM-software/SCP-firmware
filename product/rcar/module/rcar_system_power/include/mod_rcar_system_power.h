/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_SYSTEM_POWER_H
#define MOD_RCAR_SYSTEM_POWER_H

#include <mod_power_domain.h>

#include <fwk_id.h>

/*!
 * \ingroup GroupRCARModule RCAR Product Modules
 * \defgroup GroupRCARSystemPower System Power Support
 *
 * @{
 */

/*! Additional system_power power states */
enum mod_system_power_power_states {
    MOD_SYSTEM_POWER_POWER_STATE_SLEEP0 = MOD_PD_STATE_COUNT,
    MOD_SYSTEM_POWER_POWER_STATE_SLEEP1,
    MOD_SYSTEM_POWER_POWER_STATE_COUNT
};

/*! Extended PPU configuration */
struct mod_system_power_ext_ppu_config {
    /*! PPU identifier */
    fwk_id_t ppu_id;

    /*! API identifier */
    fwk_id_t api_id;
};

/*! Module configuration */
struct mod_system_power_config {
    /*! SoC wakeup IRQ number */
    unsigned int soc_wakeup_irq;

    /*! System 0 PPU element ID */
    fwk_id_t ppu_sys0_id;

    /*! System 1 PPU element ID */
    fwk_id_t ppu_sys1_id;

    /*! System PPUs API ID */
    fwk_id_t ppu_sys_api_id;

    /*! Number of extended PPUs */
    size_t ext_ppus_count;

    /*!
     * \brief Pointer to array of extended PPU configurations.
     *
     * \details These PPUs will be powered on automatically with the rest of the
     *      system.
     */
    const struct mod_system_power_ext_ppu_config *ext_ppus;

    /*! System shutdown driver identifier */
    fwk_id_t driver_id;

    /*! System shutdown driver API identifier */
    fwk_id_t driver_api_id;
};

/*!
 * \brief Driver interface.
 */
struct mod_system_power_driver_api {
    /*!
     * \brief Pointer to the system shutdown function.
     *
     * \note This function is \b mandatory. In case of a successful call the
     *      function does not return.
     *
     * \param system_shutdown Type of system shutdown.
     *
     * \retval One of the driver-defined error code.
     */
    int (*system_shutdown)(enum mod_pd_system_shutdown system_shutdown);
};

/*!
 * \defgroup GroupSystemPowerIds Identifiers
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_system_power_api_idx {
    /*! API index for the power domain driver API */
    MOD_SYSTEM_POWER_API_IDX_PD_DRIVER,

    /*! API index for the power domain driver input API */
    MOD_SYSTEM_POWER_API_IDX_PD_DRIVER_INPUT,

    /*! Number of exposed APIs */
    MOD_SYSTEM_POWER_API_COUNT
};

/*! Identifier of the power domain driver API */
static const fwk_id_t mod_system_power_api_id_pd_driver = FWK_ID_API_INIT(
    FWK_MODULE_IDX_SYSTEM_POWER,
    MOD_SYSTEM_POWER_API_IDX_PD_DRIVER);

/*! Identifier of the power domain driver input API */
static const fwk_id_t mod_system_power_api_id_pd_driver_input = FWK_ID_API_INIT(
    FWK_MODULE_IDX_SYSTEM_POWER,
    MOD_SYSTEM_POWER_API_IDX_PD_DRIVER_INPUT);

/*!
 * \}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_SYSTEM_POWER_H */
