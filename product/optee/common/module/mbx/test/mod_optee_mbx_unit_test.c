/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * mod_optee_mbx_unit_test.c is comming from the template file
 * This is the minimum file setup you need to get started with Unit Testing
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_module.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC

void setUp(void)
{
}

void tearDown(void)
{
}

int optee_mbx_test_main(void)
{
    UNITY_BEGIN();

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return optee_mbx_test_main();
}
#endif
