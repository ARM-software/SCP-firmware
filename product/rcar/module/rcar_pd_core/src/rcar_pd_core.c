/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>
#include <rcar_common.h>
#include <rcar_mmap.h>
#include <rcar_pd_core.h>

#include <mod_rcar_pd_core.h>

#include <fwk_assert.h>
#include <fwk_status.h>

#include <stddef.h>

void rcar_pwrc_cpuoff(unsigned int core)
{
    uintptr_t off_reg;
    uint32_t cpu_no;

    if (core & CLUSTER_CPU_MASK) {
        /* A53 side                */
        off_reg = (uintptr_t)RCAR_CA53CPU0CR;
        cpu_no = (core & (~CLUSTER_CPU_MASK));
    } else {
        /* A57 side                */
        off_reg = (uintptr_t)RCAR_CA57CPU0CR;
        cpu_no = core;
    }
    mmio_write_32(RCAR_CPGWPR, ~((uint32_t)CPU_PWR_OFF));
    mmio_write_32(off_reg + (cpu_no * 0x0010U), (uint32_t)CPU_PWR_OFF);
}

void rcar_pwrc_cpuon(uint32_t core)
{
    uintptr_t res_reg;
    uint32_t res_data;
    uintptr_t on_reg;
    uint32_t upper_value;
    uint32_t wup_data;
    uint32_t cpu_no;

    if (core & CLUSTER_CPU_MASK) {
        /* A53 side                */
        res_reg = (uintptr_t)RCAR_CA53RESCNT;
        on_reg = (uintptr_t)RCAR_CA53WUPCR;
        upper_value = 0x5A5A0000U;
        cpu_no = (core & (~CLUSTER_CPU_MASK));
        ;
    } else {
        /* A57 side                */
        res_reg = (uintptr_t)RCAR_CA57RESCNT;
        on_reg = (uintptr_t)RCAR_CA57WUPCR;
        upper_value = 0xA5A50000U;
        cpu_no = core;
    }

    res_data = mmio_read_32(res_reg) | upper_value;
    SCU_power_up(core);
    wup_data = (uint32_t)((uint32_t)1U << cpu_no);
    mmio_write_32(RCAR_CPGWPR, ~wup_data);
    mmio_write_32(on_reg, wup_data);
    /* Dessert to CPU reset    */
    mmio_write_32(res_reg, (res_data & (~((uint32_t)1U << (3U - cpu_no)))));
}

void SCU_power_up(uint32_t core)
{
    uint32_t reg_SYSC_bit;
    uintptr_t reg_PWRONCR;
    volatile uintptr_t reg_PWRER;
    uintptr_t reg_PWRSR;
    uintptr_t reg_CPUCMCR;
    uintptr_t reg_SYSCIER = (uintptr_t)RCAR_SYSCIER;
    uintptr_t reg_SYSCIMR = (uintptr_t)RCAR_SYSCIMR;
    volatile uintptr_t reg_SYSCSR = (volatile uintptr_t)RCAR_SYSCSR;
    volatile uintptr_t reg_SYSCISR = (volatile uintptr_t)RCAR_SYSCISR;
    volatile uintptr_t reg_SYSCISCR = (volatile uintptr_t)RCAR_SYSCISCR;

    if (core < 4) {
        /* CA57-SCU    */
        reg_SYSC_bit = (uint32_t)BIT_CA57_SCU;
        reg_PWRONCR = (uintptr_t)RCAR_PWRONCR5;
        reg_PWRER = (volatile uintptr_t)RCAR_PWRER5;
        reg_PWRSR = (uintptr_t)RCAR_PWRSR5;
        reg_CPUCMCR = (uintptr_t)RCAR_CA57CPUCMCR;
    } else {
        /* CA53-SCU    */
        reg_SYSC_bit = (uint32_t)BIT_CA53_SCU;
        reg_PWRONCR = (uintptr_t)RCAR_PWRONCR3;
        reg_PWRER = (volatile uintptr_t)RCAR_PWRER3;
        reg_PWRSR = (uintptr_t)RCAR_PWRSR3;
        reg_CPUCMCR = (uintptr_t)RCAR_CA53CPUCMCR;
    }
    if ((mmio_read_32(reg_PWRSR) & (uint32_t)STATUS_PWRDOWN) != 0x0000U) {
        if (mmio_read_32(reg_CPUCMCR) != 0U) {
            mmio_write_32(reg_CPUCMCR, (uint32_t)0x00000000U);
        }
        /* set SYSCIER and SYSCIMR        */
        mmio_write_32(reg_SYSCIER, (mmio_read_32(reg_SYSCIER) | reg_SYSC_bit));
        mmio_write_32(reg_SYSCIMR, (mmio_read_32(reg_SYSCIMR) | reg_SYSC_bit));
        do {
            /* SYSCSR[1]=1?                */
            while ((mmio_read_32(reg_SYSCSR) & (uint32_t)REQ_RESUME) == 0U)
                continue;

            /* If SYSCSR[1]=1 then set bit in PWRONCRn to 1    */
            mmio_write_32(reg_PWRONCR, 0x0001U);
        } while ((mmio_read_32(reg_PWRER) & 0x0001U) != 0U);

        /* bit in SYSCISR=1 ?                */
        while ((mmio_read_32(reg_SYSCISR) & reg_SYSC_bit) == 0U)
            continue;

        /* clear bit in SYSCISR                */
        mmio_write_32(reg_SYSCISCR, reg_SYSC_bit);

        /* Check the SCU power-up            */
        while ((mmio_read_32(reg_PWRSR) & (uint32_t)STATUS_PWRUP) == 0x0000U)
            continue;
    }
}
