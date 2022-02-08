/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_UT_UTILS_H
#define SCP_UT_UTILS_H

#define UNITY_UT_TEST

extern int _ut_unit_status;
extern int _ut_fail_line_no;

#define UT_FAILED  (1U)
#define UT_SUCCESS (0U)
#define UT_UTILS_MARK_FAILED() \
    do { \
        _ut_unit_status = UT_FAILED; \
        _ut_fail_line_no = __LINE__; \
    } while (0)

#define UT_GET_STATUS()      (_ut_unit_status)
#define UT_GET_FAILED_LINE() (_ut_fail_line_no)

#endif /* SCP_UT_UTILS_H */
