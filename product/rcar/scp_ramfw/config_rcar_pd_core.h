/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_RCAR_PD_CORE_H
#define CONFIG_RCAR_PD_CORE_H

enum rcar_pd_core_element_idx {
    RCAR_PD_CORE_ELEMENT_IDX_CPU0,
    RCAR_PD_CORE_ELEMENT_IDX_CPU1,
    RCAR_PD_CORE_ELEMENT_IDX_CPU2,
    RCAR_PD_CORE_ELEMENT_IDX_CPU3,
    RCAR_PD_CORE_ELEMENT_IDX_CPU4,
    RCAR_PD_CORE_ELEMENT_IDX_CPU5,
    RCAR_PD_CORE_ELEMENT_IDX_CPU6,
    RCAR_PD_CORE_ELEMENT_IDX_CPU7,
};

enum rcar_pd_cluster_element_idx {
    RCAR_PD_CORE_ELEMENT_IDX_CLU0,
    RCAR_PD_CORE_ELEMENT_IDX_CLU1
};

#define CPU_CORE_MAX 8
#define CPU_CLUSTER_MAX 2

#endif /* CONFIG_RCAR_PD_CORE_H */
