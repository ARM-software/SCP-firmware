/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
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
 * cli_platform_uid_notify
 *   Description
 *     If system has a UID light, this function notifies it of activity on
 *     the console.
 *   Return
 *     cli_ret_et: success if it works, something else if it fails.
 */
uint32_t cli_platform_uid_notify(void);

#endif /* _CLI_PLATFORM_H_ */
