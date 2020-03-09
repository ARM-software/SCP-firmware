/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_CSS_CLOCK_H
#define MOD_CSS_CLOCK_H

#include <fwk_element.h>
#include <fwk_id.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupCSSClock CSS Clock Driver
 *
 * \details A driver for Arm Compute Sub-System clock devices.
 *
 * @{
 */

/*!
 * \brief Type of the clock (indexed vs non-indexed).
 */
enum mod_css_clock_type {
    /*! A lookup table is used to find the settings for a given rate. */
    MOD_CSS_CLOCK_TYPE_INDEXED,
    /*! The clock rate is set without looking up settings in a table. */
    MOD_CSS_CLOCK_TYPE_NON_INDEXED,
};

/*!
 * \brief APIs that the module makes available to entities requesting binding.
 */
enum mod_css_clock_api_types {
    /*! Clock HAL */
    MOD_CSS_CLOCK_API_TYPE_CLOCK,
    MOD_CSS_CLOCK_API_COUNT,
};

/*!
 * \brief Rate lookup table entry.
 */
struct mod_css_clock_rate {
    /*! Rate of the clock in Hertz. */
    uint64_t rate;

    /*! Rate of the source PLL in Hertz. */
    uint64_t pll_rate;

    /*! The clock source used to attain the rate. */
    uint8_t clock_source;

    /*! The clock divider to program. */
    uint8_t clock_div_type;

    /*! The clock divider used to attain the rate. */
    uint32_t clock_div;

    /*! The clock modulator numerator setting, if implemented. */
    uint32_t clock_mod_numerator;

    /*! The clock modulator denominator setting, if implemented. */
    uint32_t clock_mod_denominator;
};

/*!
 * \brief Subsystem clock device configuration.
 */
struct mod_css_clock_dev_config {
    /*! The type of the clock (indexed vs non-indexed). */
    enum mod_css_clock_type clock_type;

    /*! Clock source used for non-indexed clocks. */
    uint8_t clock_default_source;

    /*! Clock source used when changing the PLL rate. */
    uint8_t clock_switching_source;

    /*! Element identifier for the associated PLL. */
    fwk_id_t pll_id;

    /*! Reference to the API provided by the PLL driver. */
    fwk_id_t pll_api_id;

    /*! Pointer to the table of clocks that are members of the group. */
    fwk_id_t const *member_table;

    /*! The number of clocks in the member table. */
    uint32_t member_count;

    /*! Reference to the API for the clocks that are members of the group. */
    fwk_id_t member_api_id;

    /*!
     * The rate, in Hz, that the member clocks run at before any configuration.
     */
    uint64_t initial_rate;

    /*! The clock's support for modulation. */
    bool modulation_supported;

    /*! Pointer to the clock's rate lookup table. */
    struct mod_css_clock_rate const *rate_table;

    /*! The number of rates in the rate lookup table. */
    uint32_t rate_count;
};

/*!
 * \brief CSS clock control interface.
 */
struct mod_css_clock_direct_api {
    /*! Set the clock device's divider */
    int (*set_div)(fwk_id_t device_id, uint32_t divider_type,
                   uint32_t divider);

    /*! Set the clock device's source (multi-source clocks only) */
    int (*set_source)(fwk_id_t device_id, uint8_t source);

    /*! Set the clock device's modulator (multi-source clocks only) */
    int (*set_mod)(fwk_id_t device_id, uint32_t numerator,
                   uint32_t denominator);

    /*!
     * \brief Handle the condition where the state of a clock's power domain is
     *     about to change.
     *
     * \details This function will be called prior to the change in power
     *     state occurring so that the clock driver implementing this API is
     *     able to perform any required preparatory work beforehand.
     *
     * \note This function is optional. If the driver does not control any
     *     clocks that require power state awareness then the pointer may be set
     *     to NULL.
     *
     * \param clock_id Clock device identifier.
     *
     * \param current_state The current power state that the clock's power
     *     domain will transition away from.
     *
     * \param new_state The power state that the clock's power domain will
     *     transition to.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*process_pending_power_transition)(
        fwk_id_t clock_id,
        unsigned int current_state,
        unsigned int new_state);

    /*!
     * \brief Handle the condition where the state of a clock's power domain
     *     has changed.
     *
     * \details This function will be called after the change in power state
     *     has occurred. The driver can take any appropriate actions that are
     *     required to accommodate the new state. The transition can be to a
     *     deeper power state (e.g. ON->OFF) or to a shallower power state
     *     (e.g. OFF->ON).
     *
     * \note This function is optional. If the driver does not control any
     *     clocks that require power state awareness then the pointer may be set
     *     to NULL.
     *
     * \param clock_id Clock device identifier.
     *
     * \param state The power state that the clock's power domain transitioned
     *     to.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*process_power_transition)(fwk_id_t clock_id, unsigned int state);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_CSS_CLOCK_H */
