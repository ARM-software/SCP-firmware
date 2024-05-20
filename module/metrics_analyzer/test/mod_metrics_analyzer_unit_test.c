/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include UNIT_TEST_SRC

void setUp(void)
{
}

void tearDown(void)
{
}

int metrics_analyzer_test_main(void)
{
    UNITY_BEGIN();

    return UNITY_END();
}

int main(void)
{
    return metrics_analyzer_test_main();
}
