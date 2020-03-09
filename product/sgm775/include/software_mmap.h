/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Software defined memory map shared between SCP and AP cores.
 */

#ifndef SOFTWARE_MMAP_H
#define SOFTWARE_MMAP_H

#include "system_mmap.h"

#include <fwk_macros.h>

/*
 * The 4KiB AP/SCP Shared memory at the base of Trusted SRAM is used for several
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
 * firmware running on the Application Processors. The SCP Firmware must zero
 * this memory before releasing any Application Processors. This area must
 * always be located in the top 64 bytes of the 4KiB reserved region.
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

/* Secure shared memory at the base of Trusted SRAM */
#define SHARED_SECURE_BASE (TRUSTED_RAM_BASE)
#define SHARED_SECURE_SIZE (4 * FWK_KIB)

/* SDS Memory Region */
#define SDS_MEM_BASE (SHARED_SECURE_BASE)
#define SDS_MEM_SIZE (3520)

/* AP Context Area */
#define AP_CONTEXT_BASE (SHARED_SECURE_BASE + SHARED_SECURE_SIZE - \
                         AP_CONTEXT_SIZE)
#define AP_CONTEXT_SIZE (64)

/* SCMI Secure Payload Areas */
#define SCMI_PAYLOAD_SIZE       (128)
#define SCMI_PAYLOAD_S_A2P_BASE (SDS_MEM_BASE + SDS_MEM_SIZE)
#define SCMI_PAYLOAD_S_P2A_BASE (SCMI_PAYLOAD_S_A2P_BASE + SCMI_PAYLOAD_SIZE)

/*
 * The 4KiB AP/SCP Shared memory at the base of Non-trusted SRAM is used for the
 * SCMI non-secure payload areas.
 *
 * Two SCMI non-Secure Payload Areas: Storage for SCMI message contents in both
 * the Agent->Platform and Platform->Agent directions.
 *
 *       +-----------------------+ 4096
 * 3584B |        Unused         |
 *       +-----------------------+
 *       |                       |
 *       |  Non-Sec. Channel 1   |
 *       | SCMI non-Sec. Payload |
 *  128B |   Platform to Agent   |
 *       |                       |
 *       +-----------------------+
 *       |                       |
 *       |  Non-Sec. Channel 1   |
 *  128B | SCMI non-Sec. Payload |
 *       |   Agent to Platform   |
 *       |                       |
 *       +-----------------------+
 *       |                       |
 *       |  Non-Sec. Channel 0   |
 *       | SCMI non-Sec. Payload |
 *  128B |   Platform to Agent   |
 *       |                       |
 *       +-----------------------+
 *       |                       |
 *       |  Non-Sec. Channel 0   |
 *  128B | SCMI non-Sec. Payload |
 *       |   Agent to Platform   |
 *       |                       |
 *       +-----------------------+ 0
 */

/* Non-secure shared memory at the base of Non-trusted SRAM */
#define SHARED_NONSECURE_BASE (NONTRUSTED_RAM_BASE)
#define SHARED_NONSECURE_SIZE (4 * FWK_KIB)

/* SCMI Non-Secure Payload Areas */
#define SCMI_PAYLOAD0_NS_A2P_BASE (SHARED_NONSECURE_BASE)
#define SCMI_PAYLOAD0_NS_P2A_BASE (SCMI_PAYLOAD0_NS_A2P_BASE + \
                                   SCMI_PAYLOAD_SIZE)
#define SCMI_PAYLOAD1_NS_A2P_BASE (SCMI_PAYLOAD0_NS_P2A_BASE + \
                                   SCMI_PAYLOAD_SIZE)
#define SCMI_PAYLOAD1_NS_P2A_BASE (SCMI_PAYLOAD1_NS_A2P_BASE + \
                                   SCMI_PAYLOAD_SIZE)

#endif /* SOFTWARE_MMAP_H */
