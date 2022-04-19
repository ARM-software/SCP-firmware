/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_mmap.h"

#include <ddr_init.h>

#include <mod_nor.h>
#include <mod_synquacer_system.h>

#include <fwk_assert.h>
#include <fwk_attributes.h>
#include <fwk_log.h>
#include <fwk_macros.h>

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

static fwk_id_t nor_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_NOR, 0);

struct FWK_PACKED fip_toc_header_s {
    uint32_t name;
    uint32_t serial_num;
    uint64_t flags;
};

typedef struct fip_toc_header_s fip_toc_header_t;
/* fip toc header 16 byte*/

struct FWK_PACKED fip_toc_entry_s {
    uint8_t uuid[16];
    uint64_t offset_addr;
    uint64_t size;
    uint64_t flags;
};

typedef struct fip_toc_entry_s fip_toc_entry_t;
/* fip toc entry 40 byte*/

struct FWK_PACKED fip_package_s {
    fip_toc_header_t fip_toc_header;
    fip_toc_entry_t fip_toc_entry[5];
    uint32_t data[1];
};

typedef struct fip_package_s fip_package_t;

struct FWK_PACKED arm_tf_fip_package_s {
    fip_toc_header_t fip_toc_header;
    fip_toc_entry_t fip_toc_entry[4];
};

typedef struct arm_tf_fip_package_s arm_tf_fip_package_t;

#define ADDR_TRANS_EN (0x1 /* ADDR_TRANS_EN */ << 20)
#define REG_ASH_SCP_POW_CTL UINT32_C(0x50000000)
#define ADDR_TRANS_OFFSET UINT32_C(0x34)
#define OPTEE_OS_BINARY_SIZE UINT32_C(0x80000)
#define SCP_ADDR_TRANS_AREA UINT32_C(0xCB000000)
#define BL32_TOC_ENTRY_INDEX (3)

#define FIP_TOC_HEADER_NAME UINT32_C(0xAA640001)

#define UUID_NULL                               \
    {                                           \
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, \
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0  \
    }

#define FIP_UUID_TFA_BL2                                \
    {                                                   \
        0x5f, 0xf9, 0xec, 0x0b, 0x4d, 0x22, 0x3e, 0x4d, \
        0xa5, 0x44, 0xc3, 0x9d, 0x81, 0xc7, 0x3f, 0x0a, \
    }

#define UUID_SECURE_PAYLOAD_BL32                                          \
    {                                                                     \
        0x05, 0xd0, 0xe1, 0x89, 0x53, 0xdc, 0x13, 0x47, 0x8d, 0x2b, 0x50, \
            0x0a, 0x4b, 0x7a, 0x3e, 0x38                                  \
    }

/*
 * New FIP loader for TBBR supported BL2
 */
void fw_fip_load_bl2(uint32_t boot_index)
{
    fip_toc_entry_t *toc_entry;
    unsigned long offset;
    uint8_t uuid_null[] = UUID_NULL;
    uint8_t uuid_bl2[] = FIP_UUID_TFA_BL2;

    arm_tf_fip_package_t fip_package;

    offset = CONFIG_SCB_ARM_BL2_OFFSET + boot_index * CONFIG_SCB_FWU_BANK_SIZE;

    synquacer_system_ctx.nor_api->read(
        nor_id,
        0,
        MOD_NOR_READ_FAST_1_4_4_4BYTE,
        offset,
        &fip_package,
        sizeof(fip_package));

    if (fip_package.fip_toc_header.name != FIP_TOC_HEADER_NAME) {
        FWK_LOG_ERR("No FIP Image found @%lx !", offset);
        return;
    }

    toc_entry = fip_package.fip_toc_entry;

    do {
        if (memcmp((void *) uuid_null, (void *)toc_entry->uuid, 16) == 0) {
            FWK_LOG_ERR("[FIP] BL2 not found!");
            return;
        }
        if (memcmp((void *)uuid_bl2, (void *)toc_entry->uuid, 16) == 0) {
            offset += (uint32_t)toc_entry->offset_addr;
            FWK_LOG_INFO("[FIP] BL2 found %ubytes @%lx ",
                         (unsigned int)toc_entry->size, offset);
            synquacer_system_ctx.nor_api->read(
                nor_id,
                0,
                MOD_NOR_READ_FAST_1_4_4_4BYTE,
                offset,
                (void *)CONFIG_SCB_ARM_TB_BL1_BASE_ADDR,
                toc_entry->size);
            return;
        }
        toc_entry++;
    } while (1);
}

/*
 * Legacy implementation expects bl32 is located as 4th binary
 * in the arm-tf fip package.
 */
static void fw_fip_load_bl32(arm_tf_fip_package_t *fip_package_p)
{
    uint32_t trans_addr_39_20;
    uint32_t src;
    void *dst;

    /* enable DRAM access by configuring address trans register */
    trans_addr_39_20 =
        ((CONFIG_SCB_ARM_TB_BL32_BASE_ADDR >> 20) | ADDR_TRANS_EN);
    *((volatile uint32_t *)(REG_ASH_SCP_POW_CTL + ADDR_TRANS_OFFSET)) =
        trans_addr_39_20;

    src = CONFIG_SCB_ARM_TF_OFFSET +
        (uint32_t)fip_package_p->fip_toc_entry[BL32_TOC_ENTRY_INDEX].offset_addr;
    dst = (void *)SCP_ADDR_TRANS_AREA;

    FWK_LOG_INFO(
        "[FIP] load BL32 src 0x%" PRIx32 ", dst %" PRIx32 ", size %" PRIu32,
        HSSPI_MEM_BASE + src,
        (uint32_t)CONFIG_SCB_ARM_TB_BL32_BASE_ADDR,
        (uint32_t)fip_package_p->fip_toc_entry[BL32_TOC_ENTRY_INDEX].size);

    synquacer_system_ctx.nor_api->read(
        nor_id,
        0,
        MOD_NOR_READ_FAST_1_4_4_4BYTE,
        src,
        dst,
        (uint32_t)fip_package_p->fip_toc_entry[BL32_TOC_ENTRY_INDEX].size);

    /* disable DRAM access */
    trans_addr_39_20 =
        ((CONFIG_SCB_ARM_TB_BL32_BASE_ADDR >> 20) & ~ADDR_TRANS_EN);
    *((volatile uint32_t *)(REG_ASH_SCP_POW_CTL + ADDR_TRANS_OFFSET)) =
        trans_addr_39_20;

    FWK_LOG_INFO("[FIP] BL32 is loaded");
}

void fw_fip_load_arm_tf(void)
{
    uint32_t i;
    uint32_t src;
    uint8_t bl32_uuid[] = UUID_SECURE_PAYLOAD_BL32;
    const uint32_t arm_tf_dst_addr[] = { CONFIG_SCB_ARM_TB_BL1_BASE_ADDR,
                                         CONFIG_SCB_ARM_TB_BL2_BASE_ADDR,
                                         CONFIG_SCB_ARM_TB_BL3_BASE_ADDR };
    arm_tf_fip_package_t fip_package;

    static_assert(
        sizeof(fip_toc_header_t) == 16, "sizeof(fip_toc_header_t) is wrong");
    static_assert(
        sizeof(fip_toc_entry_t) == 40, "sizeof(fip_toc_entry_t) is wrong");
    static_assert(
        sizeof(arm_tf_fip_package_t) == 176,
        "sizeof(arm_tf_fip_package_t) is wrong");

    synquacer_system_ctx.nor_api->read(
        nor_id,
        0,
        MOD_NOR_READ_FAST_1_4_4_4BYTE,
        CONFIG_SCB_ARM_TF_OFFSET,
        &fip_package,
        sizeof(fip_package));

    for (i = 0; i < FWK_ARRAY_SIZE(arm_tf_dst_addr); i++) {
        FWK_LOG_INFO(
            "[FIP] fip_toc_entry[%" PRIu32 "] offset_addr %" PRIx32,
            i,
            (uint32_t)fip_package.fip_toc_entry[i].offset_addr);

        FWK_LOG_INFO(
            "[FIP] fip_toc_entry[%" PRIu32 "] size        %" PRIu32,
            i,
            (uint32_t)fip_package.fip_toc_entry[i].size);

        FWK_LOG_INFO(
            "[FIP] dst addr[%" PRIu32 "]                  %" PRIx32,
            i,
            arm_tf_dst_addr[i]);

        src = CONFIG_SCB_ARM_TF_OFFSET +
            (uint32_t)fip_package.fip_toc_entry[i].offset_addr;

        FWK_LOG_INFO(
            "[FIP] src addr[%" PRIu32 "]                  %" PRIx32,
            i,
            HSSPI_MEM_BASE + src);

        synquacer_system_ctx.nor_api->read(
            nor_id,
            0,
            MOD_NOR_READ_FAST_1_4_4_4BYTE,
            src,
            (void *)arm_tf_dst_addr[i],
            (uint32_t)fip_package.fip_toc_entry[i].size);
    }

    if (ddr_is_secure_dram_enabled()) {
        if (memcmp(
                (void *)bl32_uuid,
                (void *)fip_package.fip_toc_entry[BL32_TOC_ENTRY_INDEX].uuid,
                sizeof(bl32_uuid)) == 0) {
            fw_fip_load_bl32(&fip_package);
        } else {
            FWK_LOG_WARN("[FIP] no BL32 payload in fip image");
        }
    }
}
