/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
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

#define IO_MACRO_ELEMENT_CONFIG(idx, ecam_size, mmiol_size, mmioh_size) \
    { \
        .name = "IO Macro " #idx, \
        .data = &((struct mod_pcie_integ_ctrl_config) { \
            .reg_base = PCIE_INTEG_CTRL_REG_BASE(idx), \
            .x16_ecam_mmio_mmap = { \
                .valid = true, \
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

static const struct fwk_element pcie_integ_ctrl_element_table[] = {
    IO_MACRO_ELEMENT_CONFIG(
        0,
        AP_PCIE_ECAM_SIZE_PER_RC,
        AP_PCIE_MMIOL_SIZE_PER_RC,
        AP_PCIE_MMIOH_SIZE_PER_RC),

    IO_MACRO_ELEMENT_CONFIG(
        1,
        AP_PCIE_ECAM_SIZE_PER_RC,
        AP_PCIE_MMIOL_SIZE_PER_RC,
        AP_PCIE_MMIOH_SIZE_PER_RC),

#if (PLATFORM_VARIANT == 0)
    IO_MACRO_ELEMENT_CONFIG(
        2,
        AP_PCIE_ECAM_SIZE_PER_RC,
        AP_PCIE_MMIOL_SIZE_PER_RC,
        AP_PCIE_MMIOH_SIZE_PER_RC),

    IO_MACRO_ELEMENT_CONFIG(
        3,
        AP_PCIE_ECAM_SIZE_PER_RC,
        AP_PCIE_MMIOL_SIZE_PER_RC,
        AP_PCIE_MMIOH_SIZE_PER_RC),
#endif

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
