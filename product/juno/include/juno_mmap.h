/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *    Memory map for the Juno product.
 */

#ifndef JUNO_MMAP_H
#define JUNO_MMAP_H

#include "system_mmap.h"

#include <stdint.h>

#define I2C_BASE                    (VENDOR_EXTENSION_BASE + 0x00000000)
#define PVT_AON_BASE                (VENDOR_EXTENSION_BASE + 0x00001000)
#define SCC_BASE                    (VENDOR_EXTENSION_BASE + 0x1FEFF000)

#define V2M_SYS_REG_BASE            (EXTERNAL_DEV_BASE + 0x1C010000)
#define SENSOR_CALIBRATION_BASE     (EXTERNAL_DEV_BASE + 0x1C010130)
#define FPGA_UART1_BASE             (EXTERNAL_DEV_BASE + 0x1C0A0000)
#define NIC400_GPV_CCI_PCIE_BASE    (EXTERNAL_DEV_BASE + 0x2C091000)
#define NIC400_GPV_CCI_GPU_BASE     (EXTERNAL_DEV_BASE + 0x2C093000)
#define NIC400_GPV_CCI_LITTLE_BASE  (EXTERNAL_DEV_BASE + 0x2C094000)
#define NIC400_GPV_CCI_BIG_BASE     (EXTERNAL_DEV_BASE + 0x2C095000)
#define NIC400_BASE                 (EXTERNAL_RAM_BASE + 0x3FD00000)
#define NIC400_GPV_HDLCD0_BASE      (EXTERNAL_RAM_BASE + 0x3FD44000)
#define NIC400_GPV_HDLCD1_BASE      (EXTERNAL_RAM_BASE + 0x3FD45000)
#define PVT_STDCELL_BASE            (EXTERNAL_RAM_BASE + 0x3FE00000)
#define PVT_SOC_BASE                (EXTERNAL_RAM_BASE + 0x3FE10000)
#define PVT_GPU_BASE                (EXTERNAL_RAM_BASE + 0x3FE20000)
#define PVT_BIG_BASE                (EXTERNAL_RAM_BASE + 0x3FE30000)
#define PVT_LITTLE_BASE             (EXTERNAL_RAM_BASE + 0x3FE40000)
#define DDR_PHY1_BASE               (EXTERNAL_RAM_BASE + 0x3FEE0000)
#define DDR_PHY0_BASE               (EXTERNAL_RAM_BASE + 0x3FEF0000)
#define PCIE_CONTROLLER_BASE        (EXTERNAL_RAM_BASE + 0x3FF20000)
#define PCIE_ROOT_PORT_BASE         (EXTERNAL_RAM_BASE + 0x3FF30000)
#define UART1_BASE                  (EXTERNAL_RAM_BASE + 0x3FF70000)
#define SMC_BASE                    (EXTERNAL_RAM_BASE + 0x3FFD0000)

#define NOR_BASE                    (EXTERNAL_DEV_BASE + 0x08000000)
#define HDLCD_PRESET_TABLE_BASE     (NOR_BASE + 0x025C0000)

#endif /* JUNO_MMAP_H */
