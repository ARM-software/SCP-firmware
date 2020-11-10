/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clock_devices.h>
#include <clock_mstp_devices.h>
#include <clock_sd_devices.h>
#include <clock_ext_devices.h>
#include <config_power_domain.h>
#include <rcar_core.h>

#include <mod_clock.h>
#include <mod_rcar_clock.h>
#include <mod_power_domain.h>

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>

static struct fwk_element clock_dev_desc_table[] = {
    [CLOCK_DEV_IDX_BIG] = {
        .name = "CPU_GROUP_BIG",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_CLOCK, 0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_LITTLE] = {
        .name = "CPU_GROUP_LITTLE",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_CLOCK, 1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
   [CLOCK_DEV_IDX_ZTR] = {
        .name = "ztr",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_ZTR),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_ZTRD2] = {
        .name = "ztrd2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_ZTRD2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_ZT] = {
        .name = "zt",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_ZT),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_ZX] = {
        .name = "zx",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_ZX),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S0D1] = {
        .name = "s0d1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S0D1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S0D2] = {
        .name = "s0d2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S0D2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S0D3] = {
        .name = "s0d3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S0D3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S0D4] = {
        .name = "s0d4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S0D4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S0D6] = {
        .name = "s0d6",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S0D6),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S0D8] = {
        .name = "s0d8",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S0D8),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S0D12] = {
        .name = "s0d12",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S0D12),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S1D1] = {
        .name = "s1d1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S1D1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S1D2] = {
        .name = "s1d2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S1D2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S1D4] = {
        .name = "s1d4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S1D4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S2D1] = {
        .name = "s2d1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S2D1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S2D2] = {
        .name = "s2d2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S2D2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S2D4] = {
        .name = "s2d4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S2D4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S3D1] = {
        .name = "s3d1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S3D1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S3D2] = {
        .name = "s3d2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S3D2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S3D4] = {
        .name = "s3d4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S3D4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SD0] = {
        .name = "sd0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_SD0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SD1] = {
        .name = "sd1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_SD1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SD2] = {
        .name = "sd2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_SD2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SD3] = {
        .name = "sd3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_SD3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CL] = {
        .name = "cl",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_CL),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CR] = {
        .name = "cr",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_CR),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CP] = {
        .name = "cp",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_CP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CPEX] = {
        .name = "cpex",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_CPEX),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CANFD] = {
        .name = "canfd",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_CANFD),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CSI0] = {
        .name = "csi0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_CSI0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_MSO] = {
        .name = "mso",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_MSO),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_HDMI] = {
        .name = "hdmi",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_HDMI),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_OSC] = {
        .name = "osc",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_OSC),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_R] = {
        .name = "r",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_R),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S0] = {
        .name = "s0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S1] = {
        .name = "s1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S2] = {
        .name = "s2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_S3] = {
        .name = "s3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_S3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SDSRC] = {
        .name = "sdsrc",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_SDSRC),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_RINT] = {
        .name = "rint",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       CLOCK_SD_DEV_IDX_RINT),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FDP1_1] = {
        .name = "fdp1-1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FDP1_1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FDP1_0] = {
        .name = "fdp1-0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FDP1_0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCIF5] = {
        .name = "scif5",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCIF5),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCIF4] = {
        .name = "scif4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCIF4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCIF3] = {
        .name = "scif3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCIF3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCIF1] = {
        .name = "scif1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCIF1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCIF0] = {
        .name = "scif0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCIF0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_MSIOF3] = {
        .name = "msiof3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_MSIOF3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_MSIOF2] = {
        .name = "msiof2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_MSIOF2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_MSIOF1] = {
        .name = "msiof1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_MSIOF1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_MSIOF0] = {
        .name = "msiof0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_MSIOF0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SYS_DMAC2] = {
        .name = "sys-dmac2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SYS_DMAC2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SYS_DMAC1] = {
        .name = "sys-dmac1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SYS_DMAC1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SYS_DMAC0] = {
        .name = "sys-dmac0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SYS_DMAC0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCEG_PUB] = {
        .name = "sceg-pub",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCEG_PUB),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CMT3] = {
        .name = "cmt3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CMT3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CMT2] = {
        .name = "cmt2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CMT2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CMT1] = {
        .name = "cmt1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CMT1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CMT0] = {
        .name = "cmt0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CMT0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_TPU0] = {
        .name = "tpu0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_TPU0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCIF2] = {
        .name = "scif2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCIF2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SDIF3] = {
        .name = "sdif3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SDIF3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SDIF2] = {
        .name = "sdif2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SDIF2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SDIF1] = {
        .name = "sdif1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SDIF1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SDIF0] = {
        .name = "sdif0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SDIF0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_PCIE1] = {
        .name = "pcie1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_PCIE1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_PCIE0] = {
        .name = "pcie0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_PCIE0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_USB_DMAC30] = {
        .name = "usb-dmac30",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_USB_DMAC30),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_USB3_IF0] = {
        .name = "usb3-if0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_USB3_IF0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_USB_DMAC31] = {
        .name = "usb-dmac31",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_USB_DMAC31),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_USB_DMAC0] = {
        .name = "usb-dmac0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_USB_DMAC0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_USB_DMAC1] = {
        .name = "usb-dmac1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_USB_DMAC1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_RWDT] = {
        .name = "rwdt",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_RWDT),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_INTC_EX] = {
        .name = "intc-ex",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_INTC_EX),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_INTC_AP] = {
        .name = "intc-ap",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_INTC_AP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_AUDMAC1] = {
        .name = "audmac1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_AUDMAC1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_AUDMAC0] = {
        .name = "audmac0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_AUDMAC0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DRIF31] = {
        .name = "drif31",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DRIF31),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DRIF30] = {
        .name = "drif30",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DRIF30),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DRIF21] = {
        .name = "drif21",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DRIF21),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DRIF20] = {
        .name = "drif20",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DRIF20),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DRIF11] = {
        .name = "drif11",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DRIF11),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DRIF10] = {
        .name = "drif10",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DRIF10),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DRIF01] = {
        .name = "drif01",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DRIF01),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DRIF00] = {
        .name = "drif00",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DRIF00),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_HSCIF4] = {
        .name = "hscif4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_HSCIF4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_HSCIF3] = {
        .name = "hscif3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_HSCIF3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_HSCIF2] = {
        .name = "hscif2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_HSCIF2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_HSCIF1] = {
        .name = "hscif1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_HSCIF1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_HSCIF0] = {
        .name = "hscif0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_HSCIF0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_THERMAL] = {
        .name = "thermal",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_THERMAL),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_PWM] = {
        .name = "pwm",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_PWM),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FCPVD2] = {
        .name = "fcpvd2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FCPVD2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FCPVD1] = {
        .name = "fcpvd1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FCPVD1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FCPVD0] = {
        .name = "fcpvd0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FCPVD0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FCPVB1] = {
        .name = "fcpvb1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FCPVB1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FCPVB0] = {
        .name = "fcpvb0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FCPVB0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FCPVI1] = {
        .name = "fcpvi1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FCPVI1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FCPVI0] = {
        .name = "fcpvi0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FCPVI0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FCPF1] = {
        .name = "fcpf1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FCPF1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FCPF0] = {
        .name = "fcpf0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FCPF0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_FCPCS] = {
        .name = "fcpcs",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_FCPCS),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VSPD2] = {
        .name = "vspd2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VSPD2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VSPD1] = {
        .name = "vspd1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VSPD1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VSPD0] = {
        .name = "vspd0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VSPD0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VSPBC] = {
        .name = "vspbc",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VSPBC),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VSPBD] = {
        .name = "vspbd",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VSPBD),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VSPI1] = {
        .name = "vspi1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VSPI1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VSPI0] = {
        .name = "vspi0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VSPI0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_EHCI3] = {
        .name = "ehci3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_EHCI3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_EHCI2] = {
        .name = "ehci2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_EHCI2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_EHCI1] = {
        .name = "ehci1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_EHCI1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_EHCI0] = {
        .name = "ehci0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_EHCI0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_HSUSB] = {
        .name = "hsusb",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_HSUSB),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_HSUSB3] = {
        .name = "hsusb3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_HSUSB3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CMM3] = {
        .name = "cmm3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CMM3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CMM2] = {
        .name = "cmm2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CMM2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CMM1] = {
        .name = "cmm1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CMM1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CMM0] = {
        .name = "cmm0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CMM0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CSI20] = {
        .name = "csi20",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CSI20),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CSI41] = {
        .name = "csi41",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CSI41),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CSI40] = {
        .name = "csi40",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CSI40),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DU3] = {
        .name = "du3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DU3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DU2] = {
        .name = "du2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DU2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DU1] = {
        .name = "du1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DU1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DU0] = {
        .name = "du0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_DU0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_LVDS] = {
        .name = "lvds",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_LVDS),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_HDMI1] = {
        .name = "hdmi1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_HDMI1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_HDMI0] = {
        .name = "hdmi0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_HDMI0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VIN7] = {
        .name = "vin7",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VIN7),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VIN6] = {
        .name = "vin6",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VIN6),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VIN5] = {
        .name = "vin5",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VIN5),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VIN4] = {
        .name = "vin4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VIN4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VIN3] = {
        .name = "vin3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VIN3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VIN2] = {
        .name = "vin2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VIN2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VIN1] = {
        .name = "vin1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VIN1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VIN0] = {
        .name = "vin0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_VIN0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_ETHERAVB] = {
        .name = "etheravb",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_ETHERAVB),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SATA0] = {
        .name = "sata0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SATA0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_IMR3] = {
        .name = "imr3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_IMR3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_IMR2] = {
        .name = "imr2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_IMR2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_IMR1] = {
        .name = "imr1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_IMR1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_IMR0] = {
        .name = "imr0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_IMR0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_GPIO7] = {
        .name = "gpio7",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_GPIO7),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_GPIO6] = {
        .name = "gpio6",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_GPIO6),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_GPIO5] = {
        .name = "gpio5",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_GPIO5),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_GPIO4] = {
        .name = "gpio4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_GPIO4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_GPIO3] = {
        .name = "gpio3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_GPIO3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_GPIO2] = {
        .name = "gpio2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_GPIO2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_GPIO1] = {
        .name = "gpio1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_GPIO1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_GPIO0] = {
        .name = "gpio0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_GPIO0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CAN_FD] = {
        .name = "can-fd",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CAN_FD),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CAN_IF1] = {
        .name = "can-if1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CAN_IF1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CAN_IF0] = {
        .name = "can-if0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_CAN_IF0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_I2C6] = {
        .name = "i2c6",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_I2C6),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_I2C5] = {
        .name = "i2c5",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_I2C5),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_I2C_DVFS] = {
        .name = "i2c-dvfs",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_I2C_DVFS),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_I2C4] = {
        .name = "i2c4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_I2C4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_I2C3] = {
        .name = "i2c3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_I2C3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_I2C2] = {
        .name = "i2c2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_I2C2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_I2C1] = {
        .name = "i2c1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_I2C1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_I2C0] = {
        .name = "i2c0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_I2C0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SSI_ALL] = {
        .name = "ssi-all",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SSI_ALL),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SSI9] = {
        .name = "ssi9",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SSI9),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SSI8] = {
        .name = "ssi8",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SSI8),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SSI7] = {
        .name = "ssi7",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SSI7),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SSI6] = {
        .name = "ssi6",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SSI6),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SSI5] = {
        .name = "ssi5",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SSI5),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SSI4] = {
        .name = "ssi4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SSI4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SSI3] = {
        .name = "ssi3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SSI3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SSI2] = {
        .name = "ssi2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SSI2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SSI1] = {
        .name = "ssi1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SSI1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SSI0] = {
        .name = "ssi0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SSI0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_ALL] = {
        .name = "scu-all",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_ALL),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_DVC1] = {
        .name = "scu-dvc1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_DVC1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_DVC0] = {
        .name = "scu-dvc0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_DVC0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_CTU0_MIX1] = {
        .name = "scu-ctu1-mix1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_CTU0_MIX1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_CTU0_MIX0] = {
        .name = "scu-ctu0-mix0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_CTU0_MIX0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_SRC9] = {
        .name = "scu-src9",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_SRC9),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_SRC8] = {
        .name = "scu-src8",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_SRC8),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_SRC7] = {
        .name = "scu-src7",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_SRC7),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_SRC6] = {
        .name = "scu-src6",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_SRC6),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_SRC5] = {
        .name = "scu-src5",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_SRC5),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_SRC4] = {
        .name = "scu-src4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_SRC4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_SRC3] = {
        .name = "scu-src3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_SRC3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_SRC2] = {
        .name = "scu-src2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_SRC2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_SRC1] = {
        .name = "scu-src1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_SRC1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCU_SRC0] = {
        .name = "scu-src0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       CLOCK_MSTP_DEV_IDX_SCU_SRC0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_X12_CLK] = {
        .name = "x12_clk",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_X12_CLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_X21_CLK] = {
        .name = "x21_clk",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_X21_CLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_X22_CLK] = {
        .name = "x22_clk",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_X22_CLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_X23_CLK] = {
        .name = "x23_clk",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_X23_CLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_AUDIO_CLKOUT] = {
        .name = "audio_clkout",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_AUDIO_CLKOUT),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_AUDIO_CLK_A] = {
        .name = "audio_clk_a",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_AUDIO_CLK_A),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_AUDIO_CLK_C] = {
        .name = "audio_clk_c",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_AUDIO_CLK_C),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_CAN_CLK] = {
        .name = "can_clk",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_CAN_CLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_PCIE_BUS_CLK] = {
        .name = "pcie_bus_clk",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_PCIE_BUS_CLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_SCIF_CLK] = {
        .name = "scif_clk",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_SCIF_CLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_USB3S0_CLK] = {
        .name = "usb3s0_clk",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_USB3S0_CLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_USB_EXTAL_CLK] = {
        .name = "usb_extal_clk",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       CLOCK_EXT_DEV_IDX_USB_EXTAL_CLK),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_EXT_CLOCK,
                                       MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *clock_get_dev_desc_table(fwk_id_t module_id)
{
    unsigned int i;
    unsigned int core_count;
    struct mod_clock_dev_config *dev_config;

    core_count = rcar_core_get_count();

    /* Configure all clocks to respond to changes in SYSTOP power state */
    for (i = 0; i < CLOCK_DEV_IDX_COUNT; i++) {
        dev_config =
            (struct mod_clock_dev_config *)clock_dev_desc_table[i].data;
        dev_config->pd_source_id = FWK_ID_ELEMENT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            CONFIG_POWER_DOMAIN_CHILD_COUNT + core_count);
    }
    return clock_dev_desc_table;
}

struct fwk_module_config config_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(clock_get_dev_desc_table),
    .data = &((struct mod_clock_config){
        .pd_transition_notification_id = FWK_ID_NOTIFICATION_INIT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            MOD_PD_NOTIFICATION_IDX_POWER_STATE_TRANSITION),
        .pd_pre_transition_notification_id = FWK_ID_NOTIFICATION_INIT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            MOD_PD_NOTIFICATION_IDX_POWER_STATE_PRE_TRANSITION),
    }),
};
