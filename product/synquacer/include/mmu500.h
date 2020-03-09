/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MMU500_H
#define MMU500_H

#include <fwk_macros.h>

#include <stdint.h>

#define SMMU_NUMSMRG 16
#define SMMU_NUMCB 8

typedef struct {
    FWK_RW uint32_t CR0;
    FWK_RW uint32_t SCR1;
    uint32_t reserved0[(0x010 - 0x008) / 4];
    FWK_RW uint32_t ACR;
    uint32_t reserved1[(0x020 - 0x014) / 4];
    FWK_R uint32_t IDR0;
    FWK_R uint32_t IDR1;
    FWK_R uint32_t IDR2;
    uint32_t reserved2[(0x03C - 0x02C) / 4];
    FWK_R uint32_t IDR7;
    FWK_RW uint32_t GFAR_L;
    FWK_RW uint32_t GFAR_U;
    FWK_RW uint32_t GFSR;
    FWK_W uint32_t GFSRRESTORE;
    FWK_RW uint32_t GFSYNR0;
    FWK_RW uint32_t GFSYNR1;
    uint32_t reserved3[(0x060 - 0x058) / 4];
    FWK_W uint32_t STLBIALL;
    FWK_W uint32_t TLBIVMID;
    FWK_W uint32_t TLBIALLNSNH;
    uint32_t reserved5;
    FWK_W uint32_t TLBGSYNC;
    FWK_R uint32_t TLBGSTATUS;
    uint32_t reserved6[(0x080 - 0x078) / 4];
    FWK_RW uint32_t DBGRPTR;
    FWK_R uint32_t DBGRDATA;
    uint32_t reserved7[(0x800 - 0x088) / 4];
    FWK_RW uint32_t SMR[SMMU_NUMSMRG];
    uint32_t reserved8[(0xC00 - (0x800 + SMMU_NUMSMRG * 4)) / 4];
    FWK_RW uint32_t S2CR[SMMU_NUMSMRG];
    uint32_t reserved9[(0x1000 - (0xC00 + SMMU_NUMSMRG * 4)) / 4];
} GSPACE0_Type_t;

typedef struct {
    FWK_RW uint32_t CBAR[SMMU_NUMCB];
    uint32_t reserved0[(0x400 - (0x000 + SMMU_NUMCB * 4)) / 4];
    FWK_RW uint32_t CBFRSYNRA[SMMU_NUMCB];
    uint32_t reserved1[(0x800 - (0x400 + SMMU_NUMCB * 4)) / 4];
    FWK_RW uint32_t CBA2R[SMMU_NUMCB];
    uint32_t reserved2[(0x1000 - (0x800 + SMMU_NUMCB * 4)) / 4];
} GSPACE1_Type_t;

typedef struct {
    FWK_RW uint32_t CB_SCTLR;
    FWK_RW uint32_t CB_ACTLR;
    FWK_W uint32_t CB_RESUME;
    uint32_t reserved0;
    FWK_RW uint32_t CB_TCR2;
    uint32_t reserved1[(0x20 - 0x14) / 4];
    FWK_RW uint64_t CB_TTBR0;
    FWK_RW uint64_t CB_TTBR1;
    FWK_RW uint32_t CB_TCR;
    FWK_RW uint32_t CB_CONTEXTIDR;
    FWK_RW uint32_t CB_MAIR0;
    FWK_RW uint32_t CB_MAIR1;
    uint32_t reserved2[(0x50 - 0x40) / 4];
    FWK_RW uint64_t CB_PAR;
    FWK_RW uint32_t CB_FSR;
    FWK_W uint32_t CB_FSRRESTORE;
    FWK_RW uint64_t CB_FAR;
    FWK_RW uint32_t CB_FSYNC0;
    FWK_RW uint32_t CB_FSYNC1;
    uint32_t reserved3[(0x1000 - 0x70) / 4];
} TCB_Type_t;

typedef struct {
    GSPACE0_Type_t GR0;
    GSPACE1_Type_t GR1;
    uint32_t reserved2[(0x3000 - 0x2000) / 4];
    uint32_t reserved3[(0x4000 - 0x3000) / 4];
    FWK_RW uint32_t SSDR[256];
    uint32_t reserved4[(0x8000 - 0x4400) / 4];
    TCB_Type_t TCB[SMMU_NUMCB];
} MMU500_Type_t;

typedef struct {
    unsigned stream_match_mask : 15;
    unsigned stream_match_id : 15;
    uint64_t base_addr;
} MMU500_ContextInfo_t;

//#define SMMU_BASE 0x50240000 //mmu_a2p
//#define SMMU_BASE 0x50250000 //mmu_p2a
//#define SMMU_BASE 0xd1e00000 //mmu_scb
//#define SMMU ((MMU500_Type_t *) SMMU_BASE)

/* --------------------------------- */
/* ---- Global Space Register 0 ---- */

#define SMMU_ACR_PAGESIZE_64K ((uint32_t)0x1 << 16)
#define SMMU_ACR_S2CRB_TLBEN ((uint32_t)0x1 << 10)
#define SMMU_ACR_MMUDISB_TLBEN ((uint32_t)0x1 << 9)
#define SMMU_ACR_SMTNMB_TLBEN ((uint32_t)0x1 << 8)

#define SMMU_IDR0_NUMSMRG ((uint32_t)0xff << 0)

#define SMMU_IDR1_NUMCB ((uint32_t)0xff << 0)
#define SMMU_IDR1_PAGESIZE ((uint32_t)1U << 31)

#define SMMU_CR0_SMCFCFG ((uint32_t)1 << 21)
#define SMMU_CR0_USFCFG ((uint32_t)1 << 10)
#define SMMU_CR0_GSE ((uint32_t)1 << 9)
#define SMMU_CR0_STALLD ((uint32_t)1 << 8)
#define SMMU_CR0_GCFGFIE ((uint32_t)1 << 5)
#define SMMU_CR0_GCFGFRE ((uint32_t)1 << 4)
//
#define SMMU_CR0_GFIE ((uint32_t)1 << 2)
#define SMMU_CR0_GFRE ((uint32_t)1 << 1)
#define SMMU_CR0_CLIENTPD ((uint32_t)1 << 0)

#define SMMU_SCR1_NSNUMCBO ((uint32_t)0xff << 0)
#define SMMU_SCR1_NSNUMSMRGO ((uint32_t)0xff << 8)
#define SMMU_SCR1_NSNUMCBO_OFFSET ((uint32_t)0)
#define SMMU_SCR1_NSNUMSMRGO_OFFSET ((uint32_t)8)
#define SMMU_SCR1_SPMEN ((uint32_t)1 << 27)

#define SMMU_TLBGSTATUS_GACTIVE ((uint32_t)1)

#define SMMU_SMRn_VALID ((uint32_t)1 << 31)
#define SMMU_SMRn_ID_OFFSET 0

#define SMMU_S2CRn_TYPE_CONTEXT ((uint32_t)0 << 16)
#define SMMU_S2CRn_TYPE_BYPASS ((uint32_t)1 << 16)
#define SMMU_S2CRn_TYPE_FAULT ((uint32_t)2 << 16)

#define SMMU_S2CRn_SH_DEF ((uint32_t)0 << 8)
#define SMMU_S2CRn_SH_OUTER ((uint32_t)1 << 8)
#define SMMU_S2CRn_SH_INNER ((uint32_t)2 << 8)
#define SMMU_S2CRn_SH_NON ((uint32_t)3 << 8)

#define SMMU_S2CRn_CBNDX_OFFSET 0

/* ---- Global Space Register 1 ---- */
/* --------------------------------- */
#define SMMU_CBAR_TYPE_STAGE2 ((uint32_t)0 << 16)
#define SMMU_CBAR_TYPE_STAGE1 ((uint32_t)1 << 16)
#define SMMU_CBAR_TYPE_STAGE1_2 ((uint32_t)3 << 16)
#define SMMU_CBAR_MEM_NORMAL ((uint32_t)0xf << 12)

#define SMMU_CBAR_HYPC ((uint32_t)1 << 10)

/* ------------------------------ */
#define SMMU_CBA2R_MONC ((uint32_t)1 << 1)
#define SMMU_CBA2R_VA64 ((uint32_t)1 << 0)

/* ---- Translation Context Bank --- */
/* ---- SCTLR ---------------------- */
#define SMMU_SCTLR_SH_OUTER ((uint32_t)0x1 << 22)
#define SMMU_SCTLR_MEM_NORMAL ((uint32_t)0xf << 16)
#define SMMU_SCTLR_UWXN ((uint32_t)1 << 10)
#define SMMU_SCTLR_WXN ((uint32_t)1 << 9)
#define SMMU_SCTLR_HUPCF ((uint32_t)1 << 8) // Hit Under Previous Context Faut
#define SMMU_SCTLR_CFCFG ((uint32_t)1 << 7) // Context fault Configuration
#define SMMU_SCTLR_CFIE ((uint32_t)1 << 6) // Context Fault Interrupt Enable
#define SMMU_SCTLR_CFRE ((uint32_t)1 << 5) // Context Fault Report Enable
#define SMMU_SCTLR_E ((uint32_t)1 << 4)
#define SMMU_SCTLR_A ((uint32_t)1 << 3)
#define SMMU_SCTLR_AFE ((uint32_t)1 << 2)
#define SMMU_SCTLR_TRE ((uint32_t)1 << 1)
#define SMMU_SCTLR_M ((uint32_t)1 << 0)

/* ------- TCR ----------------- */
#define SMMU_TCR_EPD1 ((uint32_t)1 << 23)
#define SMMU_TCR_A1 ((uint32_t)1 << 22)

#define SMMU_TCR_TG0_4KB ((uint32_t)0 << 14)
#define SMMU_TCR_TG0_16KB ((uint32_t)2 << 14)
#define SMMU_TCR_TG0_64KB ((uint32_t)1 << 14)

#define SMMU_TCR_SH0_NON ((uint32_t)0 << 12)
#define SMMU_TCR_SH0_RESERVED ((uint32_t)1 << 12)
#define SMMU_TCR_SH0_OUTER ((uint32_t)2 << 12)
#define SMMU_TCR_SH0_INNERN ((uint32_t)3 << 12)

#define SMMU_TCR_ORGN0_WBWA ((uint32_t)1 << 10)
#define SMMU_TCR_IRGN0_WBWA ((uint32_t)1 << 8)

#define SMMU_TCR_EPD0 ((uint32_t)1 << 7)

#define SMMU_TCR_T0SZ_4GB ((uint32_t)32 << 0)
#define SMMU_TCR_T0SZ_8GB ((uint32_t)31 << 0)
#define SMMU_TCR_T0SZ_16GB ((uint32_t)30 << 0)
#define SMMU_TCR_T0SZ_32GB ((uint32_t)29 << 0)
#define SMMU_TCR_T0SZ_64GB ((uint32_t)28 << 0)
#define SMMU_TCR_T0SZ_128GB ((uint32_t)27 << 0)
#define SMMU_TCR_T0SZ_256GB ((uint32_t)26 << 0)
#define SMMU_TCR_T0SZ_512GB ((uint32_t)25 << 0)
#define SMMU_TCR_T0SZ_1TB ((uint32_t)24 << 0)
#define SMMU_TCR_T0SZ_256TB ((uint32_t)16 << 0)

/* for stage2 */
#define SMMU_TCR_PASIZE_4GB ((uint32_t)0 << 16)
#define SMMU_TCR_PASIZE_64GB ((uint32_t)1 << 16)
#define SMMU_TCR_PASIZE_1TB ((uint32_t)2 << 16)

#define SMMU_TCR_SL0_LV3 ((uint32_t)0 << 6)
#define SMMU_TCR_SL0_LV2 ((uint32_t)1 << 6)
#define SMMU_TCR_SL0_LV1 ((uint32_t)2 << 6)

/* ------- TCR2 ----------------- */
#define SMMU_TCR2_SEP_41 ((uint32_t)3 << 15)
#define SMMU_TCR2_SEP_47 ((uint32_t)5 << 15)
#define SMMU_TCR2_PASIZE_32 ((uint32_t)0 << 0)
#define SMMU_TCR2_PASIZE_36 ((uint32_t)1 << 0)
#define SMMU_TCR2_PASIZE_40 ((uint32_t)2 << 0)
#define SMMU_TCR2_PASIZE_48 ((uint32_t)5 << 0)

/* ------------------------------ */

#define SMMU_TTBRn_ADDR_MASK_1TB_64KB ((uint64_t)0x0000ffffe0000000)
#define SMMU_TTBRn_ASID_OFFSET 48

#define SMMU_TBUID_GPU ((uint32_t)0 << 10)
#define SMMU_TBUID_ETR ((uint32_t)1 << 10)
#define SMMU_TBUID_SCP ((uint32_t)2 << 10)

#ifdef __cplusplus
extern "C" {
#endif
/**
 * MMU500 page table manipulators
 */
enum mmu500_granule {
    MMU500_GRANULE_4KB = 0,
    MMU500_GRANULE_16KB = 1,
    MMU500_GRANULE_64KB = 2
};

int32_t SMMU_s_init(
    MMU500_Type_t *SMMU,
    uint32_t num_context,
    const MMU500_ContextInfo_t *context_info,
    enum mmu500_granule granule);
int32_t SMMU_ns_init(MMU500_Type_t *SMMU, uint64_t addr);
void SMMU_ns_cb0_stage2_init(MMU500_Type_t *SMMU, uint64_t addr);
void SMMU_ns_cb0_stage1_init(MMU500_Type_t *SMMU, uint64_t addr);
void SMMU_ns_cb_stage1_init(
    MMU500_Type_t *SMMU,
    uint32_t cb,
    uint64_t addr,
    enum mmu500_granule granule);
void SMMU_s_disable(MMU500_Type_t *SMMU, uint32_t num_context);

#ifdef __cplusplus
}
#endif

#endif /* MMU500_H */
