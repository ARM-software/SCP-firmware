/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

extern int scmi_test_main(void);

int plat_execute_all_tests(void)
{
    return scmi_test_main();
}
