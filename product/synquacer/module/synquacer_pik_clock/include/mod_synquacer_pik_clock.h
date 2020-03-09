/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SYNQUACER_PIK_CLOCK_H
#define MOD_SYNQUACER_PIK_CLOCK_H

#include <fwk_element.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupSYNQUACERModule SYNQUACER Product Modules
 * @{
 */

/*!
 * \defgroup GroupSYNQUACER_PIKClock PIK Clock Driver
 *
 * \details A driver for clock devices that are part of a PIK.
 *
 * @{
 */

/*!
 * \brief APIs provided by the driver.
 */
enum mod_pik_clock_api_type {
    /*! An implementation of the Clock HAL module's clock driver API */
    MOD_PIK_CLOCK_API_TYPE_CLOCK,
    /*! A low-level API for direct control of CSS clocks */
    MOD_PIK_CLOCK_API_TYPE_CSS,
    MOD_PIK_CLOCK_API_COUNT,
};

/*!
 * \brief Sub-types of PIK clock.
 */
enum mod_pik_clock_type {
    /*! A clock with a fixed source. Only its divider can be changed. */
    MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
    /*! A clock with multiple, selectable sources and at least one divider. */
    MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
    /*!
     * A clock with multiple, selectable sources, at least one divider, and
     * support for modulation.
     */
    MOD_PIK_CLOCK_TYPE_CLUSTER,
};

/*!
 * \brief Divider register types.
 */
enum mod_pik_clock_msclock_divider {
    /*! Divider affecting the system PLL clock source. */
    MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
    /*! Divider affecting the private or external PLL clock sources. */
    MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
};

/*!
 * \brief Selectable clock sources for multi-source clocks.
 */
enum mod_pik_clock_msclock_source {
    /*! The clock is gated */
    MOD_PIK_CLOCK_MSCLOCK_SOURCE_GATED = 0,
    /*! The clock source is set to the system reference clock */
    MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSREFCLK = 1,
    /*! The clock source is set to the system PLL clock */
    MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK = 2,
    /*! The clock source is set to a private PLL */
    MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK = 4,
    /*! Number of valid clock sources */
    MOD_PIK_CLOCK_MSCLOCK_SOURCE_MAX
};

/*!
 * \brief Selectable clock sources for V8.2 cluster clocks.
 */
enum mod_clusclock_source {
    /*! The clock is gated */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_GATED = 0,
    /*! The clock source is set to the system reference clock */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK = 1,
    /*! The clock source is set to a private cluster PLL */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0 = 2,
    /*! The clock source is set to a private cluster PLL */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL1 = 3,
    /*! The clock source is set to a private cluster PLL */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL2 = 4,
    /*! The clock source is set to a private cluster PLL */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL3 = 5,
    /*! The clock source is set to a private cluster PLL */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL4 = 6,
    /*! The clock source is set to a private cluster PLL */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL5 = 7,
    /*! The clock source is set to a private cluster PLL */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL6 = 8,
    /*! The clock source is set to a private cluster PLL */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL7 = 9,
    /*! The clock source is set to the system PLL clock */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSPLLCLK = 10,
    /*! Number of valid clock sources */
    MOD_PIK_CLOCK_CLUSCLK_SOURCE_MAX
};

/*!
 * \brief Selectable clock sources for GPU clocks.
 */
enum mod_pik_clock_gpuclock_source {
    /*! The clock is gated */
    MOD_PIK_CLOCK_GPUCLK_SOURCE_GATED = 0,
    /*! The clock source is set to the system reference clock */
    MOD_PIK_CLOCK_GPUCLK_SOURCE_SYSREFCLK = 1,
    /*! The clock source is set to the system PLL clock */
    MOD_PIK_CLOCK_GPUCLK_SOURCE_SYSPLLCLK = 2,
    /*! The clock source is set to the dedicated GPU PLL */
    MOD_PIK_CLOCK_GPUCLK_SOURCE_GPUPLLCLK = 4,
    /*! Number of valid clock sources */
    MOD_PIK_CLOCK_GPUCLK_SOURCE_MAX
};

/*!
 * \brief Selectable clock sources for DPU scaler clocks.
 */
enum mod_pik_clock_dpuclock_source {
    /*! The clock is gated */
    MOD_PIK_CLOCK_DPUCLK_SOURCE_GATED = 0,
    /*! The clock source is set to the system reference clock */
    MOD_PIK_CLOCK_DPUCLK_SOURCE_SYSREFCLK = 1,
    /*! The clock source is set to the dedicated display PLL */
    MOD_PIK_CLOCK_DPUCLK_SOURCE_DISPLAYPLLCLK = 2,
    /*! The clock source is set to a pixel clock PLL */
    MOD_PIK_CLOCK_DPUCLK_SOURCE_PIXELCLK = 4,
    /*! Number of valid clock sources */
    MOD_PIK_CLOCK_DPUCLK_SOURCE_MAX
};

/*!
 * \brief Selectable clock sources for the DPU AXI clock.
 */
enum mod_pik_clock_aclkdpu_source {
    /*! The clock is gated */
    MOD_PIK_CLOCK_ACLKDPU_SOURCE_GATED = 0,
    /*! The clock source is set to the system reference clock */
    MOD_PIK_CLOCK_ACLKDPU_SOURCE_SYSREFCLK = 1,
    /*! The clock source is set to the dedicated display PLL */
    MOD_PIK_CLOCK_ACLKDPU_SOURCE_DISPLAYPLLCLK = 2,
    /*! The clock source is set to the system PLL clock */
    MOD_PIK_CLOCK_ACLKDPU_SOURCE_SYSPLLCLK = 4,
    /*! Number of valid clock sources */
    MOD_PIK_CLOCK_ACLKDPU_SOURCE_MAX
};

/*!
 * \brief Selectable clock sources for the video processor clock.
 */
enum mod_pik_clock_vpuclk_source {
    /*! The clock is gated */
    MOD_PIK_CLOCK_VPUCLK_SOURCE_GATED = 0,
    /*! The clock source is set to the system reference clock */
    MOD_PIK_CLOCK_VPUCLK_SOURCE_SYSREFCLK = 1,
    /*! The clock source is set to the system PLL clock */
    MOD_PIK_CLOCK_VPUCLK_SOURCE_SYSPLLCLK = 2,
    /*! The clock source is set to the dedicated video PLL */
    MOD_PIK_CLOCK_VPUCLK_SOURCE_VIDEOPLLCLK = 4,
    /*! Number of valid clock sources */
    MOD_PIK_CLOCK_VPUCLK_SOURCE_MAX
};

/*!
 * \brief Selectable clock sources for interconnect clock.
 */
enum mod_pik_clock_intclk_source {
    /*! The clock is gated */
    MOD_PIK_CLOCK_INTCLK_SOURCE_GATED = 0,
    /*! The clock source is set to the system reference clock */
    MOD_PIK_CLOCK_INTCLK_SOURCE_SYSREFCLK = 1,
    /*! The clock source is set to a private PLL */
    MOD_PIK_CLOCK_INTCLK_SOURCE_INTPLL = 2,
};

/*!
 * \brief Selectable clock sources for DMC clock.
 */
enum mod_pik_clock_dmcclk_source {
    /*! The clock is gated */
    MOD_PIK_CLOCK_DMCCLK_SOURCE_GATED = 0,
    /*! The clock source is set to the system reference clock */
    MOD_PIK_CLOCK_DMCCLK_SOURCE_REFCLK = 1,
    /*! The clock source is set to a private PLL */
    MOD_PIK_CLOCK_DMCCLK_SOURCE_DDRPLL = 2,
};

/*!
 * \brief Divider bitfield width.
 */
enum mod_pik_clock_divider_bitfield_width {
    /*! PIK clock with 4-bit divider. */
    MOD_PIK_CLOCK_DIVIDER_BITFIELD_WIDTH_4BITS = 4,
    /*! PIK clock with 5-bit divider. */
    MOD_PIK_CLOCK_DIVIDER_BITFIELD_WIDTH_5BITS = 5,
};

/*!
 * \brief PIK clock module configuration.
 */
struct mod_pik_clock_module_config {
    /*! The maximum divider value. */
    unsigned int divider_max;
};

/*!
 * \brief Rate lookup entry.
 */
struct mod_pik_clock_rate {
    /*! Rate in Hertz. */
    uint64_t rate;
    /*! Clock source used to obtain the rate (multi-source clocks only). */
    uint32_t source;
    /*! The divider register to use (multi-source clocks only). */
    enum mod_pik_clock_msclock_divider divider_reg;
    /*! Divider used to obtain the rate. */
    uint8_t divider;
};

/*!
 * \brief Subsystem clock device configuration.
 */
struct mod_pik_clock_dev_config {
    /*! The type of the clock device. */
    enum mod_pik_clock_type type;

    /*!
     * \brief Indicates whether the clock is part of a CSS clock group (\c true)
     *     or operating as an independent clock (\c false).
     *
     * \details The value determines the API that the clock exposes during
     *     binding. If the clock is part of a group then the \ref
     *     mod_pik_clock_api_type.MOD_PIK_CLOCK_API_TYPE_CSS API is exposed for
     *     direct control via the CSS Clock module, otherwise the \ref
     *     mod_pik_clock_api_type.MOD_PIK_CLOCK_API_TYPE_CLOCK API, defined by
     *     the Clock HAL, is exposed.
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
    const struct mod_pik_clock_rate *rate_table;

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
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SYNQUACER_PIK_CLOCK_H */
