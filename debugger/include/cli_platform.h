/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CLI_PLATFORM_H_
#define _CLI_PLATFORM_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * cli_timestamp_t
 *   Description
 *     Stores timestamp information in human-readable format.
 *   Members
 *     uint32_t days
 *       Number of days elapsed since restart.
 *     uint8_t hours
 *       Hours portion of time since restart.
 *     uint8_t minutes
 *       Minutes portion of time since restart.
 *     uint8_t seconds
 *       Seconds portion of time since restart.
 *     uint8_t fraction
 *       Value from 0-99 containing hundredths of seconds.
 */
typedef struct {
    uint32_t days;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t fraction;
} cli_timestamp_t;

/*****************************************************************************/
/* CLI Platform-Specific Function Prototypes                                 */
/*****************************************************************************/

/*
 * cli_platform_get_time
 *   Description
 *     Fills out a cli_timestamp_t structure, this can be real time or time
 *     since system startup.
 *   Parameters
 *     cli_timestamp_t *t
 *       Pointer to structure to fill out.
 */
void cli_platform_get_time(cli_timestamp_t *t);

/*
 * cli_platform_delay_ms
 *   Descriptions
 *     Delays execution for a number of milliseconds.
 *   Parameters
 *     uint32_t ms
 *       Number of milliseconds to delay.
 */
void cli_platform_delay_ms(uint32_t ms);

/*****************************************************************************/
/* CLI Platform-Specific UART Functions                                      */
/*****************************************************************************/

/*
 * cli_platform_uart_init
 *   Description
 *     Initializes the necessary hardware to send and receive characters.
 *   Return
 *     cli_ret_et: success if it works, something else if it fails.
 */
uint32_t cli_platform_uart_init(void);

/*
 * cli_platform_uid_notify
 *   Description
 *     If system has a UID light, this function notifies it of activity on
 *     the console.
 *   Return
 *     cli_ret_et: success if it works, something else if it fails.
 */
uint32_t cli_platform_uid_notify(void);

/*
 * cli_platform_uart_get
 *   Description
 *     Receives a single character from the UART.  Must support blocking and
 *     non-blocking receive operations.
 *   Parameters
 *     char *c
 *       Pointer to a char in which to place the received character.
 *     bool block
 *       If true, this function must not return until a character is
 *       received or the UART generates an error.  If false, this function
 *       returns immediately regardless of whether or not a character was
 *       received.
 *   Return
 *     cli_ret_et: success if a character is read with no errors, error_empty
 *     if block==true and no characters are available, or some other error
 *     from the UART.
 */
uint32_t cli_platform_uart_get(char *c, bool block);

/*
 * cli_platform_uart_put
 *   Description
 *     Sends a single character on the UART.  This function is blocking.
 *   Parameters
 *     char *c
 *       Pointer to character to send.
 *     bool block
 *       If true, this function must not return until a character is
 *       output or the UART generates an error.  If false, this function
 *       returns immediately regardless of whether or not a character was
 *       output.
 *   Return
 *     cli_ret_et: success if it works, some other error if it doesn't.
 */
uint32_t cli_platform_uart_put(const char *c, bool block);

#endif /* _CLI_PLATFORM_H_ */
