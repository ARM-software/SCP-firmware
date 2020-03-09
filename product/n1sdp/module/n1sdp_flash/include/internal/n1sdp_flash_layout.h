/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Header file defining the N1SDP flash layout showing the structures of
 *     flash table of content (TOC), N1SDP FIP table of content, TOC signatures
 *     and UUID values of binary images within the firmware package.
 */

#ifndef INTERNAL_N1SDP_FLASH_LAYOUT_H
#define INTERNAL_N1SDP_FLASH_LAYOUT_H

#include "uuid.h"

#include <stdint.h>

/*
 * Flash entry definitions
 */

/* Flash signature */
#define FLASH_TOC_SIGNATURE    0x5A5AA5A5

/* Flash TOC entries */
struct n1sdp_flash_toc_entry {
    uint32_t type:16;
    uint32_t instance:8;
    uint32_t reserved:8;
    uint32_t offset;
    uint32_t size;
    uint32_t flags:16;
    uint32_t checksum:16;
} __attribute__((packed));

/* Flash Table of contents (TOC) */
struct n1sdp_flash_memory_toc {
    uint32_t signature;
    uint32_t major_version:8;
    uint32_t minor_version:8;
    uint32_t rev_number:8;
    uint32_t pass_number:8;
    uint32_t date_of_last_update;
    uint32_t entry_count:16;
    uint32_t checksum:16;
    struct n1sdp_flash_toc_entry entry[];
} __attribute__((packed));


/*
 * N1SDP Firmware Image Package (NFIP) entry definitions
 */

/* NFIP signature */
#define NFIP_TOC_SIGNATURE      0xAA640001

/*
 * UUID values of NFIP entries
 */
#define UUID_SCP_FIRMWARE_SCP_BL2 \
{0x3dfd6697, 0xbe89, 0x49e8, 0xae, 0x5d, {0x78, 0xa1, 0x40, 0x60, 0x82, 0x13} }
#define UUID_MCP_FIRMWARE_MCP_BL2 \
{0xce1e5890, 0x52d5, 0x11e7, 0xb1, 0x14, {0xb2, 0xf9, 0x33, 0xd5, 0xfe, 0x66} }
#define UUID_EL3_RUNTIME_FIRMWARE_BL31 \
{0x6d08d447, 0xfe4c, 0x4698, 0x9b, 0x95, {0x29, 0x50, 0xcb, 0xbd, 0x5a, 0x00} }
#define UUID_SECURE_PAYLOAD_BL32 \
{0x89e1d005, 0xdc53, 0x4713, 0x8d, 0x2b, {0x50, 0x0a, 0x4b, 0x7a, 0x3e, 0x38} }
#define UUID_NON_TRUSTED_FIRMWARE_BL33 \
{0xa7eed0d6, 0xeafc, 0x4bd5, 0x97, 0x82, {0x99, 0x34, 0xf2, 0x34, 0xb6, 0xe4} }

/* NFIP TOC entry */
struct n1sdp_fip_toc_entry {
    struct uuid_t uuid;
    uint32_t offset;
    uint32_t size;
    uint32_t flags;
    uint32_t image_checksum:16;
    uint32_t checksum:16;
} __attribute__((packed));

/* NFIP Table of contents (TOC) */
struct n1sdp_fip_memory_toc {
    uint32_t signature;
    uint32_t major_version:8;
    uint32_t minor_version:8;
    uint32_t rev_number:8;
    uint32_t pass_number:8;
    uint32_t date_of_last_update;
    uint32_t entry_count:16;
    uint32_t checksum:16;
    struct n1sdp_fip_toc_entry entry[];
} __attribute__((packed));

#endif /* INTERNAL_N1SDP_FLASH_LAYOUT_H */
