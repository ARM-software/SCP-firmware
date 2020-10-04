/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_DEF_H
#define RCAR_DEF_H

/* Reset */
#define RCAR_CPGWPR         UINT32_C(0xE6150900)   /* CPG write protect    */
#define RCAR_MODEMR         UINT32_C(0xE6160060)   /* Mode pin             */
#define RCAR_CA57RESCNT     UINT32_C(0xE6160040)   /* Reset control A57    */
#define RCAR_CA53RESCNT     UINT32_C(0xE6160044)   /* Reset control A53    */
#define RCAR_SRESCR         UINT32_C(0xE6160110)   /* Soft Power On Reset  */
#define RCAR_CA53WUPCR      UINT32_C(0xE6151010)   /* Wake-up control A53  */
#define RCAR_CA57WUPCR      UINT32_C(0xE6152010)   /* Wake-up control A57  */
#define RCAR_CA53PSTR       UINT32_C(0xE6151040)   /* Power status A53     */
#define RCAR_CA57PSTR       UINT32_C(0xE6152040)   /* Power status A57     */
#define RCAR_CA53CPU0CR     UINT32_C(0xE6151100)   /* CPU control  A53     */
#define RCAR_CA57CPU0CR     UINT32_C(0xE6152100)   /* CPU control  A57     */
#define RCAR_CA53CPUCMCR    UINT32_C(0xE6151184)   /* Common power A53     */
#define RCAR_CA57CPUCMCR    UINT32_C(0xE6152184)   /* Common power A57     */
#define RCAR_WUPMSKCA57     UINT32_C(0xE6180014)   /* Wake-up mask A57     */
#define RCAR_WUPMSKCA53     UINT32_C(0xE6180018)   /* Wake-up mask A53     */

/* SYSC */
#define RCAR_PWRSR3         UINT32_C(0xE6180140)   /* Power stat A53-SCU   */
#define RCAR_PWRSR5         UINT32_C(0xE61801C0)   /* Power stat A57-SCU   */
#define RCAR_SYSCIER        UINT32_C(0xE618000C)   /* Interrupt enable     */
#define RCAR_SYSCIMR        UINT32_C(0xE6180010)   /* Interrupt mask       */
#define RCAR_SYSCSR         UINT32_C(0xE6180000)   /* SYSC status          */
#define RCAR_PWRONCR3       UINT32_C(0xE618014C)   /* Power resume A53-SCU */
#define RCAR_PWRONCR5       UINT32_C(0xE61801CC)   /* Power resume A57-SCU */
#define RCAR_PWROFFCR3      UINT32_C(0xE6180144)   /* Power shutof A53-SCU */
#define RCAR_PWROFFCR5      UINT32_C(0xE61801C4)   /* Power shutof A57-SCU */
#define RCAR_PWRER3         UINT32_C(0xE6180154)   /* shutoff/resume error */
#define RCAR_PWRER5         UINT32_C(0xE61801D4)   /* shutoff/resume error */
#define RCAR_SYSCISR        UINT32_C(0xE6180004)   /* Interrupt status     */
#define RCAR_SYSCISCR       UINT32_C(0xE6180008)   /* Interrupt stat clear */

/* Product register */
#define RCAR_PRR                UINT32_C(0xFFF00044)
#define RCAR_PRODUCT_MASK       UINT32_C(0x00007F00)
#define RCAR_CUT_MASK           UINT32_C(0x000000FF)
#define RCAR_PRODUCT_H3         UINT32_C(0x00004F00)
#define RCAR_PRODUCT_M3         UINT32_C(0x00005200)
#define RCAR_PRODUCT_M3N        UINT32_C(0x00005500)
#define RCAR_PRODUCT_E3         UINT32_C(0x00005700)
#define RCAR_CUT_VER10          UINT32_C(0x00000000)
#define RCAR_CUT_VER11          UINT32_C(0x00000001)   /* H3/M3N/E3 Ver.1.1 */
#define RCAR_M3_CUT_VER11       UINT32_C(0x00000010)   /* M3 Ver.1.1/Ver.1.2 */
#define RCAR_CUT_VER20          UINT32_C(0x00000010)
#define RCAR_CUT_VER30          UINT32_C(0x00000020)
#define RCAR_MAJOR_MASK         UINT32_C(0x000000F0)
#define RCAR_MINOR_MASK         UINT32_C(0x0000000F)
#define RCAR_PRODUCT_SHIFT      UINT32_C(8)
#define RCAR_MAJOR_SHIFT        UINT32_C(4)
#define RCAR_MINOR_SHIFT        UINT32_C(0)
#define RCAR_MAJOR_OFFSET       UINT32_C(1)
#define RCAR_M3_MINOR_OFFSET    UINT32_C(2)
#define RCAR_PRODUCT_H3_CUT10   (RCAR_PRODUCT_H3 | UINT32_C(0x00)) /* 1.0 */
#define RCAR_PRODUCT_H3_CUT11   (RCAR_PRODUCT_H3 | UINT32_C(0x01)) /* 1.1 */
#define RCAR_PRODUCT_H3_CUT20   (RCAR_PRODUCT_H3 | UINT32_C(0x10)) /* 2.0 */
#define RCAR_PRODUCT_M3_CUT10   (RCAR_PRODUCT_M3 | UINT32_C(0x00)) /* 1.0 */
#define RCAR_PRODUCT_M3_CUT11   (RCAR_PRODUCT_M3 | UINT32_C(0x10))
#define RCAR_CPU_MASK_CA57      UINT32_C(0x80000000)
#define RCAR_CPU_MASK_CA53      UINT32_C(0x04000000)
#define RCAR_CPU_HAVE_CA57      UINT32_C(0x00000000)
#define RCAR_CPU_HAVE_CA53      UINT32_C(0x00000000)
#define RCAR_SSCG_MASK          UINT32_C(0x1000)   /* MD12 */
#define RCAR_SSCG_ENABLE        UINT32_C(0x1000)

/* MD pin information */
#define MODEMR_BOOT_PLL_MASK    UINT32_C(0x00006000)
#define MODEMR_BOOT_PLL_SHIFT   UINT32_C(13)
/* MODEMR PLL masks and bitfield values */
#define CHECK_MD13_MD14         UINT32_C(0x6000)
#define MD14_MD13_TYPE_0        UINT32_C(0x0000)   /* MD14=0 MD13=0 */
#define MD14_MD13_TYPE_1        UINT32_C(0x2000)   /* MD14=0 MD13=1 */
#define MD14_MD13_TYPE_2        UINT32_C(0x4000)   /* MD14=1 MD13=0 */
#define MD14_MD13_TYPE_3        UINT32_C(0x6000)   /* MD14=1 MD13=1 */
/* Frequency of EXTAL(Hz) */
#define EXTAL_MD14_MD13_TYPE_0  UINT32_C(8333300)  /* MD14=0 MD13=0 */
#define EXTAL_MD14_MD13_TYPE_1  UINT32_C(10000000) /* MD14=0 MD13=1 */
#define EXTAL_MD14_MD13_TYPE_2  UINT32_C(12500000) /* MD14=1 MD13=0 */
#define EXTAL_MD14_MD13_TYPE_3  UINT32_C(16666600) /* MD14=1 MD13=1 */
#define EXTAL_SALVATOR_XS       UINT32_C(8320000)  /* Salvator-XS */
#define EXTAL_EBISU             UINT32_C(24000000) /* Ebisu */

/* CPG Secure Module Stop Control Register 9 */
#define SCMSTPCR9               (CPG_BASE + 0x0B44U)
/* CPG stop status 9 */
#define CPG_MSTPSR9             (CPG_BASE + 0x09A4U)

#endif /* RCAR_DEF_H */
