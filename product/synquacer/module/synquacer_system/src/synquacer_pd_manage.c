/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "low_level_access.h"

#include <cmsis_os2.h>

#include <internal/pmu.h>
#include <internal/reg_PPU.h>
#include <internal/reset.h>
#include <internal/synquacer_pd.h>
#include <internal/synquacer_ppu_driver.h>
#include <internal/transaction_sw.h>

#include <mod_synquacer_system.h>

#include <fwk_log.h>
#include <fwk_macros.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#define SCB_DIV_ROUND_UP(NUMERATOR, DENOMINATOR) \
    (((NUMERATOR) + (DENOMINATOR)-1) / (DENOMINATOR))
#define POWER_CONV_MS_TO_PWR_ON_CYC(MS) \
    (SCB_DIV_ROUND_UP(((MS)*32768), 32000) - 1)

#define PD_TIMEOUT_COUNT ((PD_TIMEOUT_MS / PD_CHECK_CYCLE_MS) + 1)

struct {
    int32_t on_priority;
    uint32_t dev_bitmap;
    int32_t onchip_pd;
    int32_t offchip_pd;
    int32_t offchip_cycle_ms;
} sni_pmu_info[] = SNI_PMU_INFO_MP;

static const struct {
    uint32_t no;
    uint32_t dev_bitmap;
    bool booting_force_off;
    bool force_off;
} sni_ppu_info[] = SNI_PPU_INFO_MP;

static const struct {
    uint32_t dev_bitmap;
    uint32_t reg_no;
    uint32_t sw_bitmap;
} sni_transw_info[] = SNI_TRANSACTIONSW_INFO_MP;

#define FOR_EACH_PMU_INFO(index) \
    for (index = 0; index < FWK_ARRAY_SIZE(sni_pmu_info); index++)
#define FOR_EACH_PPU_INFO(index) \
    for (index = 0; index < FWK_ARRAY_SIZE(sni_ppu_info); index++)
#define FOR_EACH_TRANSW_INFO(index) \
    for (index = 0; index < FWK_ARRAY_SIZE(sni_transw_info); index++)
#define FOR_EACH_TRANSW_REG(index) \
    for (index = 0; index < TRANSW_REG_NUM; index++)

#define CONFIG_SOC_REG_ADDR_SYSOC_TRANSW_TOP UINT32_C(0x48110000 + 0x740)

#define TRANSW_ADDR(tran_tbl_num) \
    (CONFIG_SOC_REG_ADDR_SYSOC_TRANSW_TOP + tran_tbl_num * 8)

uint32_t get_domain_base_address(int domain)
{
    uint32_t address;

    switch (domain) {
    case 0:
        address = PPU0_BASE;
        break;
    case 1:
        address = PPU1_BASE;
        break;
    case 2:
        address = PPU2_BASE;
        break;
    case 3:
        address = PPU3_BASE;
        break;
    case 4:
        address = PPU4_BASE;
        break;
    case 5:
        address = PPU5_BASE;
        break;
    case 6:
        address = PPU6_BASE;
        break;
    case 7:
        address = PPU7_BASE;
        break;
    case 8:
        address = PPU8_BASE;
        break;
    default:
        address = ~0;
    }

    return address;
}

int change_power_state(
    int domain,
    int next_power_policy,
    int hwcactiveen,
    int hwcsysreqen,
    int reten)
{
    uint32_t base_address = get_domain_base_address(domain);
    uint32_t ppr = base_address + PPU_PPR_OFFSET;
    uint32_t psr = base_address + PPU_PSR_OFFSET;
    uint32_t pcr = base_address + PPU_PCR_OFFSET;
    uint32_t wdata;

    int current_power_plicy = (readl(psr) & 0x1F);

    if (current_power_plicy == next_power_policy)
        return 1;

    wdata = (reten << 4) | (hwcsysreqen << 1) | hwcactiveen;
    writel(pcr, wdata);

    writel(ppr, next_power_policy);

    return 0;
}

int read_power_status(int domain)
{
    uint32_t base_address = get_domain_base_address(domain);
    uint32_t psr = base_address + PPU_PSR_OFFSET;

    if (base_address == (uint32_t)~0)
        return 0;

    return readl(psr);
}

static void sni_power_domain_workaround_mp(void)
{
    /*
     * sni-ppu force off
     *    --- Trouble-shooting case2 by Verification-team
     *
     * "booting force off" is FALSE : active_en = 0, sysreq_en = 1
     *                        TRUE  : active_en = 0, sysreq_en = 0
     *
     * [CAUTION] Before this process need reset-clear of adb-400(pcie and dmab)
     */
    uint32_t i, j;
    int r;

    FWK_LOG_INFO("[PowerDomain] Socionext-PPU initialize .");

    /* ppu off */
    for (i = 0; i < FWK_ARRAY_SIZE(sni_ppu_info); i++) {
        uint32_t hwcsysreqen;
        hwcsysreqen = (sni_ppu_info[i].booting_force_off) ? 0 : 1;

        r = change_power_state(
            sni_ppu_info[i].no, PPU_PP_OFF, 0, hwcsysreqen, 0);
        if (r != 0) {
            FWK_LOG_ERR(
                "[PPU] powerdomain workaround error. sni-ppu%" PRIu32
                " off-process.",
                sni_ppu_info[i].no);
        }
    }

    /* ppu off-process check */
    for (i = 0; i < FWK_ARRAY_SIZE(sni_ppu_info); i++) {
        for (j = 0; j < PD_TIMEOUT_COUNT; j++) {
            if ((read_power_status(sni_ppu_info[i].no) & PPU_STATUS_MASK) ==
                PPU_PP_OFF)
                break;

            osDelay(PD_CHECK_CYCLE_MS);
        }
        if (j >= PD_TIMEOUT_COUNT) {
            FWK_LOG_ERR(
                "[PPU] powerdomain workaround error. sni-ppu%" PRIu32
                "status timeout",
                sni_ppu_info[i].no);
        }
    }

    /* reset */
    lpcm_sysoc_reset(RST_TYPE_WO_BUS, RST_PCIE0); /* PCIe#0 Link */
    lpcm_sysoc_reset(RST_TYPE_WO_BUS, RST_PCIE1); /* PCIe#1 Link */
    lpcm_sysoc_reset(RST_TYPE_ALL, RST_DDR);

    /* adb400 reset */
    lpcm_sysoc_reset(RST_TYPE_BUS, RST_PCIE0); /* PCIe#0 PowerOnReset */
    lpcm_sysoc_reset(RST_TYPE_BUS, RST_PCIE1); /* PCIe#1 PowerOnReset */
    lpcm_sysoc_reset(RST_TYPE_BUS, RST_PCIE_TOP);
    lpcm_sysoc_reset(RST_TYPE_BUS, RST_DMA);

    FWK_LOG_INFO("[PowerDomain] Socionext-PPU initialize end .");
}

static void sni_power_domain_on_mp(uint32_t dev_bitmap)
{
    uint32_t i, j, r;
    uint32_t pmu_bitmap;
    uint32_t transw_reg_bitmap[TRANSW_REG_NUM] = { 0 };

    FWK_LOG_INFO("[PowerDomain] PowerDomain All-ON start.");

    /* pmu cycle time */
    FOR_EACH_PMU_INFO(i)
    {
        if ((sni_pmu_info[i].dev_bitmap & dev_bitmap) == 0)
            continue;

        if (sni_pmu_info[i].offchip_pd == NOT_USE)
            continue;

        pmu_write_power_on_cycle(
            sni_pmu_info[i].offchip_pd,
            POWER_CONV_MS_TO_PWR_ON_CYC(sni_pmu_info[i].offchip_cycle_ms));
    }

    /* Make pmu and transaction-sw bitmap */
    pmu_bitmap = 0;
    FOR_EACH_PMU_INFO(i)
    {
        if ((sni_pmu_info[i].dev_bitmap & dev_bitmap) == 0)
            continue;

        pmu_bitmap |= (1 << sni_pmu_info[i].onchip_pd);

        if (sni_pmu_info[i].offchip_pd != NOT_USE)
            pmu_bitmap |= (1 << sni_pmu_info[i].offchip_pd);
    }
    FOR_EACH_TRANSW_INFO(i)
    {
        if ((sni_transw_info[i].dev_bitmap & dev_bitmap) == 0)
            continue;

        r = sni_transw_info[i].reg_no;
        transw_reg_bitmap[r] |= sni_transw_info[i].sw_bitmap;
    }

    /*
     * pmu all on
     * (without booting_on is 'false')
     */

    pmu_on(pmu_bitmap);

    /* waiting pmu-on */
    r = pmu_wait(pmu_bitmap, true);
    if (r != 0)
        FWK_LOG_ERR(
            "[PPU] sni-pmu timeout expected:(0x%08" PRIx32
            ") result: (0x%08" PRIx32 ").",
            pmu_bitmap,
            pmu_read_pd_power_status());

    /* adb400 reset clear */
    if ((dev_bitmap & DEV_BMAP_PCIE_BLK) != 0)
        lpcm_sysoc_reset_clear(RST_TYPE_BUS, RST_PCIE_TOP);

    if ((dev_bitmap & DEV_BMAP_PCIE0) != 0)
        lpcm_sysoc_reset_clear(
            RST_TYPE_BUS, RST_PCIE0); /* PCIe#0 PowerOnReset */

    if ((dev_bitmap & DEV_BMAP_PCIE1) != 0)
        lpcm_sysoc_reset_clear(
            RST_TYPE_BUS, RST_PCIE1); /* PCIe#1 PowerOnReset */

    if ((dev_bitmap & DEV_BMAP_DMA_BLK) != 0)
        lpcm_sysoc_reset_clear(RST_TYPE_BUS, RST_DMA);

    /* reset clear */
    if ((dev_bitmap & DEV_BMAP_PCIE0) != 0)
        lpcm_sysoc_reset_clear(RST_TYPE_WO_BUS, RST_PCIE0); /* PCIe#0 Link */

    if ((dev_bitmap & DEV_BMAP_PCIE1) != 0)
        lpcm_sysoc_reset_clear(RST_TYPE_WO_BUS, RST_PCIE1); /* PCIe#1 Link */

    if ((dev_bitmap & (DEV_BMAP_DDR0 | DEV_BMAP_DDR1)) != 0)
        lpcm_sysoc_reset_clear(RST_TYPE_ALL, RST_DDR);

    /* set TransactionSW */
    FWK_LOG_INFO("[PowerDomain] Opening transaction switch");

    FOR_EACH_TRANSW_REG(i)
    {
        FWK_LOG_INFO("[PowerDomain] Opening transaction switch + %" PRIu32, i);

        set_transactionsw_off(TRANSW_ADDR(i), transw_reg_bitmap[i]);

        FWK_LOG_INFO(
            "[PowerDomain] Finished opening transaction switch + %" PRIu32, i);
    }
    FWK_LOG_INFO("[PowerDomain] Finished transaction switch");

    FWK_LOG_INFO("[PowerDomain] PowerDomain All-ON finished.");

    /* sni-ppu all on */
    FOR_EACH_PPU_INFO(i)
    {
        if ((sni_ppu_info[i].dev_bitmap & dev_bitmap) == 0)
            continue;

        r = change_power_state(sni_ppu_info[i].no, PPU_PP_ON, 1, 1, 0);
        if (r != 0) {
            FWK_LOG_ERR(
                "[PPU] powerdomain error. sni-ppu%" PRIu32 " on-process.",
                sni_ppu_info[i].no);
        }
    }

    /* ppu on-process check */
    FOR_EACH_PPU_INFO(i)
    {
        if ((sni_ppu_info[i].dev_bitmap & dev_bitmap) == 0)
            continue;

        for (j = 0; j < PD_TIMEOUT_COUNT; j++) {
            if ((read_power_status(sni_ppu_info[i].no) & PPU_STATUS_MASK) ==
                PPU_PP_ON) {
                break;
            }
            osDelay(PD_CHECK_CYCLE_MS);
        }
        if (j >= PD_TIMEOUT_COUNT) {
            FWK_LOG_ERR(
                "[PPU] powerdomain error. sni-ppu%" PRIu32 " status timeout.",
                sni_ppu_info[i].no);
        }
    }
}

uint32_t pmu_wait(uint32_t pmu_bitmap, bool on)
{
    uint32_t i, status;

    for (i = 0; i < PD_TIMEOUT_COUNT; i++) {
        status = on ? ~pmu_read_pd_power_status() : pmu_read_pd_power_status();
        if ((status & pmu_bitmap) == 0)
            break;

        osDelay(PD_CHECK_CYCLE_MS);
    }

    return (status & pmu_bitmap);
}

static void power_domain_on(uint32_t dev_bitmap)
{
    sni_power_domain_on_mp(dev_bitmap);
}

void power_domain_coldboot(void)
{
    FWK_LOG_INFO("[SYSTEM] Initializing power domain");
    sni_power_domain_workaround_mp();
    power_domain_on(PD_PRESET_COLDBOOT);
}
