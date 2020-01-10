/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mmu500.h"

#include <mod_synquacer_system.h>

#include <fwk_log.h>

#include <inttypes.h>

void SMMU_setup_PMU(MMU500_Type_t *SMMU)
{
    volatile uint32_t *pmu_regs;
    int page_size;

    page_size = ((SMMU->GR0.IDR1 & SMMU_IDR1_PAGESIZE) == 0) ? 0x1000 : 0x10000;

    pmu_regs = (volatile uint32_t *)(((uint32_t)SMMU) + 3 * page_size);

    pmu_regs[0x0400 / 4] =
        0x00000010U; /* PMEVTYPER0: P=U=NSP=NSU=0, EVENT=0x0010(access)  */
    pmu_regs[0x0404 / 4] =
        0x00000011U; /* PMEVTYPER1: P=U=NSP=NSU=0, EVENT=0x0011(read)    */
    pmu_regs[0x0408 / 4] =
        0x00000012U; /* PMEVTYPER2: P=U=NSP=NSU=0, EVENT=0x0012(write)   */
    pmu_regs[0x0c00 / 4] =
        0x00000007U; /* PMCNTENSET0: enable counter #0, #1, #2           */
    pmu_regs[0x0800 / 4] = 0x00000800U; /* PMCGCR0: enable=1, global_basis */
    pmu_regs[0x0e04 / 4] = 0x00000003U; /* PMCR: enable=1, reset=1 */

    FWK_LOG_INFO(
        "[MMU500] setup PMU for MMU-500@0x%08" PRIx32
        ". page_size=%d. 0x%08" PRIx32 "(access), 0x%08" PRIx32
        "(read), 0x%08" PRIx32 "(write).",
        (uint32_t)SMMU,
        page_size,
        (uint32_t)&pmu_regs[0],
        (uint32_t)&pmu_regs[1],
        (uint32_t)&pmu_regs[2]);
}

int32_t SMMU_s_init(
    MMU500_Type_t *SMMU,
    uint32_t num_context,
    const MMU500_ContextInfo_t *context_info,
    enum mmu500_granule granule)
{
    uint32_t temp;
    uint32_t i;

    SMMU->GR0.ACR |=
        SMMU_ACR_S2CRB_TLBEN | SMMU_ACR_MMUDISB_TLBEN | SMMU_ACR_SMTNMB_TLBEN;

    if ((num_context > (SMMU->GR0.IDR1 & SMMU_IDR1_NUMCB)) ||
        (num_context > (SMMU->GR0.IDR0 & SMMU_IDR0_NUMSMRG))) {
        /* num_context too large */
        return -1;
    }

    /* Use all context banks for secure access */
    temp = SMMU->GR0.SCR1;
    temp &= ~(SMMU_SCR1_NSNUMCBO | SMMU_SCR1_NSNUMSMRGO);
    SMMU->GR0.SCR1 = temp | (0 << SMMU_SCR1_NSNUMCBO_OFFSET) |
        (0 << SMMU_SCR1_NSNUMSMRGO_OFFSET) | SMMU_SCR1_SPMEN;

    SMMU->GR0.CR0 =
        (SMMU_CR0_SMCFCFG | SMMU_CR0_USFCFG | SMMU_CR0_STALLD |
         SMMU_CR0_GCFGFIE | SMMU_CR0_GCFGFRE | SMMU_CR0_GFIE | SMMU_CR0_GFRE);

    for (i = 0; i < num_context; i++) {
        SMMU->GR0.S2CR[i] = SMMU_S2CRn_TYPE_CONTEXT | i;
        SMMU->GR0.SMR[i] = SMMU_SMRn_VALID |
            (context_info[i].stream_match_mask << 16) |
            context_info[i].stream_match_id;
        SMMU_ns_cb_stage1_init(SMMU, i, context_info[i].base_addr, granule);
    }

    SMMU_setup_PMU(SMMU);

    return 0;
}

void SMMU_ns_cb_stage1_init(
    MMU500_Type_t *SMMU,
    uint32_t cb,
    uint64_t base_addr,
    enum mmu500_granule granule)
{
    uint64_t temp;

    SMMU->GR1.CBAR[cb] = SMMU_CBAR_TYPE_STAGE1 | SMMU_CBAR_MEM_NORMAL;
    SMMU->GR1.CBA2R[cb] = SMMU_CBA2R_VA64;

    SMMU->TCB[cb].CB_TCR =
        (SMMU_TCR_EPD1 | SMMU_TCR_SH0_OUTER | SMMU_TCR_ORGN0_WBWA |
         SMMU_TCR_IRGN0_WBWA | SMMU_TCR_T0SZ_256TB);

    switch (granule) {
    case MMU500_GRANULE_4KB:
        SMMU->TCB[cb].CB_TCR |= SMMU_TCR_TG0_4KB;
        break;
    case MMU500_GRANULE_16KB:
        SMMU->TCB[cb].CB_TCR |= SMMU_TCR_TG0_16KB;
        break;
    case MMU500_GRANULE_64KB:
        SMMU->TCB[cb].CB_TCR |= SMMU_TCR_TG0_64KB;
        break;
    }

    SMMU->TCB[cb].CB_TCR2 = SMMU_TCR2_SEP_47 | SMMU_TCR2_PASIZE_48;
    SMMU->TCB[cb].CB_TTBR0 =
        (((uint64_t)0) << SMMU_TTBRn_ASID_OFFSET) | ((uint64_t)base_addr);

    /*
     * AttrIndx[2:0]: setting
     *   2'b000: 0xFF=Outer write-back,non-transient normal memory,
     *                read-write/Inner write-back.non-transient normal
     * memory,read-write 2'b001: 0x00=Device-nGnRnE 2'b010-111: (not used)
     */
    temp = 0x00000000000000ffLLU;
    SMMU->TCB[cb].CB_MAIR0 = (uint32_t)temp;
    SMMU->TCB[cb].CB_MAIR1 = (uint32_t)(temp >> 32);

    SMMU->TCB[cb].CB_FSR = 0xffffffff;

    SMMU->TCB[cb].CB_SCTLR =
        (SMMU_SCTLR_MEM_NORMAL | SMMU_SCTLR_UWXN | SMMU_SCTLR_CFIE |
         SMMU_SCTLR_CFRE | SMMU_SCTLR_M);
}

void SMMU_s_disable(MMU500_Type_t *SMMU, uint32_t num_context)
{
    uint32_t i;

    if ((num_context > (SMMU->GR0.IDR1 & SMMU_IDR1_NUMCB)) ||
        (num_context > (SMMU->GR0.IDR0 & SMMU_IDR0_NUMSMRG))) {
        /* num_context too large */
        return;
    }

    SMMU->GR0.ACR &= ~(
        SMMU_ACR_S2CRB_TLBEN | SMMU_ACR_MMUDISB_TLBEN | SMMU_ACR_SMTNMB_TLBEN);

    for (i = 0; i < num_context; i++)
        SMMU->TCB[i].CB_SCTLR &= ~(SMMU_SCTLR_M);
}
