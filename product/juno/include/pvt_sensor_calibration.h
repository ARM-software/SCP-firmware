/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PVT_SENSOR_CALIBRATION_H
#define PVT_SENSOR_CALIBRATION_H

#include "juno_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

#define SOC_PART_ID_LEN     7

/*
 * PVT Sensor Calibration register definitions.
 */
struct juno_pvt_calibration_reg {
    FWK_R   char        PART_ID_PREFIX;
    FWK_R   char        PART_ID[SOC_PART_ID_LEN];

    /* Temperature type sensors: PVTs */
    FWK_R   uint16_t    G1_S0_810MV_45C;
    FWK_R   uint16_t    G1_S0_810MV_85C;
    FWK_R   uint16_t    G1_S0_900MV_45C;
    FWK_R   uint16_t    G1_S0_900MV_85C;
    FWK_R   uint16_t    G2_S0_810MV_45C;
    FWK_R   uint16_t    G2_S0_810MV_85C;
    FWK_R   uint16_t    G2_S0_900MV_45C;
    FWK_R   uint16_t    G2_S0_900MV_85C;
    FWK_R   uint16_t    G2_S1_810MV_45C;
    FWK_R   uint16_t    G2_S1_810MV_85C;
    FWK_R   uint16_t    G2_S1_900MV_45C;
    FWK_R   uint16_t    G2_S1_900MV_85C;
    FWK_R   uint16_t    G3_S0_810MV_45C;
    FWK_R   uint16_t    G3_S0_810MV_85C;
    FWK_R   uint16_t    G3_S0_900MV_45C;
    FWK_R   uint16_t    G3_S0_900MV_85C;
    FWK_R   uint16_t    G4_S0_810MV_45C;
    FWK_R   uint16_t    G4_S0_810MV_85C;
    FWK_R   uint16_t    G4_S0_900MV_45C;
    FWK_R   uint16_t    G4_S0_900MV_85C;

    /* Voltage type sensors: Ring oscillators */
    FWK_R   uint16_t    G0_S1_810MV_85C;
    FWK_R   uint16_t    G0_S1_900MV_85C;
    FWK_R   uint16_t    G0_S9_810MV_85C;
    FWK_R   uint16_t    G0_S9_900MV_85C;
    FWK_R   uint16_t    G3_S1_810MV_85C;
    FWK_R   uint16_t    G3_S1_900MV_85C;
    FWK_R   uint16_t    G4_S1_810MV_85C;
    FWK_R   uint16_t    G4_S1_900MV_85C;
};

#define JUNO_PVT_CALIBRATION ((struct juno_pvt_calibration_reg *) \
                                 SENSOR_CALIBRATION_BASE)

#endif /* PVT_SENSOR_CALIBRATION_H */
