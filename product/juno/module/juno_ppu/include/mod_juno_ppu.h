/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_JUNO_PPU_H
#define MOD_JUNO_PPU_H

#include <mod_power_domain.h>

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \addtogroup GroupJunoModule Juno Product Modules
 * \{
 */

/*!
 * \defgroup GroupJunoPPU Juno PPU
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_juno_ppu_api_idx {
    /*!
     * \brief Power domain driver API index.
     */
    MOD_JUNO_PPU_API_IDX_PD,

    /*!
     * \brief Juno ROM API index.
     *
     * \details This API is used exclusively in the ROM firmware.
     */
    MOD_JUNO_PPU_API_IDX_ROM,

    /*!
     * \brief Number of defined APIs.
     */
    MOD_JUNO_PPU_API_COUNT,
};

/*!
 * \brief Power Domain driver API identifier.
 */
static const fwk_id_t mod_juno_ppu_api_id_pd =
    FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PPU, MOD_JUNO_PPU_API_IDX_PD);

/*!
 * \brief Juno ROM API identifier.
 */
static const fwk_id_t mod_juno_ppu_api_id_rom =
    FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PPU, MOD_JUNO_PPU_API_IDX_ROM);

/*!
 * \brief Module configuration.
 */
struct mod_juno_ppu_config {
    /*!
     * \brief Identifier of the timer alarm.
     *
     * \details Used for polling a core PPU state during system suspend.
     */
    fwk_id_t timer_alarm_id;
};

/*!
 * \brief Element configuration.
 *
 * \details This is the configuration struct for an individal PPU.
 */
struct mod_juno_ppu_element_config {
    /*!
     * \brief Base address for the PPU registers.
     */
    uintptr_t reg_base;

    /*!
     * \brief Identifier of the timer.
     *
     * \details Used for time-out when operating the PPU.
     */
    fwk_id_t timer_id;

    /*!
     * \brief Power domain type.
     */
    enum mod_pd_type pd_type;

    /*!
     * \brief Wakeup IRQ number.
     */
    unsigned int wakeup_irq;

    /*!
     * \brief Wakeup FIQ number.
     */
    unsigned int wakeup_fiq;

    /*!
     * \brief Warm reset request IRQ number.
     */
    unsigned int warm_reset_irq;
};

/*!
 * \brief ROM-specific API.
 */
struct mod_juno_ppu_rom_api {
    /*!
     * \brief Set the state of a PPU and wait until it has been accepted.
     *
     * \details This function will wait until the requested state becomes the
     *     current policy of the PPU or until a time-out expires.
     *
     * \param ppu_id Element identifier of the PPU.
     * \param state The new state for the PPU (defined in the Power Domain
     *     module).
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_PARAM An invalid state was given.
     * \retval FWK_E_DEVICE The PPU did not accept the new state.
     * \return One of the standard framework error codes.
     */
    int (*set_state_and_wait)(fwk_id_t ppu_id, unsigned int state);

    /*!
     * \brief Get the current policy of a PPU.
     *
     * \param ppu_id Element identifier of the PPU.
     * \param[out] state Policy.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_PARAM The \p state parameter was \c NULL.
     * \retval FWK_E_DEVICE Device error.
     * \return One of the standard framework error codes.
     */
    int (*get_state)(fwk_id_t ppu_id, unsigned int *state);
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_JUNO_PPU_H */
