/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"
#include <mod_perf_controller.h>
#include <internal/perf_controller.h>

#include UNIT_TEST_SRC


void setUp(void)
{

}

void tearDown(void)
{

}

int perf_controller_test_main(void)
{
    UNITY_BEGIN();

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return perf_controller_test_main();
}
#endif
