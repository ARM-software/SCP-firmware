/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <rcar_scmi.h>
#include <reset_devices.h>

#include <mod_scmi_reset_domain.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct mod_scmi_reset_domain_device agent_device_table_ospm[] = {
    {
        /* fdp1-1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FDP1_1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* fdp1-0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FDP1_0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* scif5 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SCIF5),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* scif4 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SCIF4),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* scif3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SCIF3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* scif1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SCIF1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* scif0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SCIF0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* msiof3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_MSIOF3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* msiof2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_MSIOF2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* msiof1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_MSIOF1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* msiof0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_MSIOF0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* sys-dmac2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SYS_DMAC2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* sys-dmac1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SYS_DMAC1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* sys-dmac0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SYS_DMAC0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* sceg-pub */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SCEG_PUB),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* cmt3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CMT3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* cmt2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CMT2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* cmt1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CMT1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* cmt0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CMT0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* tpu0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_TPU0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* scif2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SCIF2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* sdif3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SDIF3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* sdif2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SDIF2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* sdif1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SDIF1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* sdif0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SDIF0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* pcie1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_PCIE1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* pcie0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_PCIE0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* usb-dmac30 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_USB_DMAC30),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* usb3-if0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_USB3_IF0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* usb-dmac31 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_USB_DMAC31),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* usb-dmac0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_USB_DMAC0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* usb-dmac1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_USB_DMAC1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* rwdt */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_RWDT),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* intc-ex */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_INTC_EX),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* intc-ap */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_INTC_AP),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* audmac1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_AUDMAC1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* audmac0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_AUDMAC0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* drif31 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DRIF31),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* drif30 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DRIF30),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* drif21 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DRIF21),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* drif20 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DRIF20),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* drif11 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DRIF11),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* drif10 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DRIF10),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* drif01 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DRIF01),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* drif00 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DRIF00),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* hscif4 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_HSCIF4),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* hscif3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_HSCIF3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* hscif2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_HSCIF2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* hscif1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_HSCIF1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* hscif0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_HSCIF0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* thermal */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_THERMAL),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* pwm */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_PWM),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* fcpvd2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FCPVD2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* fcpvd1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FCPVD1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* fcpvd0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FCPVD0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* fcpvb1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FCPVB1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* fcpvb0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FCPVB0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* fcpvi1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FCPVI1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* fcpvi0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FCPVI0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* fcpf1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FCPF1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* fcpf0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FCPF0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* fcpcs */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_FCPCS),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vspd2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VSPD2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vspd1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VSPD1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vspd0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VSPD0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vspbc */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VSPBC),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vspbd */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VSPBD),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vspi1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VSPI1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vspi0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VSPI0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ehci3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_EHCI3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ehci2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_EHCI2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ehci1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_EHCI1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ehci0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_EHCI0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* hsusb */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_HSUSB),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* hsusb3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_HSUSB3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* cmm3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CMM3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* cmm2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CMM2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* cmm1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CMM1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* cmm0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CMM0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* csi20 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CSI20),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* csi41 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CSI41),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* csi40 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CSI40),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* du3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DU3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* du2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DU2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* du1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DU1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* du0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_DU0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* lvds */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_LVDS),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* hdmi1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_HDMI1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* hdmi0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_HDMI0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vin7 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VIN7),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vin6 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VIN6),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vin5 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VIN5),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vin4 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VIN4),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vin3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VIN3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vin2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VIN2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vin1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VIN1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* vin0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_VIN0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* etheravb */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_ETHERAVB),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* sata0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SATA0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* imr3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_IMR3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* imr2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_IMR2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* imr1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_IMR1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* imr0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_IMR0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* gpio7 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_GPIO7),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* gpio6 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_GPIO6),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* gpio5 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_GPIO5),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* gpio4 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_GPIO4),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* gpio3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_GPIO3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* gpio2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_GPIO2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* gpio1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_GPIO1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* gpio0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_GPIO0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* can-fd */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CAN_FD),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* can-if1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CAN_IF1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* can-if0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_CAN_IF0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* i2c6 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_I2C6),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* i2c5 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_I2C5),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* i2c-dvfs */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_I2C_DVFS),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* i2c4 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_I2C4),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* i2c3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_I2C3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* i2c2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_I2C2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* i2c1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_I2C1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* i2c0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_I2C0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ssi-all */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SSI_ALL),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ssi9 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SSI9),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ssi8 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SSI8),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ssi7 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SSI7),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ssi6 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SSI6),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ssi5 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SSI5),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ssi4 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SSI4),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ssi3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SSI3),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ssi2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SSI2),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ssi1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SSI1),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
    {
        /* ssi0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
            RESET_DEV_IDX_SSI0),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
            MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
};

static const struct mod_scmi_reset_domain_agent agent_table[] = {
    [SCMI_AGENT_ID_PSCI] = { /* No access */ },
    [SCMI_AGENT_ID_OSPM] = {
        .device_table = agent_device_table_ospm,
        .agent_domain_count = FWK_ARRAY_SIZE(agent_device_table_ospm),
    },
    [SCMI_AGENT_ID_VMM] = {
        .device_table = agent_device_table_ospm,
        .agent_domain_count = FWK_ARRAY_SIZE(agent_device_table_ospm),
    },
    [SCMI_AGENT_ID_VM1] = {
        .device_table = agent_device_table_ospm,
        .agent_domain_count = FWK_ARRAY_SIZE(agent_device_table_ospm),
    },
    [SCMI_AGENT_ID_VM2] = {
        .device_table = agent_device_table_ospm,
        .agent_domain_count = FWK_ARRAY_SIZE(agent_device_table_ospm),
    },
};

const struct fwk_module_config config_scmi_reset_domain = {
    .data = &((struct mod_scmi_reset_domain_config){
        .agent_table = agent_table,
        .agent_count = FWK_ARRAY_SIZE(agent_table),
    }),
};
