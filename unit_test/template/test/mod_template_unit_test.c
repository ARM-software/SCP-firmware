/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * mod_template_unit_test.c Template file
 * This is the minimum file setup you need to get started with Unit Testing
 */

#include "scp_unity.h"
#include "unity.h"

/*
 * You may need to include mocked fwk support, for example for identifiers:
 * #include <Mockfwk_id.h>
 * and so on.
 */

#include <config_template.h>

#include <fwk_aaa.h>
#include <fwk_bbb.h>

#include UNIT_TEST_SRC

void setUp(void)
{
}

void tearDown(void)
{
}

int template_test_main(void)
{
    UNITY_BEGIN();

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return template_test_main();
}
#endif
