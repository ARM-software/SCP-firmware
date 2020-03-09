/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * During PCIe bus enumeration, when a device is not connected, accessing
 * its configuration space should return 0xFFFFFFFF. However, due to
 * a bug in the PCIe controller, bus fault is asserted instead of returning
 * 0xFFFFFFFF. To avoid bus fault errors in AP cores, SCP will perform
 * a bus enumeration, identifying valid endpoints connected and mask the bus
 * fault when reading the configuration space.
 */

#include "n1sdp_scp_mmap.h"

#include <n1sdp_pcie.h>

#include <mod_n1sdp_pcie.h>

#include <fwk_assert.h>
#include <fwk_macros.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* PCIe configuration space offset definitions */
#define PCIE_HEADER_TYPE_OFFSET         0xE
#define PCIE_PRIMARY_BUS_NUM_OFFSET     0x18
#define PCIE_SECONDARY_BUS_NUM_OFFSET   0x19
#define PCIE_SUBORDINATE_BUS_NUM_OFFSET 0x1A

#define PCIE_HEADER_TYPE_MASK           0x7F
#define PCIE_HEADER_TYPE_ENDPOINT       0
#define PCIE_HEADER_TYPE_BRIDGE         1

/* BDF table offsets for PCIe & CCIX controllers */
#define PCIE_BDF_TABLE_OFFSET           0
#define CCIX_BDF_TABLE_OFFSET           (16 * FWK_KIB)

/* PCIe standard definitions */
#define PCIE_BUS_NUM_MAX                0xFF
#define DEVICES_PER_BUS_MAX             32
#define FUNCTIONS_PER_DEVICE_MAX        8

#define BDF_ADDR_SHIFT_BUS              20
#define BDF_ADDR_SHIFT_DEVICE           15
#define BDF_ADDR_SHIFT_FUNCTION         12

/* Initial bus number definitions */
#define PCIE_PRIMARY_BUS_NUM_START      1
#define PCIE_SECONDARY_BUS_NUM_START    2

#define CCIX_PRIMARY_BUS_NUM_START      1
#define CCIX_SECONDARY_BUS_NUM_START    2

/* Structure defining the BDF table */
struct bdf_table {
    /* Base address of Root Port's configuration space */
    uint32_t rp_config_base_addr;
    /* Total valid BDF entries found during bus enumeration */
    uint32_t bdf_count;
    /* BDF table entries */
    uint32_t table_data[];
};

/*
 * Global variables used during bus enumeration
 * for BDF table generation
 */
uint32_t *bdf_table_ptr;
uint32_t bdf_count;

/*!
 * \brief Performs a "checked" 32-bit load from @src@.
 *
 * \details By default, SCPv2's exception handler will make the core enter into
 *      an indefinite while loop when an exception happens.
 *      The exception handler have special code that triggers if a load on
 *      @src@ when in this function trigger a bus fault.
 *      This function mask any bus faults triggered due to read of @src@.
 *
 * \retval  true: Load was successful and @value@ is updated.
 * \retval  false: A bus fault was triggered during the load. @value@ is not
 *      updated.
 */
static bool checked_read_u32(uint32_t *const value, const uint32_t *const src);

/*
 * This function is called if link training failed to initialize the
 * BDF table with RP base address and zero BDF count such that UEFI/Linux
 * will not read garbage table values.
 */
void pcie_init_bdf_table(struct n1sdp_pcie_dev_config *config)
{
    assert(config != NULL);
    struct bdf_table *table;

    /* Set BDF table pointer based on the root complex */
    if (config->ccix_capable)
        table = (struct bdf_table *)(SCP_NONTRUSTED_RAM_BASE +
                                     CCIX_BDF_TABLE_OFFSET);
    else
        table = (struct bdf_table *)(SCP_NONTRUSTED_RAM_BASE +
                                     PCIE_BDF_TABLE_OFFSET);

    table->rp_config_base_addr = config->global_config_base -
                                     SCP_AP_AXI_OFFSET;
    table->bdf_count = 0;
}

static uint8_t pcie_bus_scan(uint32_t ecam_addr,
                             uint8_t pri_bnum,
                             uint8_t sec_bnum)
{
    int dev_num, fn_num;
    uint32_t bdf_addr;
    uint32_t vid;
    uint32_t config_addr;
    uint8_t header_type;
    uint8_t sub_bnum = pri_bnum;

    /* Loop over all devices on pri_bnum bus */
    for (dev_num = 0; dev_num < DEVICES_PER_BUS_MAX; dev_num++) {
        /*
         * Special case:
         * Ignore dev_num > 0 on bus 1 because the controller returns
         * config space of device 0 for all other device numbers on bus 1
         */
        if (((pri_bnum == PCIE_PRIMARY_BUS_NUM_START) && (dev_num != 0)) ||
            ((pri_bnum == CCIX_PRIMARY_BUS_NUM_START) && (dev_num != 0)))
            break;
        /* Loop over all functions on dev_num device */
        for (fn_num = 0; fn_num < FUNCTIONS_PER_DEVICE_MAX; fn_num++) {
            bdf_addr = (pri_bnum << BDF_ADDR_SHIFT_BUS) |
                       (dev_num << BDF_ADDR_SHIFT_DEVICE) |
                       (fn_num << BDF_ADDR_SHIFT_FUNCTION);
            config_addr = ecam_addr + bdf_addr;

            if (!checked_read_u32(&vid, (uint32_t *)config_addr)) {
                /*
                 * Exception occurred.
                 * Ignore this bus-device-function and move to next function.
                 */
                continue;
            }

            /* Valid device is identified so fill the BDF table */
            bdf_count++;
            *bdf_table_ptr++ = bdf_addr;

            /* If function 0 of any device has invalid VID break the loop */
            if ((vid & 0xFFFF) == 0xFFFF) {
                if (fn_num == 0)
                    break;
                else
                    continue;
            }

            /*
             * Read the header type to identify if the device
             * is an endpoint or a PCI-PCI bridge.
             */
            header_type = *(uint8_t *)(config_addr + PCIE_HEADER_TYPE_OFFSET);
            if ((header_type &
                     PCIE_HEADER_TYPE_MASK) == PCIE_HEADER_TYPE_BRIDGE) {
                /*
                 * PCI-PCI bridge is identified. Set primary and secondary bus
                 * numbers. Let subordinate bus number be max possible bus
                 * number as we need to further identify devices downstream.
                 */
                *(uint8_t *)(config_addr +
                    PCIE_PRIMARY_BUS_NUM_OFFSET) = pri_bnum;
                *(uint8_t *)(config_addr +
                    PCIE_SECONDARY_BUS_NUM_OFFSET) = sec_bnum;
                *(uint8_t *)(config_addr +
                    PCIE_SUBORDINATE_BUS_NUM_OFFSET) = PCIE_BUS_NUM_MAX;
                /*
                 * Recursively call the scan function with incremented
                 * primary and secondary bus numbers.
                 */
                sub_bnum = pcie_bus_scan(ecam_addr, sec_bnum, sec_bnum + 1);
                /*
                 * The recursive call has returned from an endpoint
                 * identification so use the returned bus number as the
                 * bridge's subordinate bus number.
                 */
                *(uint8_t *)(config_addr +
                    PCIE_SUBORDINATE_BUS_NUM_OFFSET) = sub_bnum;
                sec_bnum = sub_bnum + 1;
            } else {
                /*
                 * Endpoint is identified. Proceed to other functions &
                 * devices in this bus and return to previous recursive call.
                 */
                sub_bnum = sec_bnum - 1;
            }
        }
    }
    /* Return the subordinate bus number to previous recursive call */
    return sub_bnum;
}

void pcie_bus_enumeration(struct n1sdp_pcie_dev_config *config)
{
    assert(config != NULL);

    uint32_t ecam_base_addr = config->axi_slave_base32;
    uint8_t pri_bnum, sec_bnum, sub_bnum;
    struct bdf_table *table;

    /* Set BDF table pointer based on the root complex */
    if (config->ccix_capable)
        table = (struct bdf_table *)(SCP_NONTRUSTED_RAM_BASE +
                                     CCIX_BDF_TABLE_OFFSET);
    else
        table = (struct bdf_table *)(SCP_NONTRUSTED_RAM_BASE +
                                     PCIE_BDF_TABLE_OFFSET);

    pcie_init_bdf_table(config);

    bdf_count = 0;
    bdf_table_ptr = table->table_data;

    /* Start with bus number 1 as bus 0 is root bus internal to the device */
    if (config->ccix_capable) {
        pri_bnum = CCIX_PRIMARY_BUS_NUM_START;
        sec_bnum = CCIX_SECONDARY_BUS_NUM_START;
    } else {
        pri_bnum = PCIE_PRIMARY_BUS_NUM_START;
        sec_bnum = PCIE_SECONDARY_BUS_NUM_START;
    }

    /*
     * Configure primary & secondary bus numbers for root port.
     * Let sub-ordinate bus number be maximum bus number initially.
     */
    *(volatile uint8_t *)(config->global_config_base +
                 PCIE_PRIMARY_BUS_NUM_OFFSET) = pri_bnum - 1;
    *(volatile uint8_t *)(config->global_config_base +
                 PCIE_SECONDARY_BUS_NUM_OFFSET) = pri_bnum;
    *(volatile uint8_t *)(config->global_config_base +
                 PCIE_SUBORDINATE_BUS_NUM_OFFSET) = PCIE_BUS_NUM_MAX;

    sub_bnum = pcie_bus_scan(ecam_base_addr, pri_bnum, sec_bnum);

    /*
     * Update subordinate bus number with maximum bus number identified
     * from bus scan for this bus hierarchy.
     */
    *(volatile uint8_t *)(config->global_config_base +
                 PCIE_SUBORDINATE_BUS_NUM_OFFSET) = sub_bnum;

    /*
     * Make table entries to be even count by adding a dummy entry
     * for 64-bit alignment
     */
    if (bdf_count & 0x1) {
        *bdf_table_ptr++ = 0xFFFFFFFF;
        bdf_count++;
    }

    table->bdf_count = bdf_count;
}

/*!
 * \brief Callee context at exception
 */
struct __attribute((packed)) context {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t r14;
    uint32_t PC;
    uint32_t xPSR;
};

/* Interrupt Control and State Register */
#define ICSR ((FWK_R  uint32_t *)(0xE000ED04UL))
#define ICSR_VECTACTIVE UINT32_C(0x000000FF)

/*
 * Value to indicate a bus fault was masked.
 * The definitions can not use the UINT32_C() or UL decorators to allow them to
 * be used in inline assembly.
 */
#define CHECKED_READ_U32__ERROR_VALUE 0xdeadbeef

/* Bus faults at this instruction should be ignored */
#define CHECKED_READ_U32__LOAD_LOC checked_read_u32_load_loc

/* Restore PC to this label if a bus fault is triggered and should be masked */
#define CHECKED_READ_U32__POSTLOAD_LOC checked_read_u32_postload_loc

#define EXCEPTION_BUSFAULT 5

__attribute__((noinline)) /* only one location where the checked load happen */
static bool checked_read_u32(uint32_t *const value, const uint32_t *const src)
{
    uint32_t err;
    uint32_t dst;

    /*
     * If the instruction at CHECKED_READ_U32__LOAD_LOC trigger a bus fault
     * exception the custom exception handler will:
     *   1. Set register r0 to CHECKED_READ_U32__ERROR_VALUE.
     *   2. Set PC to CHECKED_READ_U32__POSTLOAD_LOC.
     *   3. Resume execution.
     */
    __asm__ volatile(
        "ldr r0, =!(" FWK_STRINGIFY(CHECKED_READ_U32__ERROR_VALUE) ");"
        FWK_STRINGIFY(CHECKED_READ_U32__LOAD_LOC)     ": ldr %1, [%2, #0];"
        FWK_STRINGIFY(CHECKED_READ_U32__POSTLOAD_LOC) ": mov %0, r0;"
        : "=r"(err), "=r"(dst)
        : "r"(src)
        : "r0");
    if (err == CHECKED_READ_U32__ERROR_VALUE)
        return false;
    *value = dst;
    return true;
}

static bool exception_handler(const int exception,
                              struct context *const context) {
    extern void CHECKED_READ_U32__LOAD_LOC(void);
    extern void CHECKED_READ_U32__POSTLOAD_LOC(void);

    if (exception == EXCEPTION_BUSFAULT &&
            (uintptr_t)&CHECKED_READ_U32__LOAD_LOC == (uintptr_t)context->PC) {
        /*
         * we got a bus fault in the "checked" read functions so lets jump to
         * the safe place and notify that we masked the fault
         */
        context->r0 = CHECKED_READ_U32__ERROR_VALUE;
        context->PC = (uintptr_t)&CHECKED_READ_U32__POSTLOAD_LOC;
        return true;
    }
    return false;
}

void arm_exception_invalid(void)
{
    struct context *context;
    __asm__ volatile(
        "tst lr, #4;"
        "ite eq;"
        "mrseq %0, msp;"
        "mrsne %0, psp;"
        : "=r"(context));

    const int exception = *ICSR & ICSR_VECTACTIVE;
    if (!exception_handler(exception, context))
        while (true)
            __WFI();
}
