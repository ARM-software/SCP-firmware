/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>
#include <rcar_common.h>
#include <rcar_iic_dvfs.h>
#include <rcar_mmap.h>
#include <rcar_pwc.h>

#include <fwk_attributes.h>

#include <arch_helpers.h>

#include <stdint.h>
#include <stdlib.h>

/* Suspend to ram   */
#define DBSC4_REG_BASE (0xE6790000U)
#define DBSC4_REG_DBSYSCNT0 (DBSC4_REG_BASE + 0x0100U)
#define DBSC4_REG_DBACEN (DBSC4_REG_BASE + 0x0200U)
#define DBSC4_REG_DBCMD (DBSC4_REG_BASE + 0x0208U)
#define DBSC4_REG_DBWAIT (DBSC4_REG_BASE + 0x0210U)
#define DBSC4_REG_DBRFEN (DBSC4_REG_BASE + 0x0204U)

#define DBSC4_REG_DBCALCNF (DBSC4_REG_BASE + 0x0424U)
#define DBSC4_REG_DBDFIPMSTRCNF (DBSC4_REG_BASE + 0x0520U)

#define DBSC4_REG_DBCAM0CTRL0 (DBSC4_REG_BASE + 0x0940U)
#define DBSC4_REG_DBCAM0STAT0 (DBSC4_REG_BASE + 0x0980U)
#define DBSC4_REG_DBCAM1STAT0 (DBSC4_REG_BASE + 0x0990U)
#define DBSC4_REG_DBCAM2STAT0 (DBSC4_REG_BASE + 0x09A0U)
#define DBSC4_REG_DBCAM3STAT0 (DBSC4_REG_BASE + 0x09B0U)

#define DBSC4_BIT_DBCAMxSTAT0 (0x00000001U)
#define DBSC4_BIT_DBDFIPMSTRCNF_PMSTREN (0x00000001U)
#define DBSC4_SET_DBCMD_OPC_PRE (0x04000000U)
#define DBSC4_SET_DBCMD_OPC_SR (0x0A000000U)
#define DBSC4_SET_DBCMD_OPC_PD (0x08000000U)
#define DBSC4_SET_DBCMD_OPC_MRW (0x0E000000U)
#define DBSC4_SET_DBCMD_ARG_ENTER (0x00000000U)
#define DBSC4_SET_DBCMD_ARG_MRW_ODTC (0x00000B00U)
#define DBSC4_SET_DBCMD_CH_ALL (0x00800000U)
#define DBSC4_SET_DBCMD_RANK_ALL (0x00040000U)
#define DBSC4_SET_DBCMD_ARG_ALL (0x00000010U)
#define DBSC4_SET_DBSYSCNT0_WRITE_ENABLE (0x00001234U)
#define DBSC4_SET_DBSYSCNT0_WRITE_DISABLE (0x00000000U)

#define BIT_BKUP_CTRL_OUT ((uint8_t)(1U << 4))
#define PMIC_RETRY_MAX (100U)
#define PMIC_BKUP_MODE_CNT (0x20U)
#define PMIC_QLLM_CNT (0x27U)
#define DVFS_SET_VID_0V (0x00)
#define P_ALL_OFF (0x80)

#define SRESCR_CODE (0x5AA5U << 16)
#define BIT_SOFTRESET (1U << 15)
#define SCTLR_EL3_M_BIT ((uint32_t)1U << 0)
#define RCAR_CONV_MICROSEC (1000000U)

#define DBCAM_FLUSH(__bit) \
    do { \
        ; \
    } while (!( \
        mmio_read_32(DBSC4_REG_DBCAM##__bit##STAT0) & DBSC4_BIT_DBCAMxSTAT0))

extern uint32_t rcar_pwrc_switch_stack(
    uintptr_t jump,
    uintptr_t stack,
    void *arg);

extern void panic(void);

static void FWK_SECTION(".system_ram") rcar_pwrc_set_self_refresh(void)
{
    uint32_t reg = mmio_read_32(RCAR_PRR);
    uint32_t cut, product;

    product = reg & RCAR_PRODUCT_MASK;
    cut = reg & RCAR_CUT_MASK;

    if (!((product == RCAR_PRODUCT_M3 && cut < RCAR_CUT_VER30) ||
          (product == RCAR_PRODUCT_H3 && cut < RCAR_CUT_VER20)))
        mmio_write_32(DBSC4_REG_DBSYSCNT0, DBSC4_SET_DBSYSCNT0_WRITE_ENABLE);

    /* DFI_PHYMSTR_ACK setting */
    mmio_write_32(
        DBSC4_REG_DBDFIPMSTRCNF,
        mmio_read_32(DBSC4_REG_DBDFIPMSTRCNF) &
            (~DBSC4_BIT_DBDFIPMSTRCNF_PMSTREN));

    /* Set the Self-Refresh mode */
    mmio_write_32(DBSC4_REG_DBACEN, 0);

    if (product == RCAR_PRODUCT_H3 && cut < RCAR_CUT_VER20)
        udelay(100);
    else if (product == RCAR_PRODUCT_H3) {
        mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 1);
        DBCAM_FLUSH(0);
        DBCAM_FLUSH(1);
        DBCAM_FLUSH(2);
        DBCAM_FLUSH(3);
        mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 0);
    } else if (product == RCAR_PRODUCT_M3) {
        mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 1);
        DBCAM_FLUSH(0);
        DBCAM_FLUSH(1);
        mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 0);
    } else {
        mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 1);
        DBCAM_FLUSH(0);
        mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 0);
    }

    /* Set the SDRAM calibration configuration register */
    mmio_write_32(DBSC4_REG_DBCALCNF, 0);

    reg = DBSC4_SET_DBCMD_OPC_PRE | DBSC4_SET_DBCMD_CH_ALL |
        DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_ALL;
    mmio_write_32(DBSC4_REG_DBCMD, reg);
    while (mmio_read_32(DBSC4_REG_DBWAIT))
        continue;

    /* Self-Refresh entry command   */
    reg = DBSC4_SET_DBCMD_OPC_SR | DBSC4_SET_DBCMD_CH_ALL |
        DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_ENTER;
    mmio_write_32(DBSC4_REG_DBCMD, reg);
    while (mmio_read_32(DBSC4_REG_DBWAIT))
        continue;

    /* Mode Register Write command. (ODT disabled)  */
    reg = DBSC4_SET_DBCMD_OPC_MRW | DBSC4_SET_DBCMD_CH_ALL |
        DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_MRW_ODTC;
    mmio_write_32(DBSC4_REG_DBCMD, reg);
    while (mmio_read_32(DBSC4_REG_DBWAIT))
        continue;

    /* Power Down entry command     */
    reg = DBSC4_SET_DBCMD_OPC_PD | DBSC4_SET_DBCMD_CH_ALL |
        DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_ENTER;
    mmio_write_32(DBSC4_REG_DBCMD, reg);
    while (mmio_read_32(DBSC4_REG_DBWAIT))
        continue;

    /* Set the auto-refresh enable register */
    mmio_write_32(DBSC4_REG_DBRFEN, 0U);
    udelay(1);

    if (product == RCAR_PRODUCT_M3 && cut < RCAR_CUT_VER30)
        return;

    if (product == RCAR_PRODUCT_H3 && cut < RCAR_CUT_VER20)
        return;

    mmio_write_32(DBSC4_REG_DBSYSCNT0, DBSC4_SET_DBSYSCNT0_WRITE_DISABLE);
}

void FWK_SECTION(".system_ram") FWK_NOINLINE rcar_pwrc_go_suspend_to_ram(void)
{
    int32_t rc = -1, qllm = -1;
    uint8_t mode;
    uint32_t i;

    rcar_pwrc_set_self_refresh();

    /* Set QLLM Cnt Disable */
    for (i = 0; (i < PMIC_RETRY_MAX) && (qllm != 0); i++)
        qllm = rcar_iic_dvfs_send(PMIC, PMIC_QLLM_CNT, 0);

    /* Set trigger of power down to PMIV */
    for (i = 0; (i < PMIC_RETRY_MAX) && (rc != 0) && (qllm == 0); i++) {
        rc = rcar_iic_dvfs_receive(PMIC, PMIC_BKUP_MODE_CNT, &mode);
        if (rc == 0) {
            mode |= BIT_BKUP_CTRL_OUT;
            rc = rcar_iic_dvfs_send(PMIC, PMIC_BKUP_MODE_CNT, mode);
        }
    }

    wfi();

    while (1)
        continue;
}

void rcar_pwrc_set_suspend_to_ram(void)
{
    uintptr_t jump = (uintptr_t)&rcar_pwrc_go_suspend_to_ram;
    uintptr_t stack = (uintptr_t)(SCP_SRAM_STACK_BASE + SCP_SRAM_STACK_SIZE);
    uint32_t sctlr;

    /* disable MMU */
    sctlr = (uint32_t)read_sctlr_el3();
    sctlr &= (uint32_t)~SCTLR_EL3_M_BIT;
    write_sctlr_el3((uint64_t)sctlr);

    rcar_pwrc_switch_stack(jump, stack, NULL);
}

void rcar_system_off(void)
{
    if (rcar_iic_dvfs_send(PMIC, DVFS_SET_VID, DVFS_SET_VID_0V))
        panic();
}

void rcar_system_reset(void)
{
#if RCAR_SOFTWARE_RESET
    mmio_write_32(RCAR_SRESCR, SRESCR_CODE | BIT_SOFTRESET);
#else
    if (rcar_iic_dvfs_send(PMIC, BKUP_MODE_CNT, P_ALL_OFF))
        panic();
#endif /* RCAR_SOFTWARE_RESET */
}
