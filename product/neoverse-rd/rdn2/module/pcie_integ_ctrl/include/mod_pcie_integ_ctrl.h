/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     PCIe Integration Control register intialization module for Neoverse
 *     reference design platforms.
 */

#ifndef MOD_PCIE_INTEG_CTRL_H
#define MOD_PCIE_INTEG_CTRL_H

#include <fwk_id.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupModulePCIeIntegCtrl PCIe integ ctrl
 *
 * \brief PCIe integration control module.
 *
 * \details This module implements PCIe integration control register
 *    initialization.
 * \{
 */

/*!
 * \brief ECAM1, 32-bit and 64-bit MMIO space start and end address
 */
struct pcie_ecam_mmio_mmap {
    /*! Valid flag to indicate if the mapping is valid to program */
    bool valid;

    /*! Allow non-secure access
     *
     * By default only secure accesses are allowed. Set this to true to allow
     * non-secure access as well.
     */
    bool allow_ns_access;

    /*! ECAM1 start address */
    uint64_t ecam1_start_addr;

    /*! ECAM1 end address */
    uint64_t ecam1_end_addr;

    /*! 32-bit MMIO space start address */
    uint64_t mmiol_start_addr;

    /*! 32-bit MMIO space end address */
    uint64_t mmiol_end_addr;

    /*! 64-bit MMIO space start address */
    uint64_t mmioh_start_addr;

    /*! 64-bit MMIO space end address */
    uint64_t mmioh_end_addr;
};

/*!
 * \brief PCIe Integration control registers configuration data which contains
 * mapping of x4_0, x4_1, x8 and x16.
 */
struct mod_pcie_integ_ctrl_config {
    /*! Register base of IO Macro instance */
    uintptr_t reg_base;

    /*! ECAM and MMIO memory map for x4_0 */
    struct pcie_ecam_mmio_mmap x4_0_ecam_mmio_mmap;

    /*! ECAM and MMIO memory map for x4_1 */
    struct pcie_ecam_mmio_mmap x4_1_ecam_mmio_mmap;

    /*! ECAM and MMIO memory map for x8 */
    struct pcie_ecam_mmio_mmap x8_ecam_mmio_mmap;

    /*! ECAM and MMIO memory map for x16 */
    struct pcie_ecam_mmio_mmap x16_ecam_mmio_mmap;

    /*! Identifier of the clock this module depends on */
    fwk_id_t clock_id;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_PCIE_INTEG_CTRL_H */
