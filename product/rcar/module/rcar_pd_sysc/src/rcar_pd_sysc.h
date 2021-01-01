/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_PD_SYSC_H
#define RCAR_PD_SYSC_H

/*!
 * \cond
 * @{
 */

#include <mod_rcar_pd_sysc.h>

#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

#define BIT_SHIFT(nr) (1UL << (nr))

/* SYSC Common */
#define SYSC_BASE_ADDR (0xE6180000U) /* SYSC Base Address*/
#define SYSCSR 0x00 /* SYSC Status Register */
#define SYSCISR 0x04 /* Interrupt Status Register */
#define SYSCISCR 0x08 /* Interrupt Status Clear Register */
#define SYSCIER 0x0c /* Interrupt Enable Register */
#define SYSCIMR 0x10 /* Interrupt Mask Register */

/* SYSC Status Register */
#define SYSCSR_PONENB 1 /* Ready for power resume requests */
#define SYSCSR_POFFENB 0 /* Ready for power shutoff requests */

/*
 * Power Control Register Offsets inside the register block for each domain
 * Note: The "CR" registers for ARM cores exist on H1 only
 * Use WFI to power off, CPG/APMU to resume ARM cores on R-Car Gen2
 * Use PSCI on R-Car Gen3
 */
#define PWRSR_OFFS 0x00 /* Power Status Register */
#define PWROFFCR_OFFS 0x04 /* Power Shutoff Control Register */
#define PWROFFSR_OFFS 0x08 /* Power Shutoff Status Register */
#define PWRONCR_OFFS 0x0c /* Power Resume Control Register */
#define PWRONSR_OFFS 0x10 /* Power Resume Status Register */
#define PWRER_OFFS 0x14 /* Power Shutoff/Resume Error */

#define SYSCSR_RETRIES 1000
#define SYSCSR_DELAY_US 1

#define PWRER_RETRIES 1000
#define PWRER_DELAY_US 1

#define SYSCISR_RETRIES 1000
#define SYSCISR_DELAY_US 1

/*
 * Interface
 */
int rcar_sysc_power(struct rcar_sysc_pd_ctx *pd_ctx, bool on);
int rcar_sysc_power_get(struct rcar_sysc_pd_ctx *pd_ctx, unsigned int *statee);

/*!
 * \endcond
 * @}
 */

#endif /* RCAR_PD_SYSC_H */
