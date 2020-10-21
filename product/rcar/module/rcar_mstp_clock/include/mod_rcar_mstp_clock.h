/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_MSTP_CLOCK_H
#define MOD_RCAR_MSTP_CLOCK_H

#include <rcar_mmap.h>

#include <mod_clock.h>
#include <mod_rcar_clock.h>

#include <fwk_element.h>

#include <stdint.h>

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * @{
 */

/*!
 * \defgroup GroupRCARMstpClock MSTP Clock
 * @{
 */

/*!
 * \brief Subsystem clock device configuration.
 */
struct mod_rcar_mstp_clock_dev_config {
    /*! Pointer to the clock's control register. */
    volatile uint32_t const control_reg;

    /*! enable / disable bit position. */
    volatile uint32_t const bit;

    /*! If true, the driver will provide a default clock supply. */
    const bool defer_initialization;
};

/*!
 * @cond
 */

/* Device context */
struct rcar_mstp_clock_dev_ctx {
    bool initialized;
    uint64_t current_rate;
    enum mod_clock_state current_state;
    const struct mod_rcar_mstp_clock_dev_config *config;
    struct mod_rcar_clock_drv_api *api;
};

struct mod_rcar_mstp_clock_init {
    volatile uint32_t const smstpcr_init[12];
};

/* Module context */
struct rcar_mstp_clock_ctx {
    struct rcar_mstp_clock_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
    const struct mod_rcar_mstp_clock_init *mstp_init;
};

/* Module Stop Status Register offsets */
static const uint16_t mstpsr[] = {
    0x030, 0x038, 0x040, 0x048, 0x04C, 0x03C,
    0x1C0, 0x1C4, 0x9A0, 0x9A4, 0x9A8, 0x9AC,
};

/* System Module Stop Control Register offsets */
static const uint16_t smstpcr[] = {
    0x130, 0x134, 0x138, 0x13C, 0x140, 0x144,
    0x148, 0x14C, 0x990, 0x994, 0x998, 0x99C,
};

/* System Module Stop Control Register Number */
#define CPG_SMSTPCR1 1
#define CPG_SMSTPCR2 2
#define CPG_SMSTPCR3 3
#define CPG_SMSTPCR4 4
#define CPG_SMSTPCR5 5
#define CPG_SMSTPCR6 6
#define CPG_SMSTPCR7 7
#define CPG_SMSTPCR8 8
#define CPG_SMSTPCR9 9
#define CPG_SMSTPCR10 10

/* System Module Stop Control Register Address */
#define SMSTPCR0    (CPG_BASE + 0x0130U)
#define SMSTPCR1    (CPG_BASE + 0x0134U)
#define SMSTPCR2    (CPG_BASE + 0x0138U)
#define SMSTPCR3    (CPG_BASE + 0x013CU)
#define SMSTPCR4    (CPG_BASE + 0x0140U)
#define SMSTPCR5    (CPG_BASE + 0x0144U)
#define SMSTPCR6    (CPG_BASE + 0x0148U)
#define SMSTPCR7    (CPG_BASE + 0x014CU)
#define SMSTPCR8    (CPG_BASE + 0x0990U)
#define SMSTPCR9    (CPG_BASE + 0x0994U)
#define SMSTPCR10   (CPG_BASE + 0x0998U)
#define SMSTPCR11   (CPG_BASE + 0x099CU)

/* System Module Stop Control Register Init Value */
#define SMSTPCR0_VALUE  (0x00210000U)
#define SMSTPCR1_VALUE  (0xFFFFFFFFU)
#define SMSTPCR2_VALUE  (0x040E2FDCU)
#define SMSTPCR3_VALUE  (0xFFFFFBDFU)
#define SMSTPCR4_VALUE  (0x80000004U)
#define SMSTPCR5_VALUE  (0xC3BFFFFFU)
#define SMSTPCR6_VALUE  (0xFFFFFFFFU)
#define SMSTPCR7_VALUE  (0xFFFFFFFFU)
#define SMSTPCR8_VALUE  (0x01F1FFF5U)
#define SMSTPCR9_VALUE  (0xFFFFFFFFU)
#define SMSTPCR10_VALUE (0xFFFEFFE0U)
#define SMSTPCR11_VALUE (0x000000B7U)

/*!
 * @endcond
 */

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_MSTP_CLOCK_H */
