/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System Power Support
 */

#ifndef MOD_SYSTEM_POWER_H
#define MOD_SYSTEM_POWER_H

#include <mod_power_domain.h>

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupSystemPower System Power Support
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

/*! Element configuration */
struct mod_system_power_dev_config {
    /*! Identifier of the system PPU */
    fwk_id_t sys_ppu_id;

    /*! System PPU API identifier */
    fwk_id_t api_id;

    /*!
     * \brief Pointer to a table defining the power states this system PPU will
     *      be set for each system state.
     */
    const uint8_t *sys_state_table;
};

/*! Module configuration */
struct mod_system_power_config {
    /*! SoC wakeup IRQ number */
    unsigned int soc_wakeup_irq;

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

    /*! Initial System Power state after power-on */
    enum mod_pd_state initial_system_power_state;
};

/*! Platform-specific interrupt commands indices */
enum mod_system_power_platform_interrupt_cmd {
    MOD_SYSTEM_POWER_PLATFORM_INTERRUPT_CMD_INIT,
    MOD_SYSTEM_POWER_PLATFORM_INTERRUPT_CMD_ENABLE,
    MOD_SYSTEM_POWER_PLATFORM_INTERRUPT_CMD_DISABLE,
    MOD_SYSTEM_POWER_PLATFORM_INTERRUPT_CMD_CLEAR_PENDING,
    MOD_SYSTEM_POWER_PLATFORM_INTERRUPT_CMD_COUNT,
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

    /*!
     * \brief Pointer to the platform interrupt management function.
     *
     * \details This function allows the driver to manage additional
     *      platform-specific interrupts.
     *
     * \note This function is \b optional.
     *
     * \param isr_cmd Type of command requested.
     *
     * \retval ::FWK_E_PARAM The interrupt command is not valid.
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*platform_interrupts)(enum mod_system_power_platform_interrupt_cmd
                               isr_cmd);
};

/*!
 * \defgroup GroupSystemPowerIds Identifiers
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_system_power_api_idx {
#if BUILD_HAS_MOD_SYSTEM_POWER
    /*! API index for the power domain driver API */
    MOD_SYSTEM_POWER_API_IDX_PD_DRIVER,

    /*! API index for the power domain driver input API */
    MOD_SYSTEM_POWER_API_IDX_PD_DRIVER_INPUT,
#endif

    /*! Number of exposed APIs */
    MOD_SYSTEM_POWER_API_COUNT
};

#if BUILD_HAS_MOD_SYSTEM_POWER
/*! Identifier of the power domain driver API */
static const fwk_id_t mod_system_power_api_id_pd_driver =
    FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_POWER,
                    MOD_SYSTEM_POWER_API_IDX_PD_DRIVER);

/*! Identifier of the power domain driver input API */
static const fwk_id_t mod_system_power_api_id_pd_driver_input =
    FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_POWER,
                    MOD_SYSTEM_POWER_API_IDX_PD_DRIVER_INPUT);
#endif

/*!
 * \}
 */

/*!
 * @}
 */

#endif /* MOD_SYSTEM_POWER_H */
