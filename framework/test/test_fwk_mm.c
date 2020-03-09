/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_status.h>
#include <fwk_test.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define SIZE_MEM            (1024 * 1024)
#define ALLOC_NUM           5
#define ALLOC_SIZE          64
#define ALLOC_ODD_SIZE      3
#define ALLOC_ALIGN         (alignof(max_align_t) * 2)
#define ALLOC_BAD_ALIGN     12
#define ALLOC_TOTAL_SIZE    (ALLOC_NUM * ALLOC_SIZE)
#define MEM_PATTERN         0x0A
#define ALIGN_MASK          (ALLOC_ALIGN - 1)

static void test_fwk_mm_init(void);
static void test_fwk_mm_alloc(void);
static void test_fwk_mm_alloc_aligned(void);
static void test_fwk_mm_calloc(void);
static void test_fwk_mm_calloc_aligned(void);

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_mm_init),
    FWK_TEST_CASE(test_fwk_mm_alloc),
    FWK_TEST_CASE(test_fwk_mm_alloc_aligned),
    FWK_TEST_CASE(test_fwk_mm_calloc),
    FWK_TEST_CASE(test_fwk_mm_calloc_aligned)
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_mm",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};

extern int fwk_mm_init(uintptr_t start, size_t size);
extern void fwk_mm_lock(void);

static int start[SIZE_MEM];

static void test_fwk_mm_init(void)
{
    int result;
    /* Invalid start pointer */
    result = fwk_mm_init(0, SIZE_MEM);
    assert(result == FWK_E_RANGE);

    /* Invalid size */
    result = fwk_mm_init((uintptr_t)&start, 0);
    assert(result == FWK_E_RANGE);

    result = fwk_mm_init((uintptr_t)&start, SIZE_MEM);
    assert(result == FWK_SUCCESS);

    result = fwk_mm_init((uintptr_t)&start, SIZE_MEM);
    assert(result == FWK_E_STATE);
}

static void test_fwk_mm_alloc(void)
{
    int i;
    char *result;

    /* Allocate a memory block with an odd size */
    result = fwk_mm_alloc(ALLOC_NUM, ALLOC_ODD_SIZE);
    assert(result != NULL);
    assert(((uintptr_t)result % FWK_MM_DEFAULT_ALIGNMENT) == 0);

    /*
     * The start address of the last-allocated block should be 64-bit aligned,
     * while the end address of this block should not be (due to the non-even
     * size). This will force an alignment operation on the next call to alloc.
     */
    result = fwk_mm_alloc(ALLOC_NUM, ALLOC_SIZE);
    assert(result != NULL);
    assert(((uintptr_t)result % FWK_MM_DEFAULT_ALIGNMENT) == 0);
    assert((uintptr_t)result + ALLOC_TOTAL_SIZE <=
        (uintptr_t)start + SIZE_MEM);

    /* Write a pattern to the last-allocated block */
    memset(result, MEM_PATTERN, ALLOC_TOTAL_SIZE);

    /* Read back and verify the pattern */
    for (i = 0; i < ALLOC_TOTAL_SIZE; i++)
        assert(result[i] == MEM_PATTERN);
}

static void test_fwk_mm_alloc_aligned(void)
{
    int i;
    char *result;

    /* Allocate a memory block with an odd size */
    result = fwk_mm_alloc_aligned(ALLOC_NUM, ALLOC_ODD_SIZE, ALLOC_ALIGN);
    assert(result != NULL);
    assert(((uintptr_t)result & ALIGN_MASK) == 0);

    /*
     * The start address of the last-allocated block should be 64-bit aligned,
     * while the end address of this block should not be (due to the non-even
     * size). This will force an alignment operation on the next call to alloc.
     */
    result = fwk_mm_alloc_aligned(ALLOC_NUM, ALLOC_SIZE, ALLOC_ALIGN);
    assert(result != NULL);
    assert(((uintptr_t)result & ALIGN_MASK) == 0);
    assert((uintptr_t)result + ALLOC_TOTAL_SIZE <=
           (uintptr_t)start + SIZE_MEM);

    /* Write a pattern to the last-allocated block */
    memset(result, MEM_PATTERN, ALLOC_TOTAL_SIZE);

    /* Read back and verify the pattern */
    for (i = 0; i < ALLOC_TOTAL_SIZE; i++)
        assert(result[i] == MEM_PATTERN);
}

static void test_fwk_mm_calloc(void)
{
    int i;
    char *result;

    /* Allocate a memory block with an odd size */
    result = fwk_mm_calloc(ALLOC_NUM, ALLOC_ODD_SIZE);
    assert(result != NULL);
    assert(((uintptr_t)result % FWK_MM_DEFAULT_ALIGNMENT) == 0);

    /*
     * The start address of the last-allocated block should be 64-bit aligned,
     * while the end address of this block should not be (due to the non-even
     * size). This will force an alignment operation on the next call to alloc.
     */
    result = fwk_mm_calloc(ALLOC_NUM, ALLOC_SIZE);
    assert(result != NULL);
    assert(((uintptr_t)result % FWK_MM_DEFAULT_ALIGNMENT) == 0);
    assert((uintptr_t)result + ALLOC_TOTAL_SIZE <=
           (uintptr_t)start + SIZE_MEM);

     /* Every allocated byte should be initialized to zero */
    for (i = 0; i < ALLOC_TOTAL_SIZE; i++)
        assert(result[i] == 0);

    /* Write a pattern to the last-allocated block */
    memset(result, MEM_PATTERN, ALLOC_TOTAL_SIZE);

    /* Read back and verify the pattern */
    for (i = 0; i < ALLOC_TOTAL_SIZE; i++)
        assert(result[i] == MEM_PATTERN);
}

static void test_fwk_mm_calloc_aligned(void)
{
    int i;
    char *result;

    /* Allocate a memory block with an odd size */
    result = fwk_mm_alloc_aligned(ALLOC_NUM, ALLOC_ODD_SIZE, ALLOC_ALIGN);
    assert(result != NULL);
    assert(((uintptr_t)result & ALIGN_MASK) == 0);

    /*
     * The start address of the last-allocated block should be 64-bit aligned,
     * while the end address of this block should not be (due to the non-even
     * size). This will force an alignment operation on the next call to alloc.
     */
    result = fwk_mm_alloc_aligned(ALLOC_NUM, ALLOC_SIZE, ALLOC_ALIGN);
    assert(result != NULL);
    assert(((uintptr_t)result & ALIGN_MASK) == 0);
    assert((uintptr_t)result + ALLOC_TOTAL_SIZE <=
           (uintptr_t)start + SIZE_MEM);

    /* Every allocated byte should be initialized to zero */
    for (i = 0; i < ALLOC_TOTAL_SIZE; i++)
        assert(result[i] == 0);

    /* Write a pattern to the last-allocated block */
    memset(result, MEM_PATTERN, ALLOC_TOTAL_SIZE);

    /* Read back and verify the pattern */
    for (i = 0; i < ALLOC_TOTAL_SIZE; i++)
        assert(result[i] == MEM_PATTERN);

}
