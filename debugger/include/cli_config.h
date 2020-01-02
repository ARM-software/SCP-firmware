/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CLI_CONFIG_H_
#define _CLI_CONFIG_H_

#include <fwk_status.h>

/*
 * Descriptions of console configuration macros.
 *
 * CLI_CONFIG_COMMAND_BUF_SIZE
 *   The maximum number of characters that can be entered as a single command.
 *
 * CLI_CONFIG_MAX_NUM_ARGUMENTS
 *   The maximum number of individual arguments in a single command.
 *
 * CLI_CONFIG_HISTORY_LENGTH
 *   The number of past commands stored by the console.  Can be accessed with
 *   the up/down arrow keys.
 *
 * CLI_CONFIG_PROMPT_BUF_SIZE
 *   Size of the CLI prompt text buffer.
 *
 * CLI_CONFIG_DEFAULT_TERM_W
 *   Default assumed terminal window width.
 *
 * CLI_CONFIG_DEFAULT_TERM_H
 *   Default assumed terminal window height.
 *
 * CLI_CONFIG_STACK_SIZE
 *   Number of stack bytes requested when thread is defined.
 *
 * CLI_CONFIG_PRINT_BUFFER_SIZE
 *   Size of the debug print buffer used to store characters before they can
 *   be sent to the UART.
 *
 * CLI_CONFIG_SCRATCH_BUFFER_SIZE
 *   Number of stack bytes used as scratch space by print statements, size of
 *   this buffer determines the maximum length of a single print.  Threads using
 *   CLI print functionality must have this much extra stack space.
 */

#define CLI_CONFIG_COMMAND_BUF_SIZE (256)
#define CLI_CONFIG_MAX_NUM_ARGUMENTS (16)
#define CLI_CONFIG_HISTORY_LENGTH (16)
#define CLI_CONFIG_PROMPT_BUF_SIZE (16)
#define CLI_CONFIG_DEFAULT_TERM_W (80)
#define CLI_CONFIG_DEFAULT_TERM_H (24)
#define CLI_CONFIG_STACK_SIZE (2048)
#define CLI_CONFIG_PRINT_BUFFER_SIZE (1024)
#define CLI_CONFIG_SCRATCH_BUFFER_SIZE (256)

#define CLI_PROMPT  "> "

#endif /* _CLI_CONFIG_H_ */
