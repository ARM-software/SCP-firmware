/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <checkpoint.h>

#include <cli_platform.h>
#include <cli.h>

#include <fwk_status.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern struct cli_ctx cli_ctx;

const char checkpoint_call[] = "checkpoint";
const char checkpoint_help[] =
    "  Show all threads currently using checkpoints and their task IDs.\n"
    "    Usage: checkpoint list\n"
    "  Tell a paused thread to run to the next checkpoint.\n"
    "    Usage: checkpoint <task ID> next\n"
    "  Tell a paused thread to skip N future checkpoints.\n"
    "    Usage: checkpoint <task ID> skip N\n"
    "  Tell a paused thread to run to a checkpoint tag.\n"
    "    Usage: checkpoint <task ID> tag <tag>\n"
    "  Enable or disable all checkpoints in a thread.\n"
    "    Usage: checkpoint <task ID> <enable|disable>";
int32_t checkpoint_f(int32_t argc, char **argv)
{
    uint32_t i = 0;
    uint32_t run_cnt = 0;
    uint32_t id = 0;
    struct checkpoint_config *cp;

    if ((argc == 2) && (cli_strncmp(argv[1], "list", 4) == 0)) {
        for (i = 0; i < CHECKPOINT_NUM; i++) {
            cp = cli_ctx.cp_api->get(i);
            if (cp->in_use == true)
                cli_printf(NONE, "%s\n  Task ID: %d\n", cp->name, i);
        }
        return FWK_SUCCESS;
    }

    else if ((argc == 3) && (cli_strncmp(argv[2], "next", 4) == 0)) {
        id = strtoul(argv[1], NULL, 0);
        if (id >= CHECKPOINT_NUM) {
            cli_print("Task ID out of range.\n");
            return FWK_E_PARAM;
        }
        cp = cli_ctx.cp_api->get(id);
        if (cp->pause == false) {
            cli_print("Thread is still running.\n");
            return FWK_E_STATE;
        }
        cp->tag[0] = 0;
        cp->bypass = CHECKPOINT_ENABLED;
        osThreadFlagsSet(cp->tid, CHECKPOINT_SIGNAL);
        return FWK_SUCCESS;
    }

    else if ((argc == 4) && (cli_strncmp(argv[2], "skip", 4) == 0)) {
        id = strtoul(argv[1], NULL, 0);
        if (id >= CHECKPOINT_NUM) {
            cli_print("Task ID out of range.\n");
            return FWK_E_PARAM;
        }
        cp = cli_ctx.cp_api->get(id);
        if (cp->pause == false) {
            cli_print("Thread is still running.\n");
            return FWK_E_STATE;
        }
        run_cnt = strtoul(argv[3], NULL, 0);
        cp->tag[0] = 0;
        cp->bypass = run_cnt;
        osThreadFlagsSet(cp->tid, CHECKPOINT_SIGNAL);
        return FWK_SUCCESS;
    }

    else if ((argc == 4) && (cli_strncmp(argv[2], "tag", 3) == 0)) {
        id = strtoul(argv[1], NULL, 0);
        if (id >= CHECKPOINT_NUM) {
            cli_print("Task ID out of range.\n");
            return FWK_E_PARAM;
        }
        cp = cli_ctx.cp_api->get(id);
        if (cp->pause == false) {
            cli_print("Thread is still running.\n");
            return FWK_E_STATE;
        }
        strncpy(cp->tag, argv[3], CHECKPOINT_TAG_LEN);
        cp->tag[CHECKPOINT_TAG_LEN - 1] = 0;
        cp->bypass = CHECKPOINT_DISABLED;
        osThreadFlagsSet(cp->tid, CHECKPOINT_SIGNAL);
        return FWK_SUCCESS;
    }

    else if ((argc == 3) && (cli_strncmp(argv[2], "enable", 6) == 0)) {
        id = strtoul(argv[1], NULL, 0);
        if (id >= CHECKPOINT_NUM) {
            cli_print("Task ID out of range.\n");
            return FWK_E_PARAM;
        }
        cp = cli_ctx.cp_api->get(id);
        cp->bypass = CHECKPOINT_ENABLED;
    }

    else if ((argc == 3) && (cli_strncmp(argv[2], "disable", 7) == 0)) {
        id = strtoul(argv[1], NULL, 0);
        if (id >= CHECKPOINT_NUM) {
            cli_print("Task ID out of range.\n");
            return FWK_E_PARAM;
        }
        cp = cli_ctx.cp_api->get(id);
        if (cp->pause == false) {
            cli_print("Thread is still running.\n");
            return FWK_E_STATE;
        }
        cp->bypass = CHECKPOINT_DISABLED;
        osThreadFlagsSet(cp->tid, CHECKPOINT_SIGNAL);
        return FWK_SUCCESS;
    }
    cli_print("CLI: Invalid command received:\n");
    for (i = 0; i < (uint32_t)argc; i++)
        cli_printf(NONE, "Parameter %d is %s\n", i, argv[i]);

    return FWK_E_PARAM;
}

void checkpoint_boot_state(uint32_t timeout_s)
{
    char c = 0;

    /* Make sure nothing is waiting in UART buffer. */
    while (cli_platform_uart_get(&c, false) == FWK_SUCCESS)
        ;

    cli_print("Press any key to enable checkpoints before boot... ");
    while (timeout_s != 0) {
        if (cli_platform_uart_get(&c, false) == FWK_SUCCESS) {
            cli_print("\nCheckpoints enabled.\n");
            cli_ctx.cp_api->enable_all();
            return;
        }
        cli_printf(NONE, "%c%d", 0x8, timeout_s);
        osDelay(CHECKPOINT_SECOND_MS);
        timeout_s = timeout_s - 1;
    }
    cli_printf(NONE, "%c0\n", 0x8);

    return;
}
