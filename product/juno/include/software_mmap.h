/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Software-defined memory map shared between SCP and AP cores.
 */

#ifndef SOFTWARE_MMAP_H
#define SOFTWARE_MMAP_H

#include "system_mmap.h"

/*
 * The 4KB "Context Region" at the base of Trusted SRAM is used for several
 * purposes. These are: the Shared Data Storage (SDS) Memory Region, the SCMI
 * secure payload areas, and the context area for Application Processor
 * firmware.
 *
 * Shared Data Storage (SDS) Memory Region: Used for structured storage of data
 * that is shared between SCP Firmware and Application Processor firmware. The
 * SDS Memory Region occupies the area between the context region base and
 * the SCMI Secure Payload base.
 *
 * SCMI Secure Payload Areas: Storage for SCMI message contents in both the
 * Agent->Platform and Platform->Agent directions.
 *
 * Application Processor Context Area: The usage of this area is defined by the
 * firmware running on the Application Processors. The SCP firmware must zero
 * this memory before releasing any Application Processors. This area must
 * always be located in the top 64 bytes of the 4KB reserved region.
 *
 *
 *       +-----------------------+ 4096
 *       |                       |
 *   64B |    AP Context Area    |
 *       |                       |
 *       +-----------------------+
 *       |                       |
 *  256B |        Unused         |
 *       |                       |
 *       +-----------------------+
 *       |                       |
 *       |   SCMI Sec. Payload   |
 *  128B |   Platform to Agent   |
 *       |                       |
 *       +-----------------------+
 *       |                       |
 *  128B |   SCMI Sec. Payload   |
 *       |   Agent to Platform   |
 *       |                       |
 *       +-----------------------+
 *       |                       |
 * 3520B |   SDS Memory Region   |
 *       |                       |
 *       +-----------------------+ 0
 */

/* Context Region */
#define CONTEXT_REGION_BASE           (TRUSTED_RAM_BASE)
#define CONTEXT_REGION_SIZE           (4096)
#define CONTEXT_REGION_LIMIT          (CONTEXT_REGION_BASE + \
                                       CONTEXT_REGION_SIZE)
/* AP Context Area */
#define AP_CONTEXT_BASE               (CONTEXT_REGION_LIMIT - AP_CONTEXT_SIZE)
#define AP_CONTEXT_SIZE               (64)

/* SCMI Secure Payload Areas */
#define SCMI_PAYLOAD_SIZE             (128)
#define SCMI_PAYLOAD_S_BASE           (CONTEXT_REGION_BASE + 3520)
#define SCMI_PAYLOAD_S_A2P_BASE       (SCMI_PAYLOAD_S_BASE)
#define SCMI_PAYLOAD_S_P2A_BASE       (SCMI_PAYLOAD_S_A2P_BASE + \
                                       SCMI_PAYLOAD_SIZE)

/* SDS Memory Region */
#define SDS_SHARED_MEM_BASE           (CONTEXT_REGION_BASE)
#define SDS_SHARED_MEM_SIZE           (SCMI_PAYLOAD_S_BASE - \
                                       CONTEXT_REGION_BASE)

/* SCMI Payload Areas */
#define SCMI_PAYLOAD_LOW_A2P_BASE     (NONTRUSTED_RAM_BASE)
#define SCMI_PAYLOAD_LOW_P2A_BASE     (SCMI_PAYLOAD_LOW_A2P_BASE + \
                                       SCMI_PAYLOAD_SIZE)
#define SCMI_PAYLOAD_HIGH_A2P_BASE    (SCMI_PAYLOAD_LOW_P2A_BASE + \
                                       SCMI_PAYLOAD_SIZE)
#define SCMI_PAYLOAD_HIGH_P2A_BASE    (SCMI_PAYLOAD_HIGH_A2P_BASE + \
                                       SCMI_PAYLOAD_SIZE)

#endif /* SOFTWARE_MMAP_H */
