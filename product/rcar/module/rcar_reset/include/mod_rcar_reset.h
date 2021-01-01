/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_RESET_H
#define MOD_RCAR_RESET_H

#include <stdint.h>
#include <fwk_element.h>
#include <rcar_mmap.h>
#include <mod_reset_domain.h>

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * @{
 */

/*!
 * \defgroup GroupRCARReset Reset
 * @{
 */

/*!
 * \brief APIs provided by the driver.
 */
enum mod_rcar_reset_api_type {
    MOD_RCAR_RESET_API_TYPE_CLOCK,
    MOD_RCAR_RESET_API_COUNT,
};

/*!
 * \brief Subsystem reset device configuration.
 */
struct mod_rcar_reset_dev_config {
    /*! Pointer to the reset's control register. */
    volatile uint32_t const control_reg;

    /*! enable / disable bit position. */
    volatile uint32_t const bit;
};

/*!
 * @cond
 */

/* Device context */
struct rcar_reset_dev_ctx {
    const struct mod_rcar_reset_dev_config *config;
    struct mod_reset_domain_drv_api *api;
};

/* Module context */
struct rcar_reset_ctx {
    struct rcar_reset_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
};

/*
 * Module Reset Control Register offsets
 */
static const uint16_t srcr[] = {
    0x0A0, 0x0A8, 0x0B0, 0x0B8, 0x0BC, 0x0C4, 0x1C8, 0x1CC,
    0x920, 0x924, 0x928, 0x92C,
};
#define CPG_SMSTPCR1    1
#define CPG_SMSTPCR2    2
#define CPG_SMSTPCR3    3
#define CPG_SMSTPCR4    4
#define CPG_SMSTPCR5    5
#define CPG_SMSTPCR6    6
#define CPG_SMSTPCR7    7
#define CPG_SMSTPCR8    8
#define CPG_SMSTPCR9    9
#define CPG_SMSTPCR10   10

/* Software Reset Clearing Register offsets */
#define SRSTCLR(i) (0x940 + (i) * 4)

#define DELAY_CNT_1US       (10UL)
#define SCSR_DELAY_US       (DELAY_CNT_1US * 35)

/*!
 * @endcond
 */

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_RESET_H */
