/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARMV8A_GIC_H
#define ARMV8A_GIC_H

#include <fwk_arch.h>

#include <arch_helpers.h>

/* Constants to categorise priorities */
#define GIC_HIGHEST_SEC_PRIORITY 0x0
#define GIC_LOWEST_SEC_PRIORITY 0x7f
#define GIC_HIGHEST_NS_PRIORITY 0x80
#define GIC_LOWEST_NS_PRIORITY 0xfe /* 0xff would disable all interrupts */

/*******************************************************************************
 * GIC Distributor interface general definitions
 ******************************************************************************/
/* Constants to categorise interrupts */
#define MIN_SGI_ID U(0)
#define MIN_SEC_SGI_ID U(8)
#define MIN_PPI_ID U(16)
#define MIN_SPI_ID U(32)
#define MAX_SPI_ID U(1019)

/* Mask for the priority field common to all GIC interfaces */
#define GIC_PRI_MASK U(0xff)

/* Mask for the configuration field common to all GIC interfaces */
#define GIC_CFG_MASK U(0x3)

/*******************************************************************************
 * GIC Distributor interface register offsets that are common to GICv2
 ******************************************************************************/
#define GICD_CTLR U(0x0)
#define GICD_TYPER U(0x4)
#define GICD_IIDR U(0x8)
#define GICD_IGROUPR U(0x80)
#define GICD_ISENABLER U(0x100)
#define GICD_ICENABLER U(0x180)
#define GICD_ISPENDR U(0x200)
#define GICD_ICPENDR U(0x280)
#define GICD_ISACTIVER U(0x300)
#define GICD_ICACTIVER U(0x380)
#define GICD_IPRIORITYR U(0x400)
#define GICD_ITARGETSR U(0x800)
#define GICD_ICFGR U(0xc00)
#define GICD_NSACR U(0xe00)

/* GICD_CTLR bit definitions */
#define CTLR_ENABLE_G0_SHIFT 0
#define CTLR_ENABLE_G0_MASK U(0x1)
#define CTLR_ENABLE_G0_BIT BIT_32(CTLR_ENABLE_G0_SHIFT)
#define CTLR_ENABLE_G1_SHIFT 1
#define CTLR_ENABLE_G1_MASK U(0x1)
#define CTLR_ENABLE_G1_BIT BIT_32(CTLR_ENABLE_G1_SHIFT)

/*******************************************************************************
 * GICv2 specific CPU interface register offsets and constants.
 ******************************************************************************/
/* Physical CPU Interface registers */
#define GICC_CTLR U(0x0)
#define GICC_PMR U(0x4)
#define GICC_BPR U(0x8)
#define GICC_IAR U(0xC)
#define GICC_EOIR U(0x10)
#define GICC_RPR U(0x14)
#define GICC_HPPIR U(0x18)
#define GICC_AHPPIR U(0x28)
#define GICC_IIDR U(0xFC)
#define GICC_DIR U(0x1000)
#define GICC_PRIODROP GICC_EOIR

/* Common CPU Interface definitions */
#define INT_ID_MASK U(0x3ff)
#define INT_ID(n) (n & INT_ID_MASK)

/* GICC_CTLR bit definitions */
#define EOI_MODE_NS (U(1) << 10)
#define EOI_MODE_S (U(1) << 9)
#define IRQ_BYP_DIS_GRP1 (U(1) << 8)
#define FIQ_BYP_DIS_GRP1 (U(1) << 7)
#define IRQ_BYP_DIS_GRP0 (U(1) << 6)
#define FIQ_BYP_DIS_GRP0 (U(1) << 5)
#define CBPR (U(1) << 4)
#define FIQ_EN (U(1) << 3)
#define ACK_CTL (U(1) << 2)
#define ENABLE_G1 (U(1) << 1)
#define ENABLE_G0 (U(1) << 0)
#define FIQ_EN_SHIFT 3
#define FIQ_EN_BIT BIT_32(FIQ_EN_SHIFT)

/*******************************************************************************
 * GIC Distributor interface register constants that are common to GICv3 & GICv2
 ******************************************************************************/
#define PIDR2_ARCH_REV_SHIFT 4
#define PIDR2_ARCH_REV_MASK U(0xf)

/* GICv3 revision as reported by the PIDR2 register */
#define ARCH_REV_GICV3 U(0x3)
/* GICv2 revision as reported by the PIDR2 register */
#define ARCH_REV_GICV2 U(0x2)
/* GICv1 revision as reported by the PIDR2 register */
#define ARCH_REV_GICV1 U(0x1)

#define IGROUPR_SHIFT 5
#define ISENABLER_SHIFT 5
#define ICENABLER_SHIFT ISENABLER_SHIFT
#define ISPENDR_SHIFT 5
#define ICPENDR_SHIFT ISPENDR_SHIFT
#define ISACTIVER_SHIFT 5
#define ICACTIVER_SHIFT ISACTIVER_SHIFT
#define IPRIORITYR_SHIFT 2
#define ITARGETSR_SHIFT 2
#define ICFGR_SHIFT 4
#define NSACR_SHIFT 4

/* GIC */
#define RCAR_GICD_BASE U(0xF1010000)
#define RCAR_GICR_BASE U(0xF1010000)
#define RCAR_GICC_BASE U(0xF1020000)
#define RCAR_GICH_BASE U(0xF1040000)
#define RCAR_GICV_BASE U(0xF1060000)

void gic_init(void);
int arm_gic_init(const struct fwk_arch_interrupt_driver **driver);
void irq_global(uint32_t iid);

#endif /* ARMV8A_GIC_H */
