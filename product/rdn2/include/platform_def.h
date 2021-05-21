/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

/* PCIe ECAM and MMIO sizes */
#if (PLATFORM_VARIANT == 0)
#    define AP_PCIE_ECAM_SIZE_PER_RC  (64ULL * FWK_MIB)
#    define AP_PCIE_MMIOL_SIZE_PER_RC (128 * FWK_MIB)
#    define AP_PCIE_MMIOH_SIZE_PER_RC (64ULL * FWK_GIB)
#elif (PLATFORM_VARIANT == 1)
#    define AP_PCIE_ECAM_SIZE_PER_RC  (128ULL * FWK_MIB)
#    define AP_PCIE_MMIOL_SIZE_PER_RC (256 * FWK_MIB)
#    define AP_PCIE_MMIOH_SIZE_PER_RC (128ULL * FWK_GIB)
#endif

#endif /* PLATFORM_DEF_H */
