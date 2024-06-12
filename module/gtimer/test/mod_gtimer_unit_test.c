/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <gtimer_reg.h>

#include <fwk_time.h>

#include UNIT_TEST_SRC

struct cntbase_reg hw_timer_UT = { .PCTH = 0, .PCTL = 2 };

struct mod_gtimer_dev_config config_UT = {
    .frequency = 2,
};

void setUp(void)
{
    memset(&mod_gtimer_ctx, 0, sizeof(mod_gtimer_ctx));
}

void tearDown(void)
{
}

void test_mod_gtimer_timestamp_not_initialised(void)
{
    fwk_timestamp_t timestamp;

    mod_gtimer_ctx.initialized = false;

    timestamp = mod_gtimer_timestamp(&config_UT);

    TEST_ASSERT_EQUAL(0, timestamp);
}

void test_mod_gtimer_timestamp_initialised(void)
{
    fwk_timestamp_t timestamp;
    fwk_timestamp_t result;
    uint32_t counter;

    mod_gtimer_ctx.initialized = true;

    config_UT.hw_timer = (uintptr_t)&hw_timer_UT;

    counter = hw_timer_UT.PCTL;
    result = ((FWK_S(1) / config_UT.frequency) * counter);

    timestamp = mod_gtimer_timestamp(&config_UT);

    TEST_ASSERT_EQUAL(result, timestamp);
}

int gtimer_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_mod_gtimer_timestamp_not_initialised);
    RUN_TEST(test_mod_gtimer_timestamp_initialised);
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return gtimer_test_main();
}
#endif
