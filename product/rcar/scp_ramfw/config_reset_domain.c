/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <reset_devices.h>
#include <reset_mstp_devices.h>

#include <mod_reset_domain.h>
#include <mod_rcar_reset.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

const struct fwk_element reset_domain_element_table[] = {
    [RESET_DEV_IDX_FDP1_1] = {
        .name = "fdp1-1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FDP1_1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_FDP1_0] = {
        .name = "fdp1-0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FDP1_0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SCIF5] = {
        .name = "scif5",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SCIF5),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SCIF4] = {
        .name = "scif4",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SCIF4),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SCIF3] = {
        .name = "scif3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SCIF3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SCIF1] = {
        .name = "scif1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SCIF1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SCIF0] = {
        .name = "scif0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SCIF0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_MSIOF3] = {
        .name = "msiof3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_MSIOF3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_MSIOF2] = {
        .name = "msiof2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_MSIOF2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_MSIOF1] = {
        .name = "msiof1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_MSIOF1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_MSIOF0] = {
        .name = "msiof0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_MSIOF0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SYS_DMAC2] = {
        .name = "sys-dmac2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SYS_DMAC2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SYS_DMAC1] = {
        .name = "sys-dmac1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SYS_DMAC1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SYS_DMAC0] = {
        .name = "sys-dmac0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SYS_DMAC0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SCEG_PUB] = {
        .name = "sceg-pub",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SCEG_PUB),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CMT3] = {
        .name = "cmt3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CMT3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CMT2] = {
        .name = "cmt2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CMT2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CMT1] = {
        .name = "cmt1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CMT1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CMT0] = {
        .name = "cmt0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CMT0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_TPU0] = {
        .name = "tpu0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_TPU0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SCIF2] = {
        .name = "scif2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SCIF2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SDIF3] = {
        .name = "sdif3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SDIF3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SDIF2] = {
        .name = "sdif2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SDIF2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SDIF1] = {
        .name = "sdif1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SDIF1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SDIF0] = {
        .name = "sdif0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SDIF0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_PCIE1] = {
        .name = "pcie1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_PCIE1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_PCIE0] = {
        .name = "pcie0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_PCIE0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_USB_DMAC30] = {
        .name = "usb-dmac30",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_USB_DMAC30),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_USB3_IF0] = {
        .name = "usb3-if0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_USB3_IF0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_USB_DMAC31] = {
        .name = "usb-dmac31",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_USB_DMAC31),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_USB_DMAC0] = {
        .name = "usb-dmac0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_USB_DMAC0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_USB_DMAC1] = {
        .name = "usb-dmac1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_USB_DMAC1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_RWDT] = {
        .name = "rwdt",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_RWDT),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_INTC_EX] = {
        .name = "intc-ex",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_INTC_EX),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_INTC_AP] = {
        .name = "intc-ap",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_INTC_AP),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_AUDMAC1] = {
        .name = "audmac1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_AUDMAC1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_AUDMAC0] = {
        .name = "audmac0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_AUDMAC0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DRIF31] = {
        .name = "drif31",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DRIF31),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DRIF30] = {
        .name = "drif30",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DRIF30),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DRIF21] = {
        .name = "drif21",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DRIF21),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DRIF20] = {
        .name = "drif20",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DRIF20),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DRIF11] = {
        .name = "drif11",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DRIF11),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DRIF10] = {
        .name = "drif10",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DRIF10),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DRIF01] = {
        .name = "drif01",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DRIF01),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DRIF00] = {
        .name = "drif00",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DRIF00),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_HSCIF4] = {
        .name = "hscif4",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_HSCIF4),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_HSCIF3] = {
        .name = "hscif3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_HSCIF3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_HSCIF2] = {
        .name = "hscif2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_HSCIF2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_HSCIF1] = {
        .name = "hscif1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_HSCIF1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_HSCIF0] = {
        .name = "hscif0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_HSCIF0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_THERMAL] = {
        .name = "thermal",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_THERMAL),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_PWM] = {
        .name = "pwm",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_PWM),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_FCPVD2] = {
        .name = "fcpvd2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FCPVD2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_FCPVD1] = {
        .name = "fcpvd1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FCPVD1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_FCPVD0] = {
        .name = "fcpvd0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FCPVD0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_FCPVB1] = {
        .name = "fcpvb1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FCPVB1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_FCPVB0] = {
        .name = "fcpvb0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FCPVB0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_FCPVI1] = {
        .name = "fcpvi1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FCPVI1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_FCPVI0] = {
        .name = "fcpvi0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FCPVI0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_FCPF1] = {
        .name = "fcpf1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FCPF1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_FCPF0] = {
        .name = "fcpf0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FCPF0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_FCPCS] = {
        .name = "fcpcs",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_FCPCS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VSPD2] = {
        .name = "vspd2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VSPD2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VSPD1] = {
        .name = "vspd1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VSPD1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VSPD0] = {
        .name = "vspd0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VSPD0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VSPBC] = {
        .name = "vspbc",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VSPBC),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VSPBD] = {
        .name = "vspbd",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VSPBD),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VSPI1] = {
        .name = "vspi1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VSPI1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VSPI0] = {
        .name = "vspi0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VSPI0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_EHCI3] = {
        .name = "ehci3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_EHCI3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_EHCI2] = {
        .name = "ehci2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_EHCI2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_EHCI1] = {
        .name = "ehci1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_EHCI1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_EHCI0] = {
        .name = "ehci0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_EHCI0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_HSUSB] = {
        .name = "hsusb",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_HSUSB),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_HSUSB3] = {
        .name = "hsusb3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_HSUSB3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CMM3] = {
        .name = "cmm3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CMM3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CMM2] = {
        .name = "cmm2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CMM2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CMM1] = {
        .name = "cmm1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CMM1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CMM0] = {
        .name = "cmm0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CMM0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CSI20] = {
        .name = "csi20",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CSI20),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CSI41] = {
        .name = "csi41",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CSI41),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CSI40] = {
        .name = "csi40",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CSI40),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DU3] = {
        .name = "du3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DU3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DU2] = {
        .name = "du2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DU2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DU1] = {
        .name = "du1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DU1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_DU0] = {
        .name = "du0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_DU0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_LVDS] = {
        .name = "lvds",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_LVDS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_HDMI1] = {
        .name = "hdmi1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_HDMI1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_HDMI0] = {
        .name = "hdmi0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_HDMI0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VIN7] = {
        .name = "vin7",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VIN7),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VIN6] = {
        .name = "vin6",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VIN6),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VIN5] = {
        .name = "vin5",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VIN5),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VIN4] = {
        .name = "vin4",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VIN4),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VIN3] = {
        .name = "vin3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VIN3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VIN2] = {
        .name = "vin2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VIN2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VIN1] = {
        .name = "vin1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VIN1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_VIN0] = {
        .name = "vin0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_VIN0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_ETHERAVB] = {
        .name = "etheravb",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_ETHERAVB),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SATA0] = {
        .name = "sata0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SATA0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_IMR3] = {
        .name = "imr3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_IMR3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_IMR2] = {
        .name = "imr2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_IMR2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_IMR1] = {
        .name = "imr1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_IMR1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_IMR0] = {
        .name = "imr0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_IMR0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_GPIO7] = {
        .name = "gpio7",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_GPIO7),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_GPIO6] = {
        .name = "gpio6",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_GPIO6),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_GPIO5] = {
        .name = "gpio5",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_GPIO5),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_GPIO4] = {
        .name = "gpio4",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_GPIO4),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_GPIO3] = {
        .name = "gpio3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_GPIO3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_GPIO2] = {
        .name = "gpio2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_GPIO2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_GPIO1] = {
        .name = "gpio1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_GPIO1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_GPIO0] = {
        .name = "gpio0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_GPIO0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CAN_FD] = {
        .name = "can-fd",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CAN_FD),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CAN_IF1] = {
        .name = "can-if1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CAN_IF1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_CAN_IF0] = {
        .name = "can-if0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_CAN_IF0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_I2C6] = {
        .name = "i2c6",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_I2C6),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_I2C5] = {
        .name = "i2c5",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_I2C5),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_I2C_DVFS] = {
        .name = "i2c-dvfs",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_I2C_DVFS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_I2C4] = {
        .name = "i2c4",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_I2C4),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_I2C3] = {
        .name = "i2c3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_I2C3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_I2C2] = {
        .name = "i2c2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_I2C2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_I2C1] = {
        .name = "i2c1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_I2C1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_I2C0] = {
        .name = "i2c0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_I2C0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SSI_ALL] = {
        .name = "ssi-all",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SSI_ALL),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SSI9] = {
        .name = "ssi9",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SSI9),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SSI8] = {
        .name = "ssi8",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SSI8),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SSI7] = {
        .name = "ssi7",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SSI7),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SSI6] = {
        .name = "ssi6",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SSI6),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SSI5] = {
        .name = "ssi5",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SSI5),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SSI4] = {
        .name = "ssi4",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SSI4),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SSI3] = {
        .name = "ssi3",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SSI3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SSI2] = {
        .name = "ssi2",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SSI2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SSI1] = {
        .name = "ssi1",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SSI1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_SSI0] = {
        .name = "ssi0",
        .data = &((struct mod_reset_domain_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_RESET,
                RESET_MSTP_DEV_IDX_SSI0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_RESET,
                MOD_RCAR_RESET_API_TYPE_CLOCK),
            .modes = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
                MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT,
        }),
    },
    [RESET_DEV_IDX_COUNT] = { 0 }, /* Termination description. */

};

static const struct mod_reset_domain_config reset_domain_config = {
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    .notification_id = FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
        MOD_RESET_DOMAIN_NOTIFICATION_AUTORESET),
#endif
};

static const struct fwk_element *reset_domain_get_element_table
    (fwk_id_t module_id)
{
    return reset_domain_element_table;
}

struct fwk_module_config config_reset_domain = {
    .data = &reset_domain_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(reset_domain_get_element_table),
};
