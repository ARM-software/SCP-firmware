/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_SCMI_ID_H
#define RCAR_SCMI_ID_H

/* SCMI PowerDomain Domain indexes */
enum rcar_scmi_pd_domain_id {
    PD_RCAR_CLUS0CORE0 = 0,
    PD_RCAR_CLUS0CORE1,
    PD_RCAR_CLUS0CORE2,
    PD_RCAR_CLUS0CORE3,
    PD_RCAR_CLUS1CORE0,
    PD_RCAR_CLUS1CORE1,
    PD_RCAR_CLUS1CORE2,
    PD_RCAR_CLUS1CORE3,
    PD_RCAR_CLUSTER0,
    PD_RCAR_CLUSTER1,
    PD_RCAR_A3IR,
    PD_RCAR_3DGE,
    PD_RCAR_3DGD,
    PD_RCAR_3DGC,
    PD_RCAR_3DGB,
    PD_RCAR_3DGA,
    PD_RCAR_A2VC1,
    PD_RCAR_A3VC,
    PD_RCAR_CR7,
    PD_RCAR_A3VP,
    PD_RCAR_PMIC_DDR_BKUP,
    PD_RCAR_ALWAYS_ON,
    PD_RCAR_COUNT,
};

/* SCMI Clock indexes */
enum rcar_scmi_clock_id {
    CLOCK_RCAR_COUNT,
};

/* SCMI Sensor indexes */
enum rcar_scmi_sensor_id {
    SENSOR_RCAR_COUNT,
};

#endif /* RCAR_SCMI_ID_H */
