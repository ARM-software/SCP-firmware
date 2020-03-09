/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_SYNQUACER_PD_H
#define INTERNAL_SYNQUACER_PD_H

#include <ppu_v0.h>

#include <internal/reset.h>

#define PD_TIMEOUT_MS 200
#define PD_CHECK_CYCLE_MS 1

#define OFFCHIP_CYCLE_MS_PCIE 10
#define OFFCHIP_CYCLE_MS_DDR 10

#define DEV_BMAP_CA53(cluster) (1 << (cluster + 16))
#define DEV_BMAP_CA53_ALL 0x0FFF0000
#define DEV_BMAP_DDR0 0x00001000
#define DEV_BMAP_DDR1 0x00002000
#define DEV_BMAP_PCIE_BLK 0x00000800
#define DEV_BMAP_PCIE0 0x00000100
#define DEV_BMAP_PCIE1 0x00000200

#define DEV_BMAP_SCB_AM 0x00000001
#define DEV_BMAP_SCB_SUBSYS 0x00000004
#define DEV_BMAP_DMA_BLK 0x00000010
//#define DEV_BMAP_SCB_TOP              0x00000002 /* can't change now */
//#define DEV_BMAP_FE_TOP               0x00000008 /* can't change now */

#define PD_PRESET_ALL 0x0FFF3B15
#define PD_PRESET_ALL_WO_CA53 (PD_PRESET_ALL & ~(DEV_BMAP_CA53_ALL))

#define PD_PRESET_COLDBOOT PD_PRESET_ALL

#define PD_PRESET_CM3_REBOOT PD_PRESET_ALL
#define PD_PRESET_AP_REBOOT \
    (PD_PRESET_CM3_REBOOT & ~(DEV_BMAP_SCB_SUBSYS | DEV_BMAP_PCIE_BLK))

#define PD_PRESET_AP_REBOOT_WO_PCIE0 (PD_PRESET_AP_REBOOT & ~(DEV_BMAP_PCIE0))
#define PD_PRESET_AP_REBOOT_WO_PCIE1 (PD_PRESET_AP_REBOOT & ~(DEV_BMAP_PCIE1))

#define PD_PRESET_SHUTDOWN PD_PRESET_ALL

#define NOT_USE (-1)

#define CLUSTER_PMU_NO(no) (19 + no)
#define OFFCHIP_PCIE_PMU_NO(no) (10 + no)
#define OFFCHIP_DDR_PMU_NO(no) (12 + no)
#define ONCHIP_PCIE_PMU_NO(no) (14 + no)
#define ONCHIP_DDR_PMU_NO(no) (16 + no)

#define SNI_PPU_INFO_MP                                                       \
    {                                                                         \
        {                                                                     \
            /* "DDRPHY_BLK_0" */ /* no                */ 2,                   \
                                 /* dev_bitmap        */ DEV_BMAP_DDR0,       \
                                 /* booting_force_off */ false,               \
                                 /* force_off         */ false,               \
        },                                                                    \
            {                                                                 \
                /* "DDRPHY_BLK_1" */ /* no                */ 3,               \
                                     /* dev_bitmap        */ DEV_BMAP_DDR1,   \
                                     /* booting_force_off */ false,           \
                                     /* force_off         */ false,           \
            },                                                                \
            {                                                                 \
                /* "PCIEB" */ /* no                */ 4,                      \
                              /* dev_bitmap        */ DEV_BMAP_PCIE_BLK,      \
                              /* booting_force_off */ false,                  \
                              /* force_off         */ false,                  \
            },                                                                \
            {                                                                 \
                /* "PCIE_BLK_0" */ /* no                */ 6,                 \
                                   /* dev_bitmap        */ DEV_BMAP_PCIE0,    \
                                   /* booting_force_off */ false,             \
                                   /* force_off         */ false,             \
            },                                                                \
            {                                                                 \
                /* "PCIE_BLK_1" */ /* no                */ 5,                 \
                                   /* dev_bitmap        */ DEV_BMAP_PCIE1,    \
                                   /* booting_force_off */ false,             \
                                   /* force_off         */ false,             \
            },                                                                \
            {                                                                 \
                /* "DMAB" */ /* no                */ 7,                       \
                             /* dev_bitmap        */ DEV_BMAP_DMA_BLK,        \
                             /* booting_force_off */ false,                   \
                             /* force_off         */ false,                   \
            },                                                                \
            {                                                                 \
                /* "SCB_A_MASTER" */ /* no                */ 8,               \
                                     /* dev_bitmap        */ DEV_BMAP_SCB_AM, \
                                     /* booting_force_off */ true,            \
                                     /* force_off         */ true,            \
            },                                                                \
    }

#define TRANSW_NO_PCIE 0
#define TRANSW_NO_SCBM 1
#define TRANSW_NO_CM3 2
#define TRANSW_NO_DDR 3

#define TRANSW_REG_NUM 4

#define SNI_TRANSACTIONSW_INFO_MP                                 \
    {                                                             \
        {                                                         \
            /* "PCIe#0" */ /* dev_bitmap */ DEV_BMAP_PCIE0,       \
                           /* reg_no     */ TRANSW_NO_PCIE,       \
                           /* sw_bitmap  */ 0x2,                  \
        },                                                        \
            {                                                     \
                /* "PCIe#1" */ /* dev_bitmap */ DEV_BMAP_PCIE1,   \
                               /* reg_no     */ TRANSW_NO_PCIE,   \
                               /* sw_bitmap  */ 0x1,              \
            },                                                    \
            {                                                     \
                /* "SCBM" */ /* dev_bitmap */ DEV_BMAP_SCB_AM,    \
                             /* reg_no     */ TRANSW_NO_SCBM,     \
                             /* sw_bitmap  */ 0x7,                \
            },                                                    \
            {                                                     \
                /* "CM3" */ /* dev_bitmap */ DEV_BMAP_SCB_SUBSYS, \
                            /* reg_no     */ TRANSW_NO_CM3,       \
                            /* sw_bitmap  */ 0x7,                 \
            },                                                    \
            {                                                     \
                /* "DDR#0" */ /* dev_bitmap */ DEV_BMAP_DDR0,     \
                              /* reg_no     */ TRANSW_NO_DDR,     \
                              /* sw_bitmap  */ 0x1,               \
            },                                                    \
            {                                                     \
                /* "DDR#1" */ /* dev_bitmap */ DEV_BMAP_DDR1,     \
                              /* reg_no     */ TRANSW_NO_DDR,     \
                              /* sw_bitmap  */ 0x2,               \
            },                                                    \
    }

#ifdef CONFIG_SCB_DIST_FIRM
#define PD_REBOOT_DEV_BITMAP PD_PRESET_AP_REBOOT_WO_PCIE0
#else /* CONFIG_SCB_DIST_FIRM */
#define PD_REBOOT_DEV_BITMAP PD_PRESET_AP_REBOOT
#endif /* CONFIG_SCB_DIST_FIRM */

#define SNI_PMU_INFO_MP \
    { \
        { \
            /* [0]: "SCB_AM" */ /* on_priority */ 0,    \
            /* dev_bitmap       */ DEV_BMAP_SCB_AM,     \
            /* onchip_pd        */ 18,                  \
            /* offchip_pd       */ NOT_USE,             \
            /* offchip_cycle_ms */ 0,                   \
        }, \
        { \
            /* [1]: "SCB_Subsys" */ /* on_priority */ 0, \
            /* dev_bitmap       */ DEV_BMAP_SCB_SUBSYS,  \
            /* onchip_pd        */ 31,                   \
            /* offchip_pd       */ NOT_USE,              \
            /* offchip_cycle_ms */ 0,                    \
        }, \
        { \
            /* [2]: "Cluster#0(A)" */ /* on_priority */ 1, \
            /* dev_bitmap       */ DEV_BMAP_CA53(0),       \
            /* onchip_pd        */ 19,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [3]: "Cluster#5(A)" */ /* on_priority */ 1, \
            /* dev_bitmap       */ DEV_BMAP_CA53(5),       \
            /* onchip_pd        */ 24,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [4]: "Cluster#9(A)" */ /* on_priority */ 1, \
            /* dev_bitmap       */ DEV_BMAP_CA53(9),       \
            /* onchip_pd        */ 28,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [5]: "Cluster#10(A)" */ /* on_priority */ 1,\
            /* dev_bitmap       */ DEV_BMAP_CA53(10),      \
            /* onchip_pd        */ 29,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [6]: "Cluster#1(B)" */ /* on_priority */ 2, \
            /* dev_bitmap       */ DEV_BMAP_CA53(1),       \
            /* onchip_pd        */ 20,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [7]: "Cluster#6(B)" */ /* on_priority */ 2, \
            /* dev_bitmap       */ DEV_BMAP_CA53(6),       \
            /* onchip_pd        */ 25,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [8]: "Cluster#7(B)" */ /* on_priority */ 2, \
            /* dev_bitmap       */ DEV_BMAP_CA53(7),       \
            /* onchip_pd        */ 26,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [9]: "Cluster#11(B)" */ /* on_priority */ 2,\
            /* dev_bitmap       */ DEV_BMAP_CA53(11),      \
            /* onchip_pd        */ 30,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [10]: "Cluster#2(C)" */ /* on_priority */ 3,\
            /* dev_bitmap       */ DEV_BMAP_CA53(2),       \
            /* onchip_pd        */ 21,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [11]: "Cluster#3(C)" */ /* on_priority */ 3,\
            /* dev_bitmap       */ DEV_BMAP_CA53(3),       \
            /* onchip_pd        */ 22,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [12]: "Cluster#4(C)" */ /* on_priority */ 3,\
            /* dev_bitmap       */ DEV_BMAP_CA53(4),       \
            /* onchip_pd        */ 23,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [13]: "Cluster#8(C)" */ /* on_priority */ 3,\
            /* dev_bitmap       */ DEV_BMAP_CA53(8),       \
            /* onchip_pd        */ 27,                     \
            /* offchip_pd       */ NOT_USE,                \
            /* offchip_cycle_ms */ 0,                      \
        }, \
        { \
            /* [14]: "DDR#0" */ /* on_priority */ 4,       \
            /* dev_bitmap       */ DEV_BMAP_DDR0,          \
            /* onchip_pd        */ 16,                     \
            /* offchip_pd       */ 12,                     \
            /* offchip_cycle_ms */ OFFCHIP_CYCLE_MS_DDR,   \
        }, \
        { \
            /* [15]: "DDR#1" */ /* on_priority */ 5,       \
            /* dev_bitmap       */ DEV_BMAP_DDR1,          \
            /* onchip_pd        */ 17,                     \
            /* offchip_pd       */ 13,                     \
            /* offchip_cycle_ms */ OFFCHIP_CYCLE_MS_DDR,   \
        },\
        { \
            /* [16]: "PCIe#0" */ /* on_priority */ 6,      \
            /* dev_bitmap       */ DEV_BMAP_PCIE0,         \
            /* onchip_pd        */ 15,                     \
            /* offchip_pd       */ 11,                     \
            /* offchip_cycle_ms */ OFFCHIP_CYCLE_MS_PCIE,  \
        }, \
        { \
            /* [17]: "PCIe#1" */ /* on_priority */ 7,      \
            /* dev_bitmap       */ DEV_BMAP_PCIE1,         \
            /* onchip_pd        */ 14,                     \
            /* offchip_pd       */ 10,                     \
            /* offchip_cycle_ms */ OFFCHIP_CYCLE_MS_PCIE,  \
        }, \
    }

int32_t get_cluster_pmu_no(struct ppu_v0_reg *ppu);
uint32_t pmu_wait(uint32_t pmu_bitmap, bool on);
void power_domain_coldboot(void);
void power_domain_reboot(void);

#endif /* INTERNAL_SYNQUACER_PD_H */
