/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_DT_BINDINGS_H
#define JUNO_DT_BINDINGS_H

#include "juno_mmap.h"
#include "system_clock.h"

/* Level 0 */
#define POWER_DOMAIN_IDX_BIG_CPU0       0
#define POWER_DOMAIN_IDX_BIG_CPU1       1
#define POWER_DOMAIN_IDX_LITTLE_CPU0    2
#define POWER_DOMAIN_IDX_LITTLE_CPU1    3
#define POWER_DOMAIN_IDX_LITTLE_CPU2    4
#define POWER_DOMAIN_IDX_LITTLE_CPU3    5

    /* Level 1 */
#define POWER_DOMAIN_IDX_BIG_SSTOP      6
#define POWER_DOMAIN_IDX_LITTLE_SSTOP   7
#define POWER_DOMAIN_IDX_DBGSYS         8
#define POWER_DOMAIN_IDX_GPUTOP         9

    /* Level 2 */
#define POWER_DOMAIN_IDX_SYSTOP         10

#define POWER_DOMAIN_IDX_NONE           0xffffffff


/* from juno_clock_ids.h */ 
/* Juno clock indices */
#define JUNO_CLOCK_IDX_I2SCLK       0
#define JUNO_CLOCK_IDX_HDLCDREFCLK  1
#define JUNO_CLOCK_IDX_HDLCDPXL     2
#define JUNO_CLOCK_IDX_HDLCD0       3
#define JUNO_CLOCK_IDX_HDLCD1       4
#define JUNO_CLOCK_IDX_BIGCLK       5
#define JUNO_CLOCK_IDX_LITTLECLK    6
#define JUNO_CLOCK_IDX_GPUCLK       7
    
/* Juno CDCEL937 clock indices */
    /*
     * Generated clocks:
     * These clocks are generated through this driver.
     */
#define JUNO_CLOCK_CDCEL937_IDX_I2SCLK      0
#define JUNO_CLOCK_CDCEL937_IDX_HDLCDREFCLK 1
#define JUNO_CLOCK_CDCEL937_IDX_HDLCDPXL    2

    /*
     * Derived clocks:
     * These clocks are used only as reference for
     * the HDLCD module acting as a HAL.
     */
#define JUNO_CLOCK_CDCEL937_IDX_HDLCD0      3
#define JUNO_CLOCK_CDCEL937_IDX_HDLCD1      4

/* Juno HDLCD clock indices */
#define JUNO_CLOCK_HDLCD_IDX_HDLCD0         0
#define JUNO_CLOCK_HDLCD_IDX_HDLCD1         1

/* Juno SOC clock indices for the RAM firmware */
#define JUNO_CLOCK_SOC_RAM_IDX_BIGCLK       0
#define JUNO_CLOCK_SOC_RAM_IDX_LITTLECLK    1
#define JUNO_CLOCK_SOC_RAM_IDX_GPUCLK       2
#define JUNO_CLOCK_SOC_RAM_IDX_COUNT        3

/* mod_juno_cdcel937.h */
#define MOD_JUNO_CDCEL937_API_IDX_CLOCK_DRIVER 0
#define MOD_JUNO_CDCEL937_API_IDX_HDLCD_DRIVER 1

/* mod_juno_hdlcd.h */
#define MOD_JUNO_HDLCD_API_IDX_CLOCK_DRIVER             0
#define MOD_JUNO_HDLCD_API_IDX_HDLCD_DRIVER_RESPONSE    1
#define MOD_JUNO_HDLCD_API_COUNT                        2

/* mod_juno_soc_clock_ram.h */
#define MOD_JUNO_SOC_CLOCK_RAM_API_IDX_DRIVER   0
#define MOD_JUNO_SOC_CLOCK_RAM_API_IDX_COUNT    1

/* -- juno_power_domain.h -- */
/* Mask for the core valid power states */
#define JUNO_CORE_VALID_STATE_MASK (MOD_PD_STATE_OFF_MASK | \
                                    MOD_PD_STATE_ON_MASK | \
                                    MOD_PD_STATE_SLEEP_MASK)

/* Mask for the cluster valid power states */
#define JUNO_CLUSTER_VALID_STATE_MASK (MOD_PD_STATE_OFF_MASK | \
                                       MOD_PD_STATE_ON_MASK)

/* -- juno_ppu_idx.h -- */
    /* enum juno_ppu_idx  */
#define JUNO_PPU_DEV_IDX_BIG_CPU0       0
#define JUNO_PPU_DEV_IDX_BIG_CPU1       1
#define JUNO_PPU_DEV_IDX_BIG_SSTOP      2
#define JUNO_PPU_DEV_IDX_LITTLE_CPU0    3
#define JUNO_PPU_DEV_IDX_LITTLE_CPU1    4
#define JUNO_PPU_DEV_IDX_LITTLE_CPU2    5
#define JUNO_PPU_DEV_IDX_LITTLE_CPU3    6
#define JUNO_PPU_DEV_IDX_LITTLE_SSTOP   7
#define JUNO_PPU_DEV_IDX_GPUTOP         8
#define JUNO_PPU_DEV_IDX_SYSTOP         9
#define JUNO_PPU_DEV_IDX_DBGSYS         10
#define JUNO_PPU_DEV_IDX_COUNT          11

/* ---- */

/* -- mod_juno_ppu.h -- */
    /* enum mod_juno_ppu_api_idx */
#define MOD_JUNO_PPU_API_IDX_PD     0
#define MOD_JUNO_PPU_API_IDX_ROM    1
#define MOD_JUNO_PPU_API_COUNT      2

/* ---- */

/* -- juno_alarm_idx.h -- */
#define JUNO_ALARM_ELEMENT_IDX 0

    /* Alarm indices for XRP7724 */
    /* enum juno_xrp7724_alarm_idx */
#define JUNO_XRP7724_ALARM_IDX_PSU_VSYS 0
#define JUNO_XRP7724_ALARM_IDX_PSU_VBIG 1
#define JUNO_XRP7724_ALARM_IDX_PSU_VLITTLE 2
#define JUNO_XRP7724_ALARM_IDX_PSU_VGPU 3
#define JUNO_XRP7724_ALARM_IDX_COUNT 4


    /* Alarm indices for DVFS */
    /* enum juno_dvfs_alarm_idx */
#define JUNO_DVFS_ALARM_VLITTLE_IDX JUNO_XRP7724_ALARM_IDX_COUNT
#define JUNO_DVFS_ALARM_BIG_IDX (JUNO_DVFS_ALARM_VLITTLE_IDX + 1)
#define JUNO_DVFS_ALARM_GPU_IDX (JUNO_DVFS_ALARM_VLITTLE_IDX + 2)
#ifdef BUILD_HAS_FAST_CHANNELS
#define JUNO_SCMI_FAST_CHANNEL_IDX (JUNO_DVFS_ALARM_VLITTLE_IDX + 3)
#define JUNO_DVFS_ALARM_IDX_CNT (JUNO_DVFS_ALARM_VLITTLE_IDX + 4)
#else
#define JUNO_DVFS_ALARM_IDX_CNT  (JUNO_DVFS_ALARM_VLITTLE_IDX + 3)
#endif

    /* enum juno_misc_alarm_idx  */
#define JUNO_PPU_ALARM_IDX  JUNO_DVFS_ALARM_IDX_CNT
#define JUNO_THERMAL_ALARM_IDX (JUNO_PPU_ALARM_IDX + 1)
#ifdef BUILD_HAS_MOD_STATISTICS
#define JUNO_STATISTICS_ALARM_IDX (JUNO_PPU_ALARM_IDX + 2)
#define JUNO_SYSTEM_POWER_ALARM_IDX (JUNO_PPU_ALARM_IDX + 3)
#else
#define JUNO_SYSTEM_POWER_ALARM_IDX (JUNO_PPU_ALARM_IDX + 2)
#endif
#define JUNO_ALARM_IDX_COUNT (JUNO_SYSTEM_POWER_ALARM_IDX + 1)

/* ---- */

/* -- config_mock_psu.h -- */
/* Mock PSU element indices for Juno */
    /*enum mod_mock_psu_element_idx */
#define MOD_MOCK_PSU_ELEMENT_IDX_VSYS       0
#define MOD_MOCK_PSU_ELEMENT_IDX_VBIG       1
#define MOD_MOCK_PSU_ELEMENT_IDX_VLITTLE    2
#define MOD_MOCK_PSU_ELEMENT_IDX_VGPU       3
#define MOD_MOCK_PSU_ELEMENT_IDX_COUNT      4

/* ---- */

/* -- juno/include/fmw_cmsis.h */
    /* enum IRQn */
    /* cherry pick limited sub-set for now */
#define TIMREFCLK_IRQ 2
#define BIG_0_IRQ_WAKEUP_IRQ 24 /* big CPU0 IRQ Wakeup Request */
#define BIG_0_FIQ_WAKEUP_IRQ 25 /* big CPU0 FIQ Wakeup Request */
#define BIG_1_IRQ_WAKEUP_IRQ 26 /* big CPU1 IRQ Wakeup Request */
#define BIG_1_FIQ_WAKEUP_IRQ 27 /* big CPU1 FIQ Wakeup Request */
#define BIG_2_IRQ_WAKEUP_IRQ 28 /* big CPU2 IRQ Wakeup Request */
#define BIG_2_FIQ_WAKEUP_IRQ 29 /* big CPU2 FIQ Wakeup Request */
#define BIG_3_IRQ_WAKEUP_IRQ 30 /* big CPU3 IRQ Wakeup Request */
#define BIG_3_FIQ_WAKEUP_IRQ 31 /* big CPU3 FIQ Wakeup Request */
#define LITTLE_0_IRQ_WAKEUP_IRQ 32 /* LITTLE CPU0 IRQ Wakeup Request */
#define LITTLE_0_FIQ_WAKEUP_IRQ 33 /* LITTLE CPU0 FIQ Wakeup Request */
#define LITTLE_1_IRQ_WAKEUP_IRQ 34 /* LITTLE CPU1 IRQ Wakeup Request */
#define LITTLE_1_FIQ_WAKEUP_IRQ 35 /* LITTLE CPU1 FIQ Wakeup Request */
#define LITTLE_2_IRQ_WAKEUP_IRQ 36 /* LITTLE CPU2 IRQ Wakeup Request */
#define LITTLE_2_FIQ_WAKEUP_IRQ 37 /* LITTLE CPU2 FIQ Wakeup Request */
#define LITTLE_3_IRQ_WAKEUP_IRQ 38 /* LITTLE CPU3 IRQ Wakeup Request */
#define LITTLE_3_FIQ_WAKEUP_IRQ 39 /* LITTLE CPU3 FIQ Wakeup Request */
#define EXT_WAKEUP_IRQ 57 /* External GIC Interrupt Wakeup */
#define BIG_0_WARM_RST_REQ_IRQ 58 /* big CPU0 Warm Reset Request */
#define BIG_1_WARM_RST_REQ_IRQ 59 /* big CPU1 Warm Reset Request */
#define BIG_2_WARM_RST_REQ_IRQ 60 /* big CPU2 Warm Reset Request */
#define BIG_3_WARM_RST_REQ_IRQ 61 /* big CPU3 Warm Reset Request */
#define LITTLE_0_WARM_RST_REQ_IRQ 62 /* LITTLE CPU0 Warm Reset Request */
#define LITTLE_1_WARM_RST_REQ_IRQ 63 /* LITTLE CPU1 Warm Reset Request */
#define LITTLE_2_WARM_RST_REQ_IRQ 64 /* LITTLE CPU2 Warm Reset Request */
#define LITTLE_3_WARM_RST_REQ_IRQ 65 /* LITTLE CPU3 Warm Reset Request */

/* ---- */

#endif /* JUNO_DT_BINDINGS_H */
