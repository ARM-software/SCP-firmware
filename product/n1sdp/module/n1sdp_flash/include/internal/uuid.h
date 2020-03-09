/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Header file defining the structure of the Universally Unique
 *     IDentifier (UUID) used to identify different binaries within
 *     the firmware package.
 */

#ifndef INTERNAL_UUID_H
#define INTERNAL_UUID_H

#include <stdint.h>

/* Length of a node address (an IEEE 802 address). */
#define _UUID_NODE_LEN 6

/* Length of UUID string including dashes. */
#define _UUID_STR_LEN  36

/*
 * See also:
 * http://www.opengroup.org/dce/info/draft-leach-uuids-guids-01.txt
 * http://www.opengroup.org/onlinepubs/009629399/apdxa.htm
 *
 * A DCE 1.1 compatible source representation of UUIDs.
 */
struct uuid_t {
    uint32_t time_low;
    uint16_t time_mid;
    uint16_t time_hi_and_version;
    uint8_t clock_seq_hi_and_reserved;
    uint8_t clock_seq_low;
    uint8_t node[_UUID_NODE_LEN];
};

#endif /* INTERNAL_UUID_H */
