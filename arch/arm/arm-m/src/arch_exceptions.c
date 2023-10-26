/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      ARM-M exception handlers.
 */

#include "arch_exceptions.h"

#include <fwk_attributes.h>
#include <fwk_log.h>

#include <fmw_cmsis.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define ICSR SCB->ICSR

#define HFSR SCB->HFSR
#define HARD_FAULT_FORCED() \
    ((HFSR & SCB_HFSR_FORCED_Msk) == SCB_HFSR_FORCED_Msk)

#define MMFSR (SCB->CFSR & SCB_CFSR_MEMFAULTSR_Msk)
#define MMFAR (SCB->MMFAR)
#define MMFAR_VALID() \
    ((MMFSR & SCB_CFSR_MMARVALID_Msk) == SCB_CFSR_MMARVALID_Msk)

#define BFSR         (SCB->CFSR & SCB_CFSR_BUSFAULTSR_Msk)
#define BFAR         SCB->BFAR
#define BFAR_VALID() ((BFSR & SCB_CFSR_BFARVALID_Msk) == SCB_CFSR_BFARVALID_Msk)

#define UFSR (SCB->CFSR & SCB_CFSR_USGFAULTSR_Msk)

#define SFSR         SCB->SFSR
#define SFAR         SCB->SFAR
#define SFAR_VALID() ((SFSR & SAU_SFSR_SFARVALID_Msk) == SAU_SFSR_SFARVALID_Msk)

#define HARD_FAULT      0x3U
#define MEMMANAGE_FAULT 0x4U
#define BUS_FAULT       0x5U
#define USAGE_FAULT     0x6U
#define SECURE_FAULT    0x7U

struct FWK_PACKED stacked_context {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t r14;
    uint32_t PC;
    uint32_t xPSR;
};

#if (FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR) && defined(FWK_LOG_BUFFERED)
static const char *const mmanage_fault_errors[] = {
    "Instruction access violation",
    "Data access violation",
    "Reserved",
    "MemManage fault on unstacking for a return from exception",
    "MemManage fault on stacking for exception entry",
    "MemManage fault occurred during floating-point lazy state preservation",
    "Reserved",
};

static const char *const bus_fault_errors[] = {
    "Instruction bus error",
    "Precise data bus error",
    "Imprecise data bus error",
    "BusFault on unstacking for a return from exception",
    "BusFault on stacking for exception entry",
    "BusFault occurred during floating-point lazy state preservation",
    "Reserved",
};

static const char *const usage_fault_errors[] = {
    "Undefined instruction",
    "Invalid state",
    "Invalid PC load UsageFault, caused by an invalid PC load by EXC_RETURN",
    "The processor has attempted to access a coprocessor",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "The processor has made an unaligned memory access",
    "Divide by zero",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

#    if defined(ARMV8M) && (__ARM_FEATURE_CMSE == 3U)
static const char *const secure_fault_errors[] = {
    "Invalid entry point",
    "Invalid integrity signature",
    "Invalid exception return",
    "Attribution unit violation",
    "Invalid transition",
    "Lazy state preservation error flag",
    "", /* FAR is reported separately */
    "Lazy state error flag",
};
#    endif /* ARMV8M && __ARM_FEATURE_CMSE == 3U */
#endif /* (FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR) && defined(FWK_LOG_BUFFERED) \
        */

#ifdef __NEWLIB__
/*
 * This function overloads a weak definition provided by Newlib. It is called
 * during initialization of the C runtime just after .bss has been zeroed.
 */
void software_init_hook(void)
{
    extern char __data_load__;
    extern char __data_start__;
    extern char __data_end__;

    char *load = &__data_load__;
    char *start = &__data_start__;
    char *end = &__data_end__;

    if (load != start) {
        (void)memcpy(start, load, (size_t)(end - start));
    }
}
#endif

#ifdef __ARMCC_VERSION
extern char Image$$ARM_LIB_STACKHEAP$$ZI$$Limit;

#    define arch_exception_stack (&Image$$ARM_LIB_STACKHEAP$$ZI$$Limit)
#else
extern char __stackheap_end__;

#    define arch_exception_stack (&__stackheap_end__)
#endif

#if (FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR) && defined(FWK_LOG_BUFFERED)
static void handle_hard_fault(struct stacked_context *context)
{
    FWK_LOG_ERR("HARD FAULT occured at 0x%" PRIX32, context->PC);
    if (HARD_FAULT_FORCED()) {
        FWK_LOG_ERR("Forced HARD FAULT");
    }
}

static void handle_memmanage_fault(struct stacked_context *context)
{
    size_t number_of_faults;
    uint8_t idx;

    number_of_faults = FWK_ARRAY_SIZE(mmanage_fault_errors);

    FWK_LOG_ERR("MEMMANAGE FAULT occured at 0x%" PRIX32, context->PC);
    for (idx = 0; idx < number_of_faults; idx++) {
        if (((MMFSR >> (idx + SCB_CFSR_MEMFAULTSR_Pos)) & 1U) == 1U) {
            FWK_LOG_ERR("%s", mmanage_fault_errors[idx]);
        }
    }

    if (MMFAR_VALID()) {
        FWK_LOG_ERR("MMFAR: 0x%" PRIX32, MMFAR);
    }
}

static void handle_bus_fault(struct stacked_context *context)
{
    size_t number_of_faults;
    uint8_t idx;

    number_of_faults = FWK_ARRAY_SIZE(bus_fault_errors);

    FWK_LOG_ERR("BUS FAULT occured at 0x%" PRIX32, context->PC);
    for (idx = 0; idx < number_of_faults; idx++) {
        if (((BFSR >> (idx + SCB_CFSR_BUSFAULTSR_Pos)) & 1U) == 1U) {
            FWK_LOG_ERR("%s", bus_fault_errors[idx]);
        }
    }

    if (BFAR_VALID()) {
        FWK_LOG_ERR("BFAR: 0x%" PRIX32, BFAR);
    }
}

static void handle_usage_fault(struct stacked_context *context)
{
    size_t number_of_faults;
    uint8_t idx;

    number_of_faults = FWK_ARRAY_SIZE(usage_fault_errors);

    FWK_LOG_ERR("USAGE FAULT occured at 0x%" PRIX32, context->PC);
    for (idx = 0; idx < number_of_faults; idx++) {
        if (((UFSR >> (idx + SCB_CFSR_USGFAULTSR_Pos)) & 1U) == 1U) {
            FWK_LOG_ERR("%s", usage_fault_errors[idx]);
        }
    }
}

#    if defined(ARMV8M) && (__ARM_FEATURE_CMSE == 3U)
static void handle_secure_fault(struct stacked_context *context)
{
    size_t number_of_faults;
    uint8_t idx;

    number_of_faults = FWK_ARRAY_SIZE(secure_fault_errors);

    FWK_LOG_ERR("SECURE FAULT occured at 0x%" PRIX32, context->PC);
    for (idx = 0; idx < number_of_faults; idx++) {
        if (idx == SAU_SFSR_SFARVALID_Pos) {
            continue;
        }
        if (((SFSR >> idx) & 1U) == 1U) {
            FWK_LOG_ERR("%s", secure_fault_errors[idx]);
        }
    }

    if (SFAR_VALID) {
        FWK_LOG_ERR("SFAR: 0x%" PRIX32, SFAR);
    }
}
#    endif /* ARMV8M && __ARM_FEATURE_CMSE == 3U */

static void handle_exception(struct stacked_context *context)
{
    uint32_t exception_number;

    exception_number = ICSR & SCB_ICSR_VECTACTIVE_Msk;
    switch (exception_number) {
    case HARD_FAULT:
        handle_hard_fault(context);
        break;
    case MEMMANAGE_FAULT:
        handle_memmanage_fault(context);
        break;
    case BUS_FAULT:
        handle_bus_fault(context);
        break;
    case USAGE_FAULT:
        handle_usage_fault(context);
        break;
#    if defined(ARMV8M) && (__ARM_FEATURE_CMSE == 3U)
    case SECURE_FAULT:
        handle_secure_fault(context);
        break;
#    endif /* ARMV8M && __ARM_FEATURE_CMSE == 3U */
    default:
        FWK_LOG_ERR(
            "Unhandled Fault: (%" PRIX32 ") occured at 0x%08" PRIX32,
            exception_number,
            context->PC);
        break;
    };

    FWK_LOG_ERR(
        "r0:\t0x%08" PRIX32 " r1:\t0x%08" PRIX32, context->r0, context->r1);
    FWK_LOG_ERR(
        "r2:\t0x%08" PRIX32 " r3:\t0x%08" PRIX32, context->r2, context->r3);
    FWK_LOG_ERR(
        "r12:\t0x%08" PRIX32 " r14:\t0x%08" PRIX32, context->r12, context->r14);
    FWK_LOG_ERR("PC:\t0x%08" PRIX32, context->PC);
    FWK_LOG_ERR("xPSR:\t0x%08" PRIX32, context->xPSR);
}

__attribute__((naked)) noreturn void handle_arch_exception(void)
{
    /* It is recommended not to have C code in naked function. */
    __asm(
        "mov r0, sp \n\t"
        "bl %0 \n\t"
        "b %1"
        :
        : "i"(handle_exception), "i"(arch_exception_invalid));
}
#else /* (FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR) && defined(FWK_LOG_BUFFERED) \
       */

__attribute__((naked)) noreturn void handle_arch_exception(void)
{
    __asm("b %0" : : "i"(arch_exception_invalid));
}

#endif /* (FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR) && defined(FWK_LOG_BUFFERED) \
        */

/*
 * Set up the exception table. The structure below is added to the
 * .exceptions section which will be explicitly placed at the beginning of the
 * binary by the linker script.
 */
const struct {
    uintptr_t stack;
    uintptr_t exceptions[NVIC_USER_IRQ_OFFSET - 1];
} arch_exceptions FWK_SECTION(".exceptions") = {
    .stack = (uintptr_t)(arch_exception_stack),
    .exceptions = {
        [NVIC_USER_IRQ_OFFSET + Reset_IRQn - 1] =
            (uintptr_t)(arch_exception_reset),
        [NonMaskableInt_IRQn +  (NVIC_USER_IRQ_OFFSET - 1)] =
            (uintptr_t)(arch_exception_invalid),
        [NVIC_USER_IRQ_OFFSET + HardFault_IRQn - 1] =
            (uintptr_t)(handle_arch_exception),
        [NVIC_USER_IRQ_OFFSET + MemoryManagement_IRQn - 1] =
            (uintptr_t)(handle_arch_exception),
        [NVIC_USER_IRQ_OFFSET + BusFault_IRQn - 1] =
            (uintptr_t)(handle_arch_exception),
        [NVIC_USER_IRQ_OFFSET + UsageFault_IRQn - 1] =
            (uintptr_t)(handle_arch_exception),
#ifdef ARMV8M
        [NVIC_USER_IRQ_OFFSET + SecureFault_IRQn - 1] =
            (uintptr_t)(handle_arch_exception),
#endif /* ARMV8M */
        [NVIC_USER_IRQ_OFFSET + DebugMonitor_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),

        [NVIC_USER_IRQ_OFFSET + SVCall_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),
        [NVIC_USER_IRQ_OFFSET + PendSV_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),
        [NVIC_USER_IRQ_OFFSET + SysTick_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),
    },
};
