/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "platform_def.h"
#include "scp_css_mmap.h"
#include "scp_software_mmap.h"

#include <mod_pcie_integ_ctrl.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define RD_N2_PCIE_INTEG_START_ADDR(index, base, size) (base + (size * index))
#define RD_N2_PCIE_INTEG_END_ADDR(index, base, size) \
    (base + (size * (index + 1ULL)) - 1ULL)

#define IO_MACRO_ELEMENT_CONFIG( \
    idx, pcie_integ_ctrl_reg_base, ecam_size, mmiol_size, mmioh_size) \
    { \
        .name = "IO Macro " #idx, \
        .data = &((struct mod_pcie_integ_ctrl_config) { \
            .reg_base = pcie_integ_ctrl_reg_base, \
            .x16_ecam_mmio_mmap = { \
                .valid = true, \
                .allow_ns_access = true, \
                .ecam1_start_addr = RD_N2_PCIE_INTEG_START_ADDR(idx, \
                    AP_PCIE_ECAM_BASE, ecam_size), \
                .ecam1_end_addr = RD_N2_PCIE_INTEG_END_ADDR(idx, \
                    AP_PCIE_ECAM_BASE, ecam_size), \
                .mmiol_start_addr = RD_N2_PCIE_INTEG_START_ADDR (idx, \
                    AP_PCIE_MMIOL_BASE, mmiol_size), \
                .mmiol_end_addr = RD_N2_PCIE_INTEG_END_ADDR(idx, \
                    AP_PCIE_MMIOL_BASE, mmiol_size), \
                .mmioh_start_addr = RD_N2_PCIE_INTEG_START_ADDR(idx, \
                    AP_PCIE_MMIOH_BASE, mmioh_size), \
                .mmioh_end_addr = RD_N2_PCIE_INTEG_END_ADDR(idx, \
                    AP_PCIE_MMIOH_BASE, mmioh_size), \
            }, \
            .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, \
                CLOCK_IDX_INTERCONNECT), \
        }) \
    }

#if (PLATFORM_VARIANT == 0 || PLATFORM_VARIANT == 3)
#    define NON_PCIE_IO_MACRO_REG_BASE PCIE_INTEG_CTRL_REG_BASE(4)
#elif (PLATFORM_VARIANT == 1) || (PLATFORM_VARIANT == 2)
#    define NON_PCIE_IO_MACRO_REG_BASE PCIE_INTEG_CTRL_REG_BASE(1)
#endif

static const struct fwk_element pcie_integ_ctrl_element_table[] = {
    IO_MACRO_ELEMENT_CONFIG(
        0,
        PCIE_INTEG_CTRL_REG_BASE(0),
        AP_PCIE_ECAM_SIZE_PER_RC,
        AP_PCIE_MMIOL_SIZE_PER_RC,
        AP_PCIE_MMIOH_SIZE_PER_RC),

#if (PLATFORM_VARIANT == 0 || PLATFORM_VARIANT == 3)
    IO_MACRO_ELEMENT_CONFIG(
        1,
        PCIE_INTEG_CTRL_REG_BASE(1),
        AP_PCIE_ECAM_SIZE_PER_RC,
        AP_PCIE_MMIOL_SIZE_PER_RC,
        AP_PCIE_MMIOH_SIZE_PER_RC),

    IO_MACRO_ELEMENT_CONFIG(
        2,
        PCIE_INTEG_CTRL_REG_BASE(2),
        AP_PCIE_ECAM_SIZE_PER_RC,
        AP_PCIE_MMIOL_SIZE_PER_RC,
        AP_PCIE_MMIOH_SIZE_PER_RC),

    IO_MACRO_ELEMENT_CONFIG(
        3,
        PCIE_INTEG_CTRL_REG_BASE(3),
        AP_PCIE_ECAM_SIZE_PER_RC,
        AP_PCIE_MMIOL_SIZE_PER_RC,
        AP_PCIE_MMIOH_SIZE_PER_RC),
#endif
    {
        .name = "Non-PCIe IO Macro",
        .data = &((struct mod_pcie_integ_ctrl_config) {
            .reg_base = NON_PCIE_IO_MACRO_REG_BASE,
            /* PL011_UART0 (64 KB) and MEM0 (4 MB) */
            .x4_0_ecam_mmio_mmap = {
                .valid = true,
                .allow_ns_access = true,
                .mmioh_start_addr = 0x1080000000,
                .mmioh_end_addr = 0x108040FFFF,
            },
            /* PL330_DMA0_NS (64 KB) and PL330_DMA0_S (64 KB) */
            .x4_1_ecam_mmio_mmap = {
                .valid = true,
                .allow_ns_access = true,
                .mmioh_start_addr = 0x1090000000,
                .mmioh_end_addr = 0x109001FFFF,
            },
            /* PL011_UART1 (64 KB) */
            .x8_ecam_mmio_mmap = {
                .valid = true,
                .allow_ns_access = true,
                .mmioh_start_addr = 0x10A0000000,
                .mmioh_end_addr = 0x10A000FFFF,
            },
            /* PL330_DMA0_NS (64 KB), PL330_DMA0_S(64 KB) and MEM1 (4 MB) */
            .x16_ecam_mmio_mmap = {
                .valid = true,
                .allow_ns_access = true,
                .mmioh_start_addr = 0x10B0000000,
                .mmioh_end_addr = 0x10B041FFFF,
            },
            .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                CLOCK_IDX_INTERCONNECT),
        }),
    },

    { 0 }
};

static const struct fwk_element *pcie_integ_get_element_table(
    fwk_id_t module_id)
{
    return pcie_integ_ctrl_element_table;
}

struct fwk_module_config config_pcie_integ_ctrl = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(pcie_integ_get_element_table),
};
