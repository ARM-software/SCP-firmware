/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_mmap.h"

#include <ddr_init.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_macros.h>

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

struct fip_toc_header_s {
    uint32_t name;
    uint32_t serial_num;
    uint64_t flags;
} __attribute__((packed));

typedef struct fip_toc_header_s fip_toc_header_t;
/* fip toc header 16 byte*/

struct fip_toc_entry_s {
    uint8_t uuid[16];
    uint64_t offset_addr;
    uint64_t size;
    uint64_t flags;
} __attribute__((packed));

typedef struct fip_toc_entry_s fip_toc_entry_t;
/* fip toc entry 40 byte*/

struct fip_package_s {
    fip_toc_header_t fip_toc_header;
    fip_toc_entry_t fip_toc_entry[5];
    uint32_t data[1];
} __attribute__((packed));

typedef struct fip_package_s fip_package_t;

struct arm_tf_fip_package_s {
    fip_toc_header_t fip_toc_header;
    fip_toc_entry_t fip_toc_entry[4];
} __attribute__((packed));

typedef struct arm_tf_fip_package_s arm_tf_fip_package_t;

#define ADDR_TRANS_EN (0x1 /* ADDR_TRANS_EN */ << 20)
#define REG_ASH_SCP_POW_CTL UINT32_C(0x50000000)
#define ADDR_TRANS_OFFSET UINT32_C(0x34)
#define OPTEE_OS_BINARY_SIZE UINT32_C(0x80000)
#define SCP_ADDR_TRANS_AREA UINT32_C(0xCB000000)
#define BL32_TOC_ENTRY_INDEX (3)

#define UUID_SECURE_PAYLOAD_BL32                                          \
    {                                                                     \
        0x05, 0xd0, 0xe1, 0x89, 0x53, 0xdc, 0x13, 0x47, 0x8d, 0x2b, 0x50, \
            0x0a, 0x4b, 0x7a, 0x3e, 0x38                                  \
    }

/*
 * Current implementation expects bl32 is located as 4th binary
 * in the arm-tf fip package.
 */
static void fw_fip_load_bl32(void)
{
    uint32_t trans_addr_39_20;
    void *src, *dst;
    uint8_t bl32_uuid[] = UUID_SECURE_PAYLOAD_BL32;

    arm_tf_fip_package_t *fip_package_p =
        (arm_tf_fip_package_t *)CONFIG_SCB_ARM_TF_BASE_ADDR;

    if (memcmp(
            (void *)bl32_uuid,
            (void *)fip_package_p->fip_toc_entry[BL32_TOC_ENTRY_INDEX].uuid,
            sizeof(bl32_uuid)) != 0) {
        FWK_LOG_ERR("[FIP] BL32 UUID is wrong, skip loading");
        return;
    }

    FWK_LOG_ERR("[FIP] load BL32");

    /* enable DRAM access by configuring address trans register */
    trans_addr_39_20 =
        ((CONFIG_SCB_ARM_TB_BL32_BASE_ADDR >> 20) | ADDR_TRANS_EN);
    *((volatile uint32_t *)(REG_ASH_SCP_POW_CTL + ADDR_TRANS_OFFSET)) =
        trans_addr_39_20;

    src = (void *)((uint32_t)fip_package_p +
     (uint32_t)fip_package_p->fip_toc_entry[BL32_TOC_ENTRY_INDEX].offset_addr);
    dst = (void *)SCP_ADDR_TRANS_AREA;
    memcpy(dst, src, fip_package_p->fip_toc_entry[BL32_TOC_ENTRY_INDEX].size);

    /* disable DRAM access */
    trans_addr_39_20 =
        ((CONFIG_SCB_ARM_TB_BL32_BASE_ADDR >> 20) & ~ADDR_TRANS_EN);
    *((volatile uint32_t *)(REG_ASH_SCP_POW_CTL + ADDR_TRANS_OFFSET)) =
        trans_addr_39_20;

    FWK_LOG_ERR("[FIP] BL32 is loaded");
}

void fw_fip_load_arm_tf(void)
{
    uint32_t i;

    const uint32_t arm_tf_dst_addr[3] = { CONFIG_SCB_ARM_TB_BL1_BASE_ADDR,
                                          CONFIG_SCB_ARM_TB_BL2_BASE_ADDR,
                                          CONFIG_SCB_ARM_TB_BL3_BASE_ADDR };

    arm_tf_fip_package_t *fip_package_p =
        (arm_tf_fip_package_t *)CONFIG_SCB_ARM_TF_BASE_ADDR;

    static_assert(
        sizeof(fip_toc_header_t) == 16, "sizeof(fip_toc_header_t) is wrong");
    static_assert(
        sizeof(fip_toc_entry_t) == 40, "sizeof(fip_toc_entry_t) is wrong");
    static_assert(
        sizeof(arm_tf_fip_package_t) == 176,
        "sizeof(arm_tf_fip_package_t) is wrong");

    for (i = 0; i < FWK_ARRAY_SIZE(arm_tf_dst_addr); i++) {
        FWK_LOG_INFO(
            "[FIP] fip_toc_entry[%" PRIu32 "] offset_addr %" PRIx64,
            i,
            fip_package_p->fip_toc_entry[i].offset_addr);

        FWK_LOG_INFO(
            "[FIP] fip_toc_entry[%" PRIu32 "] size        %" PRIu64,
            i,
            fip_package_p->fip_toc_entry[i].size);

        FWK_LOG_INFO(
            "[FIP] dst addr[%" PRIu32 "]                  %" PRIx32,
            i,
            arm_tf_dst_addr[i]);

        FWK_LOG_INFO(
            "[FIP] src addr[%" PRIu32 "]                  %" PRIx32,
            i,
            ((uint32_t)fip_package_p +
             (uint32_t)fip_package_p->fip_toc_entry[i].offset_addr));

        memcpy((void *)arm_tf_dst_addr[i],
               (void *)((uint32_t)fip_package_p +
                        (uint32_t)fip_package_p->fip_toc_entry[i].offset_addr),
               (uint32_t)fip_package_p->fip_toc_entry[i].size);
    }

    if (ddr_is_secure_dram_enabled())
        fw_fip_load_bl32();
}
