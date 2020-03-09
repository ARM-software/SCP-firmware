/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*****************************************************************************/
/*                                                                           */
/* Adding New Commands                                                       */
/*                                                                           */
/* Each new command must have, at minimum, 3 components: a string that is    */
/* typed to invoke it (<name>_call), a string describing how to use the      */
/* command (<name>_help), and a function that implements it (<name>_f).      */
/*                                                                           */
/* After implementing these three things, you need to go to the bottom of    */
/* this file and add them to the cli_commands array.                         */
/*                                                                           */
/* <name>_call                                                               */
/*   This should be only lower case letters, no spaces/special characters.   */
/*                                                                           */
/* <name>_help                                                               */
/*   This can contain any characters you want and be as long as you want. It */
/*   should explain what the command does and describe each argument. Each   */
/*   line of this string must start with 2 spaces so help printouts are      */
/*   uniformly indented and easy to read.                                    */
/*                                                                           */
/* <name>_f                                                                  */
/*   The prototype is cli_ret_et <name>_f(int32_t argc, char **argv).  The   */
/*   parameter argv is an array of strings containing arguments, and argc is */
/*   the number of arguments in argv, so very similar to argc and argc in a  */
/*   typical C main function.  The first argument is always the command name */
/*   and then following arguments are what the user typed.  Each argument is */
/*   guaranteed to be a null terminated array of characters, so it is safe   */
/*   use functions such as strcmp that depend on this.                       */
/*                                                                           */
/* Useful APIs                                                               */
/*   cli_print and cli_printf                                                */
/*     Non-formatted and formatted print functions used to write text to the */
/*     console, see cli_module.h for full descriptions.                      */
/*   cli_getline                                                             */
/*     Retrieves a line of user input from the console, see cli_module.h for */
/*     full description.                                                     */
/*   cli_platform_uart_get and cli_platform_uart_put                         */
/*     Direct access to the UART hardware, using these is not recommended    */
/*     but shouldn't hurt anything.                                          */
/*   cli_snprintf                                                            */
/*     Takes the place of snprintf and it's derivatives, a bit rudimentary   */
/*     but has no heap dependence, see cli_module.h for full descriptions.   */
/*   Most C Library Functions                                                */
/*     Most of the C library is accessible, barring functions that depend on */
/*     having heap access. (snprintf and derivatives, sscanf and derivatives,*/
/*     etc.)                                                                 */
/*                                                                           */
/*****************************************************************************/

#include <cli.h>
#include <cli_config.h>
#include <cli_fifo.h>
#include <cli_platform.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * dump_memory
 * Reads the contents of a memory region and prints them to the terminal.
 */
static const char dump_memory_call[] = "dumpmem";
static const char dump_memory_help[] =
    "  Reads raw bytes from memory and displays it on the screen.\n"
    "    Usage: dumpmem <base address> <number of bytes to read>\n"
    "      Base address and size must be on 8 byte boundaries.\n";
#define NUM_BYTES_PER_LINE (8)
static int32_t dump_memory_f(int32_t argc, char **argv)
{
    uint8_t bytes[8] = { 0 };

    /*
     * Reads aligned to 8 byte bondaries so remove lower 3 bits of address and
     * size parameters.
     */
    uint32_t addr = (uint32_t)(strtoul(argv[1], 0, 0) & 0xFFFFFFF8);
    uint32_t size = (uint32_t)(strtoul(argv[2], 0, 0) & 0x000003F8);
    uint32_t i = 0;
    uint32_t j = 0;

    /* Sanity check. */
    if (size == 0)
        return FWK_E_PARAM;

    cli_printf(NONE, "Reading %d bytes from 0x%08x.\n", size, addr);

    /* Loop through all bytes. */
    for (i = 0; i < size; i = i + NUM_BYTES_PER_LINE) {
        /* Read line worth of bytes from EEPROM. */
        memcpy((void *)bytes, (void *)(addr + i), NUM_BYTES_PER_LINE);

        /* Print line base address. */
        cli_printf(NONE, "0x%08x", addr + i);

        /* Print hex bytes. */
        for (j = 0; j < NUM_BYTES_PER_LINE; j++)
            cli_printf(NONE, " %02x", bytes[j]);

        /* Print ASCII representation. */
        cli_print(" \"");
        for (j = 0; j < NUM_BYTES_PER_LINE; j++) {
            if ((bytes[j] >= 0x20) && (bytes[j] <= 0x7E))
                /* Character is printing. */
                cli_platform_uart_put((const char *)&bytes[j], true);
            else
                /* Character is non-printing so put a period. */
                cli_platform_uart_put(".", true);
        }
        cli_print("\"\n");
    }

    return FWK_SUCCESS;
}

/*
 * Cycle Memory
 * Cycle the memory reads for up to 256 words and find the rate of change or
 * deviations for given iteration
 */
#define MAX_CYCLE_BUFFER_SZ 256
volatile uint32_t cycle_buffer[MAX_CYCLE_BUFFER_SZ] = { 0 };

static const char cycle_memory_call[] = "cyclemem";
static const char cycle_memory_help[] =
    "  Cycle memory for given iterations and displays data shifts.\n"
    "   Usage: cyclemem <base address in hex> <num of words in decimal (max "
    "256> <number of iterations to read in decimal>\n"
    "   Base address and size must be on 4 byte boundaries.\n";
static int32_t cycle_memory_f(int32_t argc, char **argv)
{
    uint32_t addr = (uint32_t)(strtoul(argv[1], 0, 0) & 0xFFFFFFF8);
    uint32_t size = (uint32_t)strtoul(argv[2], 0, 0);
    uint32_t iterations = (uint32_t)strtoul(argv[3], 0, 0);
    volatile uint32_t *tmp_address = (volatile uint32_t *)addr;
    uint32_t deviation_count = 0;
    uint32_t cycle_count, index = 0;

    /* Sanity check. */
    if ((size == 0) || (size > MAX_CYCLE_BUFFER_SZ))
        return FWK_E_PARAM;

    memset((void *)cycle_buffer, 0, sizeof(uint32_t) * MAX_CYCLE_BUFFER_SZ);

    cli_printf(
        NONE,
        "Cycle Compare Starts for 0x%08x for Length 0x%08x for iterations "
        "%d.\n",
        addr,
        size,
        iterations);

    /* Snapshot the current state */
    for (index = 0; index < size; index++)
        cycle_buffer[index] = tmp_address[index];

    /* Loop through all bytes. */
    for (cycle_count = 0; cycle_count < iterations; cycle_count++) {
        for (index = 0; index < size; index++) {
            if (tmp_address[index] != cycle_buffer[index]) {
                cycle_buffer[index] = tmp_address[index];
                deviation_count++;
            }
        }
    }

    cli_printf(NONE, "Cycle Compare Deviation Count %d \r\n", deviation_count);

    return FWK_SUCCESS;
}

/*
 * read_memory
 * Reads a value from memory and displays it on the terminal.
 */
static const char read_memory_call[] = "readmem";
static const char read_memory_help[] =
    "  Reads a value from memory.\n"
    "    Usage: readmem <address> <width in bytes 1|2|4|8>\n"
    "  WARNING: READING FROM ILLEGAL ADDRESSES CAN CRASH THE SYSTEM.";
static int32_t read_memory_f(int32_t argc, char **argv)
{
    /* Checking for the correct number of arguments. */
    if (argc != 3)
        return FWK_E_PARAM;

    /* Getting address of access. */
    uintptr_t address = (uintptr_t)strtoul(argv[1], 0, 0);

    /* Getting width of access and making sure it is valid. */
    uint32_t width = strtoul(argv[2], 0, 0);
    if (width != 1 && width != 2 && width != 4 && width != 8)
        return FWK_E_PARAM;

    /* Switching based on width. */
    cli_print("Value: 0x");
    switch (width) {
    case 1:
        /* No boundary restrictions on single byte accesses. */
        cli_printf(NONE, "%02x", *((uint8_t *)address));
        break;
    case 2:
        if (address % 2) {
            /* 16 bit accesses need to be aligned on 2 byte boundaries. */
            return FWK_E_PARAM;
        }
        cli_printf(NONE, "%04x", *((uint16_t *)address));
        break;
    case 4:
        if (address % 4) {
            /* 32 bit accesses need to be aligned to 4 byte boundaries. */
            return FWK_E_PARAM;
        }
        cli_printf(NONE, "%08x", *((uint32_t *)address));
        break;
    case 8:
        if (address % 4) {
            /* 64 bit accesses need to be aligned on 4 byte boundaries. */
            return FWK_E_PARAM;
        }
        cli_printf(NONE, "%016lx", *((uint64_t *)address));
        break;
    }

    cli_print("\n");

    return FWK_SUCCESS;
}

/*
 * write_memory
 * Writes either an 8, 16, 32, or 64 bit value to a memory address.
 */
static const char write_memory_call[] = "writemem";
static const char write_memory_help[] =
    "  Writes a value to memory.\n"
    "    Usage: writemem <base address> <width in bytes 1|2|4|8> <value to "
    "write>\n";
static int32_t write_memory_f(int32_t argc, char **argv)
{
    /* Checking for the correct number of arguments. */
    if (argc != 4)
        return FWK_E_PARAM;

    /* Getting address of access. */
    uintptr_t address = (uintptr_t)strtoul(argv[1], 0, 0);

    /* Getting width of access and making sure it is valid. */
    uint32_t width = strtoul(argv[2], 0, 0);
    if (width != 1 && width != 2 && width != 4 && width != 8)
        return FWK_E_PARAM;

    /* Switching based on width. */
    switch (width) {
    case 1:
        /* No boundary restrictions on single byte accesses. */
        *((uint8_t *)address) = (uint8_t)strtoul(argv[3], 0, 0);
        break;
    case 2:
        if (address % 2) {
            /* 16 bit accesses need to be aligned on 2 byte boundaries. */
            return FWK_E_PARAM;
        }
        *((uint16_t *)address) = (uint16_t)strtoul(argv[3], 0, 0);
        break;
    case 4:
        if (address % 4) {
            /* 32 bit accesses need to be aligned to 4 byte boundaries. */
            return FWK_E_PARAM;
        }
        *((uint32_t *)address) = (uint32_t)strtoul(argv[3], 0, 0);
        break;
    case 8:
        if (address % 4) {
            /* 64 bit accesses need to be aligned on 4 byte boundaries. */
            return FWK_E_PARAM;
        }
        *((uint64_t *)address) = (uint64_t)strtoull(argv[3], 0, 0);
        break;
    }

    return FWK_SUCCESS;
}

/*
 * time
 * Prints the system up time or real time.
 */
static const char uptime_call[] = "uptime";
static const char uptime_help[] = "  Prints the system uptime.";
static int32_t uptime_f(int32_t argc, char **argv)
{
    cli_timestamp_t t = { 0 };
    cli_platform_get_time(&t);
    cli_printf(
        NONE,
        "System Uptime: %02d:%02d:%02d:%02d.%02d\n",
        t.days,
        t.hours,
        t.minutes,
        t.seconds,
        t.fraction);
    return FWK_SUCCESS;
}

/*
 * reset_system
 * Performs a software reset.
 */
static const char reset_sys_call[] = "reset";
static const char reset_sys_help[] = "  Resets the system immediately.";
static int32_t reset_sys_f(int32_t argc, char **argv)
{
    cli_print("This command is not implemented.\n");
    return 0;
}

/*****************************************************************************/
/* Command Structure Array                                                   */
/*****************************************************************************/
extern const char checkpoint_call[];
extern const char checkpoint_help[];
extern int32_t checkpoint_f(int32_t argc, char **argv);

/* The last parameter in each of the commands below indicates whether the */
/* command handles its own help or not.  Right now, the PCIe/CCIX commands */
/* are the only ones that do that. */

cli_command_st cli_commands[] = {
    /* Add commands in this section. */
    { dump_memory_call, dump_memory_help, &dump_memory_f, false },
    { cycle_memory_call, cycle_memory_help, &cycle_memory_f, false },
    { read_memory_call, read_memory_help, &read_memory_f, false },
    { write_memory_call, write_memory_help, &write_memory_f, false },
    { reset_sys_call, reset_sys_help, &reset_sys_f, false },
    { uptime_call, uptime_help, &uptime_f, false },
    { checkpoint_call, checkpoint_help, &checkpoint_f, false },

    /* End of commands. */
    { 0, 0, 0 }
};
