/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <mod_rcar_reset.h>

const struct fwk_element rcar_reset_element_table[] = {
    {
        .name = "fdp1-1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR1,
            .bit = 18,
        }),
    },
    {
        .name = "fdp1-0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR1,
            .bit = 19,
        }),
    },
    {
        .name = "scif5",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 2,
        }),
    },
    {
        .name = "scif4",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 3,
        }),
    },
    {
        .name = "scif3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 4,
        }),
    },
    {
        .name = "scif1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 6,
        }),
    },
    {
        .name = "scif0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 7,
        }),
    },
    {
        .name = "msiof3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 8,
        }),
    },
    {
        .name = "msiof2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 9,
        }),
    },
    {
        .name = "msiof1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 10,
        }),
    },
    {
        .name = "msiof0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 11,
        }),
    },
    {
        .name = "sys-dmac2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 17,
        }),
    },
    {
        .name = "sys-dmac1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 18,
        }),
    },
    {
        .name = "sys-dmac0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 19,
        }),
    },
    {
        .name = "sceg-pub",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR2,
            .bit = 29,
        }),
    },
    {
        .name = "cmt3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 0,
        }),
    },
    {
        .name = "cmt2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 1,
        }),
    },
    {
        .name = "cmt1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 2,
        }),
    },
    {
        .name = "cmt0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 3,
        }),
    },
    {
        .name = "tpu0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 4,
        }),
    },
    {
        .name = "scif2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 10,
        }),
    },
    {
        .name = "sdif3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 11,
        }),
    },
    {
        .name = "sdif2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 12,
        }),
    },
    {
        .name = "sdif1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 13,
        }),
    },
    {
        .name = "sdif0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 14,
        }),
    },
    {
        .name = "pcie1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 18,
        }),
    },
    {
        .name = "pcie0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 19,
        }),
    },
    {
        .name = "usb-dmac30",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 26,
        }),
    },
    {
        .name = "usb3-if0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 28,
        }),
    },
    {
        .name = "usb-dmac31",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 29,
        }),
    },
    {
        .name = "usb-dmac0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 30,
        }),
    },
    {
        .name = "usb-dmac1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR3,
            .bit = 31,
        }),
    },
    {
        .name = "rwdt",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR4,
            .bit = 2,
        }),
    },
    {
        .name = "intc-ex",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR4,
            .bit = 7,
        }),
    },
    {
        .name = "intc-ap",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR4,
            .bit = 8,
        }),
    },
    {
        .name = "audmac1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 1,
        }),
    },
    {
        .name = "audmac0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 2,
        }),
    },
    {
        .name = "drif31",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 8,
        }),
    },
    {
        .name = "drif30",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 9,
        }),
    },
    {
        .name = "drif21",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 10,
        }),
    },
    {
        .name = "drif20",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 11,
        }),
    },
    {
        .name = "drif11",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 12,
        }),
    },
    {
        .name = "drif10",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 13,
        }),
    },
    {
        .name = "drif01",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 14,
        }),
    },
    {
        .name = "drif00",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 15,
        }),
    },
    {
        .name = "hscif4",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 16,
        }),
    },
    {
        .name = "hscif3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 17,
        }),
    },
    {
        .name = "hscif2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 18,
        }),
    },
    {
        .name = "hscif1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 19,
        }),
    },
    {
        .name = "hscif0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 20,
        }),
    },
    {
        .name = "thermal",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 22,
        }),
    },
    {
        .name = "pwm",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR5,
            .bit = 23,
        }),
    },
    {
        .name = "fcpvd2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 1,
        }),
    },
    {
        .name = "fcpvd1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 2,
        }),
    },
    {
        .name = "fcpvd0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 3,
        }),
    },
    {
        .name = "fcpvb1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 6,
        }),
    },
    {
        .name = "fcpvb0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 7,
        }),
    },
    {
        .name = "fcpvi1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 10,
        }),
    },
    {
        .name = "fcpvi0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 11,
        }),
    },
    {
        .name = "fcpf1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 14,
        }),
    },
    {
        .name = "fcpf0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 15,
        }),
    },
    {
        .name = "fcpcs",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 19,
        }),
    },
    {
        .name = "vspd2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 21,
        }),
    },
    {
        .name = "vspd1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 22,
        }),
    },
    {
        .name = "vspd0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 23,
        }),
    },
    {
        .name = "vspbc",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 24,
        }),
    },
    {
        .name = "vspbd",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 26,
        }),
    },
    {
        .name = "vspi1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 30,
        }),
    },
    {
        .name = "vspi0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR6,
            .bit = 31,
        }),
    },
    {
        .name = "ehci3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 0,
        }),
    },
    {
        .name = "ehci2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 1,
        }),
    },
    {
        .name = "ehci1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 2,
        }),
    },
    {
        .name = "ehci0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 3,
        }),
    },
    {
        .name = "hsusb",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 4,
        }),
    },
    {
        .name = "hsusb3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 5,
        }),
    },
    {
        .name = "cmm3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 8,
        }),
    },
    {
        .name = "cmm2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 9,
        }),
    },
    {
        .name = "cmm1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 10,
        }),
    },
    {
        .name = "cmm0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 11,
        }),
    },
    {
        .name = "csi20",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 14,
        }),
    },
    {
        .name = "csi41",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 15,
        }),
    },
    {
        .name = "csi40",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 16,
        }),
    },
    {
        .name = "du3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 21,
        }),
    },
    {
        .name = "du2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 22,
        }),
    },
    {
        .name = "du1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 23,
        }),
    },
    {
        .name = "du0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 24,
        }),
    },
    {
        .name = "lvds",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 27,
        }),
    },
    {
        .name = "hdmi1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 28,
        }),
    },
    {
        .name = "hdmi0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR7,
            .bit = 29,
        }),
    },
    {
        .name = "vin7",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 4,
        }),
    },
    {
        .name = "vin6",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 5,
        }),
    },
    {
        .name = "vin5",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 6,
        }),
    },
    {
        .name = "vin4",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 7,
        }),
    },
    {
        .name = "vin3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 8,
        }),
    },
    {
        .name = "vin2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 9,
        }),
    },
    {
        .name = "vin1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 10,
        }),
    },
    {
        .name = "vin0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 11,
        }),
    },
    {
        .name = "etheravb",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 12,
        }),
    },
    {
        .name = "sata0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 15,
        }),
    },
    {
        .name = "imr3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 20,
        }),
    },
    {
        .name = "imr2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 21,
        }),
    },
    {
        .name = "imr1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 22,
        }),
    },
    {
        .name = "imr0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR8,
            .bit = 23,
        }),
    },
    {
        .name = "gpio7",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 5,
        }),
    },
    {
        .name = "gpio6",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 6,
        }),
    },
    {
        .name = "gpio5",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 7,
        }),
    },
    {
        .name = "gpio4",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 8,
        }),
    },
    {
        .name = "gpio3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 9,
        }),
    },
    {
        .name = "gpio2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 10,
        }),
    },
    {
        .name = "gpio1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 11,
        }),
    },
    {
        .name = "gpio0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 12,
        }),
    },
    {
        .name = "can-fd",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 14,
        }),
    },
    {
        .name = "can-if1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 15,
        }),
    },
    {
        .name = "can-if0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 16,
        }),
    },
    {
        .name = "i2c6",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 18,
        }),
    },
    {
        .name = "i2c5",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 19,
        }),
    },
    {
        .name = "i2c-dvfs",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 26,
        }),
    },
    {
        .name = "i2c4",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 27,
        }),
    },
    {
        .name = "i2c3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 28,
        }),
    },
    {
        .name = "i2c2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 29,
        }),
    },
    {
        .name = "i2c1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 30,
        }),
    },
    {
        .name = "i2c0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR9,
            .bit = 31,
        }),
    },
    {
        .name = "ssi-all",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 5,
        }),
    },
    {
        .name = "ssi9",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 6,
        }),
    },
    {
        .name = "ssi8",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 7,
        }),
    },
    {
        .name = "ssi7",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 8,
        }),
    },
    {
        .name = "ssi6",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 9,
        }),
    },
    {
        .name = "ssi5",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 10,
        }),
    },
    {
        .name = "ssi4",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 11,
        }),
    },
    {
        .name = "ssi3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 12,
        }),
    },
    {
        .name = "ssi2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 13,
        }),
    },
    {
        .name = "ssi1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 14,
        }),
    },
    {
        .name = "ssi0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 15,
        }),
    },
    {
        .name = "scu-all",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 17,
        }),
    },
    {
        .name = "scu-dvc1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 18,
        }),
    },
    {
        .name = "scu-dvc0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 19,
        }),
    },
    {
        .name = "scu-ctu1-mix1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 20,
        }),
    },
    {
        .name = "scu-ctu0-mix0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 21,
        }),
    },
    {
        .name = "scu-src9",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 22,
        }),
    },
    {
        .name = "scu-src8",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 23,
        }),
    },
    {
        .name = "scu-src7",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 24,
        }),
    },
    {
        .name = "scu-src6",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 25,
        }),
    },
    {
        .name = "scu-src5",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 26,
        }),
    },
    {
        .name = "scu-src4",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 27,
        }),
    },
    {
        .name = "scu-src3",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 28,
        }),
    },
    {
        .name = "scu-src2",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 29,
        }),
    },
    {
        .name = "scu-src1",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 30,
        }),
    },
    {
        .name = "scu-src0",
        .data = &((struct mod_rcar_reset_dev_config) {
            .control_reg = CPG_SMSTPCR10,
            .bit = 31,
        }),
    },
    { 0 }, /* Termination description. */
};

static const struct mod_reset_domain_config rcar_reset_config = {
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    .notification_id = FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                            MOD_RESET_DOMAIN_NOTIFICATION_AUTORESET),
#endif
};

static const struct fwk_element *rcar_reset_get_element_table
    (fwk_id_t module_id)
{
    return rcar_reset_element_table;
}

struct fwk_module_config config_rcar_reset = {
    .data = &rcar_reset_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(rcar_reset_get_element_table),
};
