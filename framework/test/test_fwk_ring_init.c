/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_macros.h>
#include <fwk_ring.h>
#include <fwk_test.h>

#include <assert.h>

static void test_fwk_ring_init(void)
{
    char storage[4];

    struct fwk_ring ring;

    fwk_ring_init(&ring, storage, sizeof(storage));

    assert(fwk_ring_get_capacity(&ring) == sizeof(storage));
    assert(fwk_ring_get_length(&ring) == 0);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_ring_init),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_ring_init",

    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
