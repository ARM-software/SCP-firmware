/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2024, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "arch_gic.h"
#include "mmio.h"
#include "rcar_irq.h"
#include "rcar_mmap.h"
#include "task.h"

uint32_t c_interrupt;

/* ARM Generic Timer */
#define CORE0_TIMER_IRQCNTL ((volatile uint32_t *)(0x40000040))
static uint32_t timer_cntfrq = 0;
static uint32_t timer_tick = 0;

static void init_generic_timer(void)
{
    uint32_t reg;
    uint32_t reg_cntfid;
    uint32_t modemr;
    uint32_t modemr_pll;

    const uint32_t pll_table[] = {
        EXTAL_MD14_MD13_TYPE_0, /* MD14/MD13 : 0b00 */
        EXTAL_MD14_MD13_TYPE_1, /* MD14/MD13 : 0b01 */
        EXTAL_MD14_MD13_TYPE_2, /* MD14/MD13 : 0b10 */
        EXTAL_MD14_MD13_TYPE_3 /* MD14/MD13 : 0b11 */
    };

    modemr = mmio_read_32(RCAR_MODEMR);
    modemr_pll = (modemr & MODEMR_BOOT_PLL_MASK);

    /* Set frequency data in CNTFID0 */
    reg_cntfid = pll_table[modemr_pll >> MODEMR_BOOT_PLL_SHIFT];
    reg = mmio_read_32(RCAR_PRR) & (RCAR_PRODUCT_MASK | RCAR_CUT_MASK);
    switch (modemr_pll) {
    case MD14_MD13_TYPE_0:
#ifdef SALVATORE_XS
        reg_cntfid = EXTAL_SALVATOR_XS;
#endif
        break;
    case MD14_MD13_TYPE_3:
        if (RCAR_PRODUCT_H3_CUT10 == reg) {
            reg_cntfid = reg_cntfid >> 1U;
        }
        break;
    default:
        /* none */
        break;
    }
    /* Update memory mapped and register based freqency */
    __asm__ volatile("msr cntfrq_el0, %0" ::"r"(reg_cntfid));
}

void disable_cntv(void)
{
    uint32_t cntv_ctl;
    cntv_ctl = 0;
    __asm__ volatile("msr cntv_ctl_el0, %0" ::"r"(cntv_ctl));
}
/*-----------------------------------------------------------*/

void enable_cntv(void)
{
    uint32_t cntv_ctl;
    cntv_ctl = 1;
    __asm__ volatile("msr cntv_ctl_el0, %0" ::"r"(cntv_ctl));
}
/*-----------------------------------------------------------*/

void write_cntv_tval(uint32_t val)
{
    __asm__ volatile("msr cntv_tval_el0, %0" ::"r"(val));
    return;
}
/*-----------------------------------------------------------*/

uint32_t read_cntfrq(void)
{
    uint32_t val;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(val));
    return val;
}
/*-----------------------------------------------------------*/

uint32_t read_cntv_tval(void)
{
    uint32_t val;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(val));
    return val;
}
/*-----------------------------------------------------------*/

void init_timer(void)
{
    timer_cntfrq = timer_tick = read_cntfrq();
    /* clear cntv interrupt and set next 1 sec timer. */
    write_cntv_tval(timer_cntfrq);
    return;
}
/*-----------------------------------------------------------*/

void timer_set_tick_rate_hz(uint32_t rate)
{
    timer_tick = timer_cntfrq / rate;
    write_cntv_tval(timer_tick);
}
/*-----------------------------------------------------------*/

void vConfigureTickInterrupt(void)
{
    /* disable timer */
    disable_cntv();

    /* init timer device. */
    init_generic_timer();
    init_timer();

    /* set tick rate. */
    timer_set_tick_rate_hz(configTICK_RATE_HZ);

    /* start & enable interrupts in the timer. */
    enable_cntv();
}
/*-----------------------------------------------------------*/

void vClearTickInterrupt(void)
{
    /* clear cntv interrupt and set next timer. */
    write_cntv_tval(timer_tick);
    return;
}
/*-----------------------------------------------------------*/

void vApplicationIRQHandler(void)
{
    uint32_t ulInterruptID;

    c_interrupt = mmio_read_32(RCAR_GICC_BASE + GICC_IAR);
    mmio_write_32(RCAR_GICC_BASE + GICC_EOIR, c_interrupt);
    ulInterruptID = c_interrupt & 0x00000FFFUL;

    /* call handler function */
    if (ulInterruptID == VIRTUAL_TIMER_IRQ) {
        /* Generic Timer */
        FreeRTOS_Tick_Handler();
    } else {
        /* Peripherals */
        irq_global(ulInterruptID);
    }
    c_interrupt = 0;
}
