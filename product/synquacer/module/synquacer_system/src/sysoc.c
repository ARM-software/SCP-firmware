/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "low_level_access.h"
#include "synquacer_mmap.h"

#include <cmsis_os2.h>

#include <internal/reset.h>
#include <internal/sysoc.h>

#include <mod_synquacer_system.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAIN_BUS_RESET_BIT (0x1)

#define REG_ADDR_RSTSTA (0)
#define REG_ADDR_RSTSET (0x4U)
#define REG_ADDR_RSTCLR (0x8U)

#define SYSOC_WAIT_TIME_MS 1U
#define SYSOC_WAIT_NUM 500

#define SYSOC_OFFSET_RESET_STATUS (0)
#define SYSOC_OFFSET_RESET (4)
#define SYSOC_OFFSET_RESET_CLEAR (8)

#define LPCM_OFFSET_CONTROL (0)
#define LPCM_OFFSET_MON (4)

#define LPCM_OFFSET_NUM (8)

static const struct {
    uint32_t block_sysoc;
    uint32_t block_lpcm;
    uint32_t bus_sysoc;
    uint32_t bus_lpcm;
    uint32_t addr_sysoc_blk;
    uint32_t addr_sysoc_bus;
    uint32_t addr_lpcm;
} reset_info[] = RESET_INFO;

void sysoc_set_reset(uint32_t sysoc_addr, uint32_t value)
{
    if ((void *)sysoc_addr == NULL)
        return;

    writel((sysoc_addr + REG_ADDR_RSTSET), value);
}

void sysoc_clr_reset(uint32_t sysoc_addr, uint32_t value)
{
    if ((void *)sysoc_addr == NULL)
        return;

    writel((sysoc_addr + REG_ADDR_RSTCLR), value);
}

uint32_t sysoc_read_reset_status(uint32_t sysoc_addr)
{
    if ((void *)sysoc_addr == NULL)
        return 0;

    return readl(sysoc_addr + REG_ADDR_RSTSTA);
}

int sysoc_wait_status_change(
    uint32_t sysoc_addr,
    bool reset_set_flag,
    uint32_t set_bit)
{
    uint32_t i;
    uint32_t wait_bit;
    int ret;

    if ((void *)sysoc_addr == NULL)
        return FWK_E_PARAM;

    if (reset_set_flag)
        wait_bit = set_bit;
    else
        wait_bit = 0;

    /* wait ppu power status change */
    for (i = 0, ret = FWK_E_TIMEOUT; i < SYSOC_WAIT_NUM; i++) {
        if ((readl(sysoc_addr + REG_ADDR_RSTSTA) & set_bit) == wait_bit) {
            ret = FWK_SUCCESS;
            break;
        }
        /* sleep for wait status change */
        if (osDelay(SYSOC_WAIT_TIME_MS) != osOK) {
            /* sleep error */
            return FWK_E_OS;
        }
    }
    return ret;
}

static void sysoc_reset(uint32_t addr_head, uint32_t bitmap)
{
    writel(addr_head + SYSOC_OFFSET_RESET, bitmap);
}

static void sysoc_reset_clear(uint32_t addr_head, uint32_t bitmap)
{
    writel(addr_head + SYSOC_OFFSET_RESET_CLEAR, bitmap);
}

static void lpcm_reset(uint32_t addr_head, uint32_t bitmap)
{
    int i;

    for (i = 0; i < LPCM_OFFSET_NUM; i++) {
        if ((bitmap & 1) == 1)
            writel(addr_head + (i * 8), 0);

        bitmap >>= 1;
    }
}

static void lpcm_reset_clear(uint32_t addr_head, uint32_t bitmap)
{
    int i;

    for (i = 0; i < LPCM_OFFSET_NUM; i++) {
        if ((bitmap & 1) == 1)
            writel(addr_head + (i * 8), 1);

        bitmap >>= 1;
    }
}

static uint32_t sysoc_reset_status(uint32_t addr_head, uint32_t bitmap)
{
    return readl(addr_head + SYSOC_OFFSET_RESET_STATUS) & bitmap;
}

static uint32_t lpcm_reset_status(uint32_t addr_head, uint32_t bitmap)
{
    int i, status_bitmap, b;
    status_bitmap = 0;

    for (i = 0; i < LPCM_OFFSET_NUM; i++) {
        if ((bitmap & 1) == 1) {
            b = readl(addr_head + (i * 8) + LPCM_OFFSET_MON) & 1;
            status_bitmap |= b << i;
        }
        bitmap >>= 1;
    }
    return status_bitmap;
}

#define RESET_TIMEOUT_MS 100
#define RESET_CHECK_CYCLE_MS 1

void lpcm_sysoc_reset(RST_TYPE_t type, RST_BLOCK block)
{
    uint32_t lpcm_bitmap, sysoc_bitmap_blk, sysoc_bitmap_bus;
    uint32_t status_check_num, i, r;

    lpcm_bitmap = 0;
    sysoc_bitmap_blk = 0;
    sysoc_bitmap_bus = 0;

    status_check_num = (RESET_TIMEOUT_MS / RESET_CHECK_CYCLE_MS);

    if (type != RST_TYPE_BUS) {
        sysoc_bitmap_blk |= reset_info[block].block_sysoc;
        lpcm_bitmap |= reset_info[block].block_lpcm;
    }
    if (type != RST_TYPE_WO_BUS) {
        sysoc_bitmap_bus |= reset_info[block].bus_sysoc;
        lpcm_bitmap |= reset_info[block].bus_lpcm;
    }

    if (lpcm_bitmap != 0)
        lpcm_reset(reset_info[block].addr_lpcm, lpcm_bitmap);

    if (sysoc_bitmap_bus != 0)
        sysoc_reset(reset_info[block].addr_sysoc_bus, sysoc_bitmap_bus);

    if (sysoc_bitmap_blk != 0)
        sysoc_reset(reset_info[block].addr_sysoc_blk, sysoc_bitmap_blk);

    if (lpcm_bitmap != 0) {
        for (i = 0; i < status_check_num; i++) {
            r = lpcm_reset_status(reset_info[block].addr_lpcm, lpcm_bitmap);
            if (r == 0)
                break;

            osDelay(RESET_CHECK_CYCLE_MS);
        }
        if (i == status_check_num) {
            FWK_LOG_ERR(
                "[LPCM] Reset timeout.(%dms, %08" PRIx32 ")",
                RESET_TIMEOUT_MS,
                reset_info[block].addr_lpcm);
        }
    }

    if (sysoc_bitmap_bus != 0) {
        for (i = 0; i < status_check_num; i++) {
            r = sysoc_reset_status(
                reset_info[block].addr_sysoc_bus, sysoc_bitmap_bus);
            if (r == sysoc_bitmap_bus)
                break;

            osDelay(RESET_CHECK_CYCLE_MS);
        }
        if (i == status_check_num) {
            FWK_LOG_ERR(
                "[SYSOC] Reset timeout.(%dms, %08" PRIx32 ")",
                RESET_TIMEOUT_MS,
                reset_info[block].addr_sysoc_bus);
        }
    }

    if (sysoc_bitmap_blk != 0) {
        for (i = 0; i < status_check_num; i++) {
            r = sysoc_reset_status(
                reset_info[block].addr_sysoc_blk, sysoc_bitmap_blk);
            if (r == sysoc_bitmap_blk)
                break;

            osDelay(RESET_CHECK_CYCLE_MS);
        }
        if (i == status_check_num) {
            FWK_LOG_ERR(
                "[SYSOC] Reset timeout.(%dms, %08" PRIx32 ")",
                RESET_TIMEOUT_MS,
                reset_info[block].addr_sysoc_blk);
        }
    }
}

void lpcm_sysoc_reset_clear(RST_TYPE_t type, RST_BLOCK block)
{
    uint32_t lpcm_bitmap, sysoc_bitmap_blk, sysoc_bitmap_bus;
    uint32_t status_check_num, i, r;

    lpcm_bitmap = 0;
    sysoc_bitmap_blk = 0;
    sysoc_bitmap_bus = 0;

    status_check_num = (RESET_TIMEOUT_MS / RESET_CHECK_CYCLE_MS);

    if (type != RST_TYPE_BUS) {
        sysoc_bitmap_blk |= reset_info[block].block_sysoc;
        lpcm_bitmap |= reset_info[block].block_lpcm;
    }
    if (type != RST_TYPE_WO_BUS) {
        sysoc_bitmap_bus |= reset_info[block].bus_sysoc;
        lpcm_bitmap |= reset_info[block].bus_lpcm;
    }

    if (sysoc_bitmap_blk != 0)
        sysoc_reset_clear(reset_info[block].addr_sysoc_blk, sysoc_bitmap_blk);

    if (sysoc_bitmap_bus != 0)
        sysoc_reset_clear(reset_info[block].addr_sysoc_bus, sysoc_bitmap_bus);

    if (lpcm_bitmap != 0)
        lpcm_reset_clear(reset_info[block].addr_lpcm, lpcm_bitmap);

    if (sysoc_bitmap_blk != 0) {
        for (i = 0; i < status_check_num; i++) {
            r = sysoc_reset_status(
                reset_info[block].addr_sysoc_blk, sysoc_bitmap_blk);
            if (r == 0)
                break;

            osDelay(RESET_CHECK_CYCLE_MS);
        }
        if (i == status_check_num) {
            FWK_LOG_ERR(
                "[SYSOC] Reset clear timeout.(%dms, %08" PRIx32 ")",
                RESET_TIMEOUT_MS,
                reset_info[block].addr_sysoc_blk);
        }
    }

    if (sysoc_bitmap_bus != 0) {
        for (i = 0; i < status_check_num; i++) {
            r = sysoc_reset_status(
                reset_info[block].addr_sysoc_bus, sysoc_bitmap_bus);
            if (r == 0)
                break;

            osDelay(RESET_CHECK_CYCLE_MS);
        }
        if (i == status_check_num) {
            FWK_LOG_ERR(
                "[SYSOC] Reset clear timeout.(%dms, %08" PRIx32 ")",
                RESET_TIMEOUT_MS,
                reset_info[block].addr_sysoc_bus);
        }
    }

    if (lpcm_bitmap != 0) {
        for (i = 0; i < status_check_num; i++) {
            r = lpcm_reset_status(reset_info[block].addr_lpcm, lpcm_bitmap);
            if (r == lpcm_bitmap)
                break;

            osDelay(RESET_CHECK_CYCLE_MS);
        }
        if (i == status_check_num) {
            FWK_LOG_ERR(
                "[LPCM] Reset clear timeout.(%dms, %08" PRIx32 ")",
                RESET_TIMEOUT_MS,
                reset_info[block].addr_lpcm);
        }
    }
}

void bus_sysoc_init(void)
{
    int result;
    uint32_t value;

    (void)result;

    value = sysoc_read_reset_status(CONFIG_SOC_REG_ADDR_SYSOC_BUS_TOP);
    if (value == 0)
        return; /* reset bit not active */

    /* reset set only active bit */
    sysoc_clr_reset(
        CONFIG_SOC_REG_ADDR_SYSOC_BUS_TOP, (MAIN_BUS_RESET_BIT & value));
    result = sysoc_wait_status_change(
        CONFIG_SOC_REG_ADDR_SYSOC_BUS_TOP, false, MAIN_BUS_RESET_BIT);

    assert(result == FWK_SUCCESS);
}

#define CONFIG_SOC_PCIEB_SYSOC_REG_BASE UINT32_C(0x78240000)

/* configure PCIEB SYSOC as secure */
void pcie_wrapper_configure_pcieb_sysoc(void)
{
    const struct {
        uint32_t offset;
        uint32_t value;
    } pcieb_sysoc_config[] = {
        { 0x00, 0 }, /* CFG_CTTW */
        { 0x04, 0 }, /* CFG_NORMALIZE */
        { 0x08, 0 }, /* INTEG_SEC_OVERRIDE */
        { 0x0C, 0 }, /* SYSBARDISABLE_TBU */
        { 0x10, 0 }, /* WSB_NS */
        { 0x14, 0 }, /* RSB_NS */
        { 0x18, 0 }, /* AWDOMAIN */
        { 0x1C, 0 } /* ARDOMAIN */
    };

    size_t i;

    for (i = 0; i < FWK_ARRAY_SIZE(pcieb_sysoc_config); i++) {
        writel(
            CONFIG_SOC_PCIEB_SYSOC_REG_BASE + pcieb_sysoc_config[i].offset,
            pcieb_sysoc_config[i].value);
    }
}

void pcie_wrapper_tweak_slv_axmisc_info_tph(uint32_t tweak)
{
    writel(0x7FFF0080, tweak);
}

void pcie_wrapper_configure(void)
{
    pcie_wrapper_configure_pcieb_sysoc();
    pcie_wrapper_tweak_slv_axmisc_info_tph(
        CONFIG_SCB_TWEAK_PCIE_TLP_BE_OVERRIDE);
}
