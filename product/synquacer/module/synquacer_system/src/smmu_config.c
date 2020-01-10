/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mmu500.h"
#include "synquacer_config.h"
#include "synquacer_mmap.h"

#include <internal/smmu_wrapper.h>

#include <mod_synquacer_system.h>

#include <fwk_log.h>

#include <stdint.h>
#include <string.h>

typedef struct {
    uint64_t desc[0x1000 / 8];
} smmu_granule4kb_desc_t;

typedef struct {
    uint64_t desc[0x10000 / 8];
} smmu_granule16kb_desc_t;

typedef struct {
    uint64_t desc[0x40000 / 8];
} smmu_granule64kb_desc_t;

enum upper_attr {
    MMU500_UPPER_ATTR_CONTIGUOUS = (1 << 0),
    MMU500_UPPER_ATTR_PXN = (1 << 1),
    MMU500_UPPER_ATTR_XN = (1 << 2),
};

enum lower_attr {
    MMU500_LOWER_ATTR_ATTRINDX_0 = 0,
    MMU500_LOWER_ATTR_ATTRINDX_1 = 1,
    MMU500_LOWER_ATTR_ATTRINDX_2 = 2,
    MMU500_LOWER_ATTR_ATTRINDX_3 = 3,
    MMU500_LOWER_ATTR_ATTRINDX_4 = 4,
    MMU500_LOWER_ATTR_ATTRINDX_5 = 5,
    MMU500_LOWER_ATTR_ATTRINDX_6 = 6,
    MMU500_LOWER_ATTR_ATTRINDX_7 = 7,
    MMU500_LOWER_ATTR_NS = (1 << 3),
    MMU500_LOWER_ATTR_AP_RW = (1 << 4),
    MMU500_LOWER_ATTR_AP_RO = (3 << 4),
    MMU500_LOWER_ATTR_SH_NON_SHAREABLE = 0,
    MMU500_LOWER_ATTR_SH_OUTER_SHAREABLE = (2 << 6),
    MMU500_LOWER_ATTR_SH_INNER_SHAREABLE = (3 << 6),
    MMU500_LOWER_ATTR_AF = (1 << 8),
    MMU500_LOWER_ATTR_NG = (1 << 9)
};

enum pagetable_valid {
    MMU500_PAGETABLE_VALID_OFF = 0,
    MMU500_PAGETABLE_VALID_ON = 1
};

#define SMMU_DESC_T smmu_granule4kb_desc_t

uint64_t smmu_convert_to_axi_addr(void *addr_cm3view);
void smmu_wrapper_pcie(void);

static uint64_t mmu500_make_next_level_table_addr_desc(
    enum mmu500_granule granule,
    int level,
    void *next_level_table_addr_cm3view)
{
    return (smmu_convert_to_axi_addr(next_level_table_addr_cm3view) | 0x3);
}

static uint64_t mmu500_make_block_desc(
    enum mmu500_granule granule,
    int level,
    uint64_t output_addr,
    uint64_t upper_attr,
    uint64_t lower_attr)
{
    return ((upper_attr << 52) | output_addr | (lower_attr << 2) | 0x1);
}

static uint64_t mmu500_make_level3_desc(
    enum mmu500_granule granule,
    uint64_t output_addr,
    uint64_t upper_attr,
    uint64_t lower_attr)
{
    return ((upper_attr << 52) | output_addr | (lower_attr << 2) | 0x3);
}

uint64_t smmu_convert_to_axi_addr(void *addr_cm3view)
{
    uint32_t addr = (uint32_t)addr_cm3view;

    return (uint64_t)(addr - 0xa0000000U);
}

void smmu_wrapper_initialize(void)
{
    FWK_LOG_INFO("Configure System MMUs starts");

    /* Basic Configuration */
    smmu_wrapper_pcie();

    FWK_LOG_INFO("Configure System MMUs finished");
}

void smmu_wrapper_pcie(void)
{
    uint64_t upper_attr = 0;
    uint64_t lower_attr_noncoherent =
        (MMU500_LOWER_ATTR_SH_NON_SHAREABLE | MMU500_LOWER_ATTR_AP_RW |
         MMU500_LOWER_ATTR_AF | MMU500_LOWER_ATTR_ATTRINDX_1);

    uint64_t lower_attr_coherent =
        (MMU500_LOWER_ATTR_SH_OUTER_SHAREABLE | MMU500_LOWER_ATTR_AP_RW |
         MMU500_LOWER_ATTR_AF | MMU500_LOWER_ATTR_ATTRINDX_0);

    SMMU_DESC_T *curr_smmu_desc =
        (SMMU_DESC_T *)CONFIG_SCB_SMMU_PAGE_TABLE_BASE_ADDR;
    SMMU_DESC_T *tbu0_smmu_desc_addr;
    SMMU_DESC_T *tbu2_smmu_desc_addr;
    SMMU_DESC_T *tbu3_smmu_desc_addr;

    MMU500_ContextInfo_t context_info[3];
    int i;
    uint64_t w_lower_attr_tbu3, w_lower_attr_tbu2;

    /* attribute set(TBU3) */
    if (eeprom_config.pcie_cache_snoop_valid[0] == 0)
        w_lower_attr_tbu3 = lower_attr_noncoherent | MMU500_LOWER_ATTR_NS;
    else
        w_lower_attr_tbu3 = lower_attr_coherent | MMU500_LOWER_ATTR_NS;

    /* attribute set(TBU2) */
    if (eeprom_config.pcie_cache_snoop_valid[1] == 0)
        w_lower_attr_tbu2 = lower_attr_noncoherent | MMU500_LOWER_ATTR_NS;
    else
        w_lower_attr_tbu2 = lower_attr_coherent | MMU500_LOWER_ATTR_NS;

    memset(
        (void *)CONFIG_SCB_SMMU_PAGE_TABLE_BASE_ADDR,
        0,
        sizeof(SMMU_DESC_T) * 15);

    /**********************************************************************
     * North MMU(TBU0) settings
     **********************************************************************/
    tbu0_smmu_desc_addr = curr_smmu_desc;

    /* page#0: level0 descriptor */

    /* 0x0000_0000_0000_0000/39 */
    curr_smmu_desc->desc[0x000] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 0, (curr_smmu_desc + 1));

    /* page#1: level1 descriptor #0 */
    ++curr_smmu_desc;

    /* 0x0000_0000_4000_0000/30 */
    curr_smmu_desc->desc[0x001] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 1, (curr_smmu_desc + 1));

    /* 0x0000_003e_0000_0000/32 */
    for (i = 0xf8; i < 0xfc; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0xcLLU << 44) | 0x3e00000000LLU | (((uint64_t)(i & 0x3)) << 30)),
            upper_attr,
            lower_attr_noncoherent);
    }

    /* 0x0000_003f_0000_0000/32 */
    for (i = 0xfc; i < 0x100; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0x8LLU << 44) | 0x3f00000000LLU | (((uint64_t)(i & 0x3)) << 30)),
            upper_attr,
            lower_attr_noncoherent);
    }

#ifndef PCIE_FILTER_BUS0_TYPE0_CONFIG
    /* page#2: level2 descriptor #0 */
    ++curr_smmu_desc;

    /* 0x0000_0000_6000_0000/28 */
    for (i = 0x100; i < 0x180; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0xcLLU << 44) | 0x60000000LLU | (((uint64_t)(i & 0x7f)) << 21)),
            upper_attr,
            lower_attr_noncoherent);
    }

    /* 0x0000_0000_7000_0000/28 */
    for (i = 0x180; i < 0x200; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0x8LLU << 44) | 0x70000000LLU | (((uint64_t)(i & 0x7f)) << 21)),
            upper_attr,
            lower_attr_noncoherent);
    }

#else
    /* page#2: level2 descriptor #0 */
    ++curr_smmu_desc;

    /* 0x0000_0000_6000_0000/21 */
    curr_smmu_desc->desc[0x100] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 0, (curr_smmu_desc + 1));

    /* 0x0000_0000_6020_0000/28 */
    for (i = 0x101; i < 0x180; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0xcLLU << 44) | 0x60000000LLU | (((uint64_t)(i & 0x7f)) << 21)),
            upper_attr,
            lower_attr_noncoherent);
    }

    /* 0x0000_0000_7000_0000/28 */
    curr_smmu_desc->desc[0x180] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 0, (curr_smmu_desc + 2));

    /* 0x0000_0000_7020_0000/28 */
    for (i = 0x181; i < 0x200; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0x8LLU << 44) | 0x70000000LLU | (((uint64_t)(i & 0x7f)) << 21)),
            upper_attr,
            lower_attr_noncoherent);
    }

    /* page#3: level3 descriptor #0 */
    ++curr_smmu_desc;

    /* 0x0000_0000_6000_0000/15 - 0x0000_0000_6000_7FFF/15 */
    /* direct mapping to PCIe bus#0 dev#0(32KB) */
    for (i = 0x0; i < 0x8; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_level3_desc(
            MMU500_GRANULE_4KB,
            ((0xcLLU << 44) | 0x60000000LLU | (((uint64_t)(i & 0x7)) << 12)),
            upper_attr,
            lower_attr_noncoherent);
    }

    /* 0x0000_0000_6000_8000/20 - 0x0000_0000_600F_FFFF/20 */
    /* translate PCIe bus#0 dev#1-31 access to the region always return 0xFF..F
     */
    for (i = 0x8; i < 0x100; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_level3_desc(
            MMU500_GRANULE_4KB,
            ((0xcLLU << 44) | 0x67F10000LLU | (((uint64_t)(i & 0x7)) << 12)),
            upper_attr,
            lower_attr_noncoherent);
    }

    /* 0x0000_0000_6010_0000/21 - 0x0000_0000_601F_FFFF/21 */
    /* direct mapping to PCIe bus#1 */
    for (i = 0x100; i < 0x200; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_level3_desc(
            MMU500_GRANULE_4KB,
            ((0xcLLU << 44) | 0x60000000LLU | (((uint64_t)(i & 0x1ff)) << 12)),
            upper_attr,
            lower_attr_noncoherent);
    }

    /* page#4: level3 descriptor #1 */
    ++curr_smmu_desc;

    /* 0x0000_0000_7000_0000/15 - 0x0000_0000_7000_7FFF/15 */
    /* direct mapping to PCIe bus#0 dev#0(32KB) */
    for (i = 0x0; i < 0x8; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_level3_desc(
            MMU500_GRANULE_4KB,
            ((0x8LLU << 44) | 0x70000000LLU | (((uint64_t)(i & 0x7)) << 12)),
            upper_attr,
            lower_attr_noncoherent);
    }

    /* 0x0000_0000_7000_8000/20 - 0x0000_0000_700F_FFFF/20 */
    /* translate PCIe bus#0 dev#1-31 access to the region always return 0xFF..F
     */
    for (i = 0x8; i < 0x100; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_level3_desc(
            MMU500_GRANULE_4KB,
            ((0x8LLU << 44) | 0x77F10000LLU | (((uint64_t)(i & 0x7)) << 12)),
            upper_attr,
            lower_attr_noncoherent);
    }

    /* 0x0000_0000_7010_0000/21 - 0x0000_0000_701F_FFFF/21 */
    /* direct mapping to PCIe bus#1 */
    for (i = 0x100; i < 0x200; i++) {
        curr_smmu_desc->desc[i] = mmu500_make_level3_desc(
            MMU500_GRANULE_4KB,
            ((0x8LLU << 44) | 0x70000000LLU | (((uint64_t)(i & 0x1ff)) << 12)),
            upper_attr,
            lower_attr_noncoherent);
    }
#endif /* PCIE_FILTER_BUS0_TYPE0_CONFIG */

    /**********************************************************************
     * North MMU(TBU3) settings
     **********************************************************************/

    /* page#5: level0 descriptor */
    ++curr_smmu_desc;
    tbu3_smmu_desc_addr = curr_smmu_desc;

    /* 0x0000_0000_0000_0000/39 */
    curr_smmu_desc->desc[0x000] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 0, (curr_smmu_desc + 1));

    /* 0x0000_0080_0000_0000/39 */
    curr_smmu_desc->desc[0x001] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 0, (curr_smmu_desc + 2));

    /* 0x0000_0800_0000_0000/39 */
    curr_smmu_desc->desc[0x010] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 0, (curr_smmu_desc + 3));

    /* page#6: level1 descriptor #0 */
    ++curr_smmu_desc;

    /* 0x0000_0000_0000_0000-0x0000_0000_7fff_ffff: always non-coherent */
    for (i = 0x000; i < 0x002; i++) {
        uint64_t phy_base_addr = (((uint64_t)i) << 30);

        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0x4LLU << 44) | phy_base_addr),
            upper_attr,
            lower_attr_noncoherent);
    }
    /* 0x0000_0000_8000_0000-0x0000_007f_ffff_ffff: user-configured coherence */
    for (i = 0x002; i < 0x200; i++) {
        uint64_t phy_base_addr = (((uint64_t)i) << 30);

        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0x4LLU << 44) | phy_base_addr),
            upper_attr,
            w_lower_attr_tbu3);
    }

    /* page#7: level1 descriptor #1 */
    ++curr_smmu_desc;

    /* 0x0000_0080_0000_0000/39 */
    for (i = 0x000; i < 0x200; i++) {
        uint64_t phy_base_addr = (((uint64_t)i) << 30);

        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0x4LLU << 44) | phy_base_addr | 0x008000000000LLU),
            upper_attr,
            w_lower_attr_tbu3);
    }

    /* page#8: level1 descriptor #2 */
    ++curr_smmu_desc;

    /* 0x0000_0800_0000_0000/30 */
    curr_smmu_desc->desc[0x000] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 1, (curr_smmu_desc + 1));

    /* page#9: level2 descriptor #0 */
    ++curr_smmu_desc;

    /* 0x0000_0800_0000_0000/22 */
    for (i = 0x000; i < 0x002; i++) {
        uint64_t peri_phys_base_addr = (((uint64_t)i) << 21) | 0x58000000LLU;

        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            2,
            ((0x4LLU << 44) | peri_phys_base_addr),
            upper_attr,
            lower_attr_noncoherent);
    }

    /**********************************************************************
     * North MMU(TBU2) settings
     **********************************************************************/

    /* page#10: level0 descriptor */
    ++curr_smmu_desc;
    tbu2_smmu_desc_addr = curr_smmu_desc;

    /* 0x0000_0000_0000_0000/39 */
    curr_smmu_desc->desc[0x000] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 0, (curr_smmu_desc + 1));

    /* 0x0000_0080_0000_0000/39 */
    curr_smmu_desc->desc[0x001] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 0, (curr_smmu_desc + 2));

    /* 0x0000_0800_0000_0000/39 */
    curr_smmu_desc->desc[0x010] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 0, (curr_smmu_desc + 3));

    /* page#11: level1 descriptor #0 */
    ++curr_smmu_desc;

    /* 0x0000_0000_0000_0000-0x0000_0000_7fff_ffff: always non-coherent */
    for (i = 0x000; i < 0x002; i++) {
        uint64_t phy_base_addr = (((uint64_t)i) << 30);

        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0x4LLU << 44) | phy_base_addr),
            upper_attr,
            lower_attr_noncoherent);
    }
    /* 0x0000_0000_8000_0000-0x0000_007f_ffff_ffff: user-configured coherence */
    for (i = 0x002; i < 0x200; i++) {
        uint64_t phy_base_addr = (((uint64_t)i) << 30);

        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0x4LLU << 44) | phy_base_addr),
            upper_attr,
            w_lower_attr_tbu2);
    }

    /* page#12: level1 descriptor #1 */
    ++curr_smmu_desc;

    /* 0x0000_0080_0000_0000/39 */
    for (i = 0x000; i < 0x200; i++) {
        uint64_t phy_base_addr = (((uint64_t)i) << 30);

        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            1,
            ((0x4LLU << 44) | phy_base_addr | 0x008000000000LLU),
            upper_attr,
            w_lower_attr_tbu2);
    }

    /* page#13: level1 descriptor #2 */
    ++curr_smmu_desc;

    /* 0x0000_0800_0000_0000/30 */
    curr_smmu_desc->desc[0x000] = mmu500_make_next_level_table_addr_desc(
        MMU500_GRANULE_4KB, 1, (curr_smmu_desc + 1));

    /* page#14: level2 descriptor #0 */
    ++curr_smmu_desc;

    /* 0x0000_0800_0000_0000/22 */
    for (i = 0x000; i < 0x002; i++) {
        uint64_t peri_phys_base_addr = (((uint64_t)i) << 21) | 0x58000000LLU;

        curr_smmu_desc->desc[i] = mmu500_make_block_desc(
            MMU500_GRANULE_4KB,
            2,
            ((0x4LLU << 44) | peri_phys_base_addr),
            upper_attr,
            lower_attr_noncoherent);
    }

    /* stream id mapping */
    /* TBU0 */
    context_info[0].stream_match_mask = 0x3ff;
    context_info[0].stream_match_id = 0;
    context_info[0].base_addr = smmu_convert_to_axi_addr(tbu0_smmu_desc_addr);
    /* TBU3 */
    context_info[1].stream_match_mask = 0x3ff;
    context_info[1].stream_match_id = 0xC00;
    context_info[1].base_addr = smmu_convert_to_axi_addr(tbu3_smmu_desc_addr);
    /* TBU2 */
    context_info[2].stream_match_mask = 0x3ff;
    context_info[2].stream_match_id = 0x800;
    context_info[2].base_addr = smmu_convert_to_axi_addr(tbu2_smmu_desc_addr);

    SMMU_s_init(
        (MMU500_Type_t *)CONFIG_SOC_NORTH_SMMU_REG_BASE,
        (uint32_t)3,
        context_info,
        MMU500_GRANULE_4KB);
}
