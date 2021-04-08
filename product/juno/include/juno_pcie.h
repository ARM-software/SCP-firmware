/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions specific to the Juno PCIe implementation
 */

#ifndef JUNO_PCIE_H
#define JUNO_PCIE_H

#define PCI_BRIDGE_REVISION_ID  1U
#define PCI_BRIDGE_CLASS        UINT32_C(0x00060400)

/* Juno-specific memory translation regions */
#define PCI_ECAM_BASE                  UINT32_C(0x40000000)
#define PCI_ECAM_SIZE                  UINT32_C(0x10000000)
#define PCI_IO_BASE                    UINT32_C(0x5F800000)
#define PCI_IO_SIZE                    UINT32_C(0x00800000)
#define PCI_MEM32_BASE                 UINT32_C(0x50000000)
#define PCI_MEM32_SIZE                 UINT32_C(0x08000000)
#define PCI_MEM64_BASE                 UINT64_C(0x4000000000)
#define PCI_MEM64_SIZE                 UINT64_C(0x100000000)
#define GICV2M_MSI_BASE                UINT32_C(0x2C1C0000)
#define GICV2M_MSI_SIZE                UINT32_C(0x00040000)
#define JUNO_SYSTEM_MEMORY_BASE        UINT32_C(0x80000000)
#define JUNO_SYSTEM_MEMORY_SIZE        UINT32_C(0x7F000000)
#define JUNO_EXTRA_SYSTEM_MEMORY_BASE  UINT64_C(0x0880000000)
#define JUNO_EXTRA_SYSTEM_MEMORY_SIZE  UINT64_C(0x180000000)

#endif /* JUNO_PCIE_H */
