/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_RCAR_PD_SYSC_H
#define CONFIG_RCAR_PD_SYSC_H

enum rcar_pd_sysc_element_idx {
    RCAR_PD_SYSC_ELEMENT_IDX_A3IR,
    RCAR_PD_SYSC_ELEMENT_IDX_3DGE,
    RCAR_PD_SYSC_ELEMENT_IDX_3DGD,
    RCAR_PD_SYSC_ELEMENT_IDX_3DGC,
    RCAR_PD_SYSC_ELEMENT_IDX_3DGB,
    RCAR_PD_SYSC_ELEMENT_IDX_3DGA,
    RCAR_PD_SYSC_ELEMENT_IDX_A2VC1,
    RCAR_PD_SYSC_ELEMENT_IDX_A3VC,
    RCAR_PD_SYSC_ELEMENT_IDX_CR7,
    RCAR_PD_SYSC_ELEMENT_IDX_A3VP,
    RCAR_PD_SYSC_ELEMENT_IDX_COUNT
};

#define R8A7795_PD_CA57_CPU0    0
#define R8A7795_PD_CA57_CPU1    1
#define R8A7795_PD_CA57_CPU2    2
#define R8A7795_PD_CA57_CPU3    3
#define R8A7795_PD_CA53_CPU0    5
#define R8A7795_PD_CA53_CPU1    6
#define R8A7795_PD_CA53_CPU2    7
#define R8A7795_PD_CA53_CPU3    8
#define R8A7795_PD_A3VP         9
#define R8A7795_PD_CA57_SCU     12
#define R8A7795_PD_CR7          13
#define R8A7795_PD_A3VC         14
#define R8A7795_PD_3DG_A        17
#define R8A7795_PD_3DG_B        18
#define R8A7795_PD_3DG_C        19
#define R8A7795_PD_3DG_D        20
#define R8A7795_PD_CA53_SCU     21
#define R8A7795_PD_3DG_E        22
#define R8A7795_PD_A3IR         24
#define R8A7795_PD_A2VC0        25 /* ES1.x only */
#define R8A7795_PD_A2VC1        26

#endif /* CONFIG_RCAR_PD_SYSC_H */
