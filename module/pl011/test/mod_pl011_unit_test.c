/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_module.h>
#include <pl011.h>

#include <mod_pl011.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC
#include "config_pl011.h"

struct fwk_io_stream stream;
struct mod_pl011_element_cfg *cfg_ut;

void setUp(void)
{
    memset(&pl011_ctx, 0, sizeof(pl011_ctx));

    cfg_ut =
        (struct mod_pl011_element_cfg *)config_pl011_ut.elements.table[0].data;

    pl011_ctx.elements = fwk_mm_alloc(1, sizeof(pl011_ctx.elements[0]));
    pl011_ctx.elements[0] = (struct mod_pl011_element_ctx){
        .powered = true, /* Assume the device is always powered */
        .clocked = true, /* Assume the device is always clocked  */
        .open = false,
    };
}

void tearDown(void)
{
}

void test_mod_pl011_putch_true(void)
{
    bool status;
    char ch = 64;
    fwk_id_t id;

    fwk_module_get_data_ExpectAnyArgsAndReturn(cfg_ut);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    status = mod_pl011_putch(id, ch);
    TEST_ASSERT_EQUAL(status, true);
    TEST_ASSERT_EQUAL(mod_reg.DR, ch);
}

void test_mod_pl011_io_putch_success(void)
{
    int status;
    char ch = 64;
    struct fwk_io_stream stream;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    pl011_ctx.elements[0].open = true;

    fwk_module_get_data_ExpectAnyArgsAndReturn(cfg_ut);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    status = mod_pl011_io_putch(&stream, ch);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(mod_reg.DR, ch);
}

void test_mod_pl011_io_putch_fail_powered(void)
{
    int status;
    char ch = 0;
    struct fwk_io_stream stream;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    pl011_ctx.elements[0].open = true;
    pl011_ctx.elements[0].powered = false;

    fwk_module_get_data_ExpectAnyArgsAndReturn(cfg_ut);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    status = mod_pl011_io_putch(&stream, ch);
    TEST_ASSERT_EQUAL(status, FWK_E_PWRSTATE);
}

void test_mod_pl011_io_putch_fail_clocked(void)
{
    int status;
    char ch = 0;
    struct fwk_io_stream stream;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    pl011_ctx.elements[0].open = true;
    pl011_ctx.elements[0].clocked = false;

    fwk_module_get_data_ExpectAnyArgsAndReturn(cfg_ut);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    status = mod_pl011_io_putch(&stream, ch);
    TEST_ASSERT_EQUAL(status, FWK_E_PWRSTATE);
}

void test_mod_pl011_init_ctx(void)
{
    /* Clear module context to ensure it is properly initialized */
    memset(&pl011_ctx, 0, sizeof(pl011_ctx));

    fwk_module_get_element_count_ExpectAnyArgsAndReturn(1);
    fwk_module_get_data_ExpectAnyArgsAndReturn(cfg_ut);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);

    mod_pl011_init_ctx();
    TEST_ASSERT_EQUAL(pl011_ctx.initialized, true);
    TEST_ASSERT_EQUAL(pl011_ctx.elements[0].powered, true);
    TEST_ASSERT_EQUAL(pl011_ctx.elements[0].clocked, true);
    TEST_ASSERT_EQUAL(pl011_ctx.elements[0].open, false);
}

void test_mod_pl011_init_initialised(void)
{
    int status;
    fwk_id_t module_id;

    pl011_ctx.initialized = true;

    status = mod_pl011_init(module_id, 1, NULL);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_mod_pl011_init(void)
{
    int status;
    fwk_id_t module_id;

    /* Clear module context to ensure it is properly initialized */
    memset(&pl011_ctx, 0, sizeof(pl011_ctx));

    pl011_ctx.initialized = false;

    /* Set up the mock calls for the context initialisation */
    fwk_module_get_element_count_ExpectAnyArgsAndReturn(1);
    fwk_module_get_data_ExpectAnyArgsAndReturn(cfg_ut);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);

    status = mod_pl011_init(module_id, 1, NULL);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(pl011_ctx.initialized, true);
    TEST_ASSERT_EQUAL(pl011_ctx.elements[0].powered, true);
    TEST_ASSERT_EQUAL(pl011_ctx.elements[0].clocked, true);
    TEST_ASSERT_EQUAL(pl011_ctx.elements[0].open, false);
}

void test_mod_pl011_io_open_support(void)
{
    int status;
    struct fwk_io_stream stream;

    fwk_id_is_type_ExpectAnyArgsAndReturn(false);

    status = mod_pl011_io_open(&stream);
    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void test_mod_pl011_io_open_busy(void)
{
    int status;
    struct fwk_io_stream stream;

    fwk_id_is_type_ExpectAnyArgsAndReturn(true);

    pl011_ctx.initialized = true;
    pl011_ctx.elements[0].open = true;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    status = mod_pl011_io_open(&stream);
    TEST_ASSERT_EQUAL(status, FWK_E_BUSY);
}

void test_mod_pl011_io_open_success(void)
{
    int status;
    struct fwk_io_stream stream;

    pl011_ctx.initialized = true;
    pl011_ctx.elements[0].clocked = true;
    pl011_ctx.elements[0].powered = true;
    pl011_ctx.elements[0].open = false;

    fwk_id_is_type_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_module_get_data_ExpectAnyArgsAndReturn(cfg_ut);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    status = mod_pl011_io_open(&stream);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(pl011_ctx.elements[0].open, true);
    TEST_ASSERT_EQUAL(mod_reg.ECR, PL011_ECR_CLR);
    TEST_ASSERT_EQUAL(mod_reg.LCR_H, PL011_LCR_H_WLEN_8BITS | PL011_LCR_H_FEN);
    TEST_ASSERT_EQUAL(
        mod_reg.CR, PL011_CR_UARTEN | PL011_CR_RXE | PL011_CR_TXE);
}

void test_mod_pl011_io_getch(void)
{
    bool status;
    char ch = 0;
    fwk_id_t id;

    mod_reg.DR = 64;

    fwk_module_get_data_ExpectAnyArgsAndReturn(cfg_ut);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    status = mod_pl011_getch(id, &ch);
    TEST_ASSERT_EQUAL(status, true);
    TEST_ASSERT_EQUAL(ch, 64);
}

void test_mod_pl011_flush(void)
{
    fwk_id_t id;

    fwk_module_get_data_ExpectAnyArgsAndReturn(cfg_ut);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    /* The PL011_FR_BUSY is zeroed so the flush should take place silently */
    mod_pl011_flush(id);
}

int pl011_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_mod_pl011_putch_true);
    RUN_TEST(test_mod_pl011_io_putch_success);
    RUN_TEST(test_mod_pl011_io_putch_fail_powered);
    RUN_TEST(test_mod_pl011_io_putch_fail_clocked);
    RUN_TEST(test_mod_pl011_io_open_busy);
    RUN_TEST(test_mod_pl011_init_ctx);
    RUN_TEST(test_mod_pl011_init_initialised);
    RUN_TEST(test_mod_pl011_init);
    RUN_TEST(test_mod_pl011_io_open_support);
    RUN_TEST(test_mod_pl011_io_open_success);
    RUN_TEST(test_mod_pl011_io_getch);
    RUN_TEST(test_mod_pl011_flush);
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return pl011_test_main();
}
#endif
