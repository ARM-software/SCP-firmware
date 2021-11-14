/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* The use of "subordinate" may not be in sync with platform documentation */

#ifndef MOD_RCAR_CLOCK_H
#define MOD_RCAR_CLOCK_H

#include <rcar_mmap.h>
#include <utils_def.h>

#include <mod_clock.h>

#include <fwk_element.h>

#include <stdint.h>

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * @{
 */

/*!
 * \defgroup GroupRCARClock Clock
 * @{
 */

/*!
 * \brief Clock driver interface.
 */
struct mod_rcar_clock_drv_api {
    /*! Name of the driver */
    const char *name;

    /*!
     * \brief Set a new clock rate by providing a frequency in Hertz (Hz).
     *
     * \param clock_id Clock device identifier.
     *
     * \param rate The desired frequency in Hertz.
     *
     * \param round_mode The type of rounding to perform, if required, to
     *      achieve the given rate.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*set_rate)(
        fwk_id_t clock_id,
        uint64_t rate,
        enum mod_clock_round_mode round_mode);

    /*!
     * \brief Get the current rate of a clock in Hertz (Hz).
     *
     * \param clock_id Clock device identifier.
     *
     * \param [out] rate The current clock rate in Hertz.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_rate)(fwk_id_t clock_id, uint64_t *rate);

    /*!
     * \brief Get a clock rate in Hertz from an index into the clock's range.
     *
     * \param clock_id Clock device identifier.
     *
     * \param rate_index The index into the clock's range to get the rate of.
     *
     * \param[out] rate The rate, in Hertz, corresponding to the index.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_rate_from_index)(
        fwk_id_t clock_id,
        unsigned int rate_index,
        uint64_t *rate);

    /*!
     * \brief Set the running state of a clock.
     *
     * \param clock_id Clock device identifier.
     *
     * \param state One of the valid clock states.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*set_state)(fwk_id_t clock_id, enum mod_clock_state state);

    /*!
     * \brief Get the running state of a clock.
     *
     * \param clock_id Clock device identifier.
     *
     * \param[out] state The current clock state.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_state)(fwk_id_t clock_id, enum mod_clock_state *state);

    /*!
     * \brief Get the range of rates that the clock supports.
     *
     * \param clock_id Clock device identifier.
     *
     * \param[out] range The clock range structure.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_range)(fwk_id_t clock_id, struct mod_clock_range *range);

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
 * \brief APIs provided by the driver.
 */
enum mod_rcar_clock_api_type {
    /*! An implementation of the Clock HAL module's clock driver API */
    MOD_RCAR_CLOCK_API_TYPE_CLOCK,
    /*! A low-level API for direct control of CSS clocks */
    MOD_RCAR_CLOCK_API_TYPE_CSS,
    MOD_RCAR_CLOCK_API_TYPE_SYSTEM,
    MOD_RCAR_CLOCK_API_COUNT,
};

/*!
 * \brief Sub-types of rcar clock.
 */
enum mod_rcar_clock_type {
    /*! A clock with a fixed source. Only its divider can be changed. */
    MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE,
    /*! A clock with multiple, selectable sources and at least one divider. */
    MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
    /*!
     * A clock with multiple, selectable sources, at least one divider, and
     * support for modulation.
     */
    MOD_RCAR_CLOCK_TYPE_CLUSTER,
};

/*!
 * \brief Divider register types.
 */
enum mod_rcar_clock_msclock_divider {
    /*! Divider affecting the A57 PLL clock source. */
    MOD_RCAR_CLOCK_A57_DIVIDER_DIV_EXT = 1,
    /*! Divider affecting the A53 PLL clock source. */
    MOD_RCAR_CLOCK_A53_DIVIDER_DIV_EXT = 2,
};

/*!
 * \brief Selectable clock sources for V8.2 cluster clocks.
 */
enum mod_clusclock_source {
    /*! The clock source is set to a private cluster PLL */
    MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL0 = 0,
    /*! The clock source is set to a private cluster PLL */
    MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL1 = 1,
    /*! The clock source is set to a private cluster PLL */
    MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL2 = 2,
    /*! The clock source is set to a private cluster PLL */
    MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL3 = 3,
    /*! The clock source is set to a private cluster PLL */
    MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL4 = 4,
    /*! The clock source is set to a private cluster PLL */
    MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL5 = 5,
    /*! The clock source is set to a private cluster PLL */
    MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL6 = 6,
    /*! The clock source is set to a private cluster PLL */
    MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL7 = 7,
    /*! Number of valid clock sources */
    MOD_RCAR_CLOCK_CLUSCLK_SOURCE_MAX
};

/*!
 * \brief Rate lookup entry.
 */
struct mod_rcar_clock_rate {
    /*! Rate in Hertz. */
    uint64_t rate;
    /*! Clock source used to obtain the rate (multi-source clocks only). */
    uint8_t source;
    /*! The divider register to use (multi-source clocks only). */
    enum mod_rcar_clock_msclock_divider divider_reg;
    /*! Divider used to obtain the rate. */
    uint8_t divider;
};

/*!
 * \brief Ext clock rate lookup entry.
 */
struct mod_ext_clock_rate {
    /*! Rate in Hertz. */
    uint32_t ext_clk_rate;
};

/*!
 * \brief Subsystem clock device configuration.
 */
struct mod_rcar_clock_dev_config {
    /*! The type of the clock device. */
    enum mod_rcar_clock_type type;

    /*!
     * \brief Indicates whether the clock is part of a CSS clock group (\c true)
     *     or operating as an independent clock (\c false).
     */
    bool is_group_member;

    /*! Pointer to the clock's control register. */
    volatile uint32_t *const control_reg;

    /*! Pointer to the clock's modulator register, if any. */
    volatile uint32_t *const modulator_reg;

    /*! Pointer to the clock's DIV_SYS divider register, if any. */
    volatile uint32_t *const divsys_reg;

    /*! Pointer to the clock's DIV_EXT divider register, if any. */
    volatile uint32_t *const divext_reg;

    /*! Pointer to the clock's rate lookup table. */
    const struct mod_rcar_clock_rate *rate_table;

    /*! The number of rates in the rate lookup table. */
    uint32_t rate_count;

    /*! The rate, in Hz, to set during module initialization. */
    uint64_t initial_rate;

    /*!
     * If \c true, the driver will not attempt to set a default frequency, or to
     * otherwise configure the PLL during the pre-runtime phase. The PLL is
     * expected to be initialized later in response to a notification or other
     * event.
     */
    const bool defer_initialization;
};

/*!
 * @cond
 */

/* Device context */
struct rcar_clock_dev_ctx {
    bool initialized;
    uint64_t current_rate;
    enum mod_clock_state current_state;
    const struct mod_rcar_clock_dev_config *config;
};

/* Module context */
struct rcar_clock_ctx {
    struct rcar_clock_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
    uint32_t extal_clk;
};

struct op_points {
    unsigned long freq; /* Hz */
    unsigned long volt; /* uV */
};

#define PLL_BASE_CLOCK (16640000UL)
#define CPG_FRQCRB (CPG_BASE + 0x0004)
#define CPG_FRQCRB_KICK 0x80000000
#define CPG_FRQCRC (CPG_BASE + 0x00e0)
#define CPG_FRQCRC_ZFC_A57_MASK 0x00001F00
#define CPG_FRQCRC_ZFC_A57_SHIFT 8
#define CPG_FRQCRC_ZFC_A53_MASK 0x0000001F
#define CPG_FRQCRC_ZFC_A53_SHIFT 0
#ifndef CPG_PLL0CR
#    define CPG_PLL0CR (CPG_BASE + 0x00d8)
#endif
#define CPG_PLL0CR_ZFC_MASK 0x7F000000
#define CPG_PLL0CR_ZFC_SHIFT 24
#ifndef CPG_PLL2CR
#    define CPG_PLL2CR (CPG_BASE + 0x002c)
#endif
#define CPG_PLL2CR_ZFC_MASK 0x7F000000
#define CPG_PLL2CR_ZFC_SHIFT 24

#define min(x, y) \
    ({ \
        __typeof__(x) _x = (x); \
        __typeof__(y) _y = (y); \
        _x < _y ? _x : _y; \
    })

#define max(x, y) \
    ({ \
        __typeof__(x) _x = (x); \
        __typeof__(y) _y = (y); \
        _x > _y ? _x : _y; \
    })

#define clamp(val, lo, hi) min((__typeof__(val))max(val, lo), hi)

#define A57_DOMAIN 0
#define A53_DOMAIN 1
#define NR_H3_A57_OPP 5
#define NR_M3_A57_OPP 6
#define NR_H3_A53_OPP 3
#define NR_M3_A53_OPP 4
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define DIV_ROUND(n, d) (((n) + (d) / 2) / (d))

/* CPG base address */
#define CPG_PLLECR 0x00D0

/* Implementation for customized clocks (Z-clk, Z2-clk, PLL0-clk) for CPUFreq */
#define CPG_PLLECR_PLL0ST BIT(8)
#define CPG_PLLECR_PLL2ST BIT(10)

/* Define for PLL0 clk driver */
#define CPG_PLLCR_STC_MASK 0x7f000000
#define CPG_PLLCR_STC_SHIFT 24

/* Modify for Z-clock and Z2-clock
 *
 * Traits of this clock:
 * prepare - clk_prepare only ensures that parents are prepared
 * enable - clk_enable only ensures that parents are enabled
 * rate - rate is adjustable.  clk->rate = parent->rate * mult / 32
 * parent - fixed parent.  No clk_set_parent support
 */
#define CPG_FRQCRC_ZFC_MASK (0x1f << 8)
#define CPG_FRQCRC_ZFC_SHIFT 8
#define CPG_FRQCRC_Z2FC_MASK 0x1f
#define Z_CLK_MAX_THRESHOLD 1500000000U
#define Z2_CLK_MAX_THRESHOLD 1200000000U

extern int32_t rcar_iic_dvfs_receive(
    uint8_t subordinate,
    uint8_t reg,
    uint8_t *data);
extern int32_t rcar_iic_dvfs_send(
    uint8_t subordinate,
    uint8_t regr,
    uint8_t data);

/*!
 * @endcond
 */

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_CLOCK_H */
