/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "internal/Mockfwk_id_internal.h"
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_module.h>

#include <internal/atu_manage_utils.h>

#include <mod_atu.h>

#include UNIT_TEST_SRC
#include "config_atu.h"

#include <atu_common.c>
#include <atu_manage.c>
#include <atu_manage_utils.c>

struct mod_atu_ctx atu_ctx;

void setUp(void)
{
    /* Do Nothing */
}

void tearDown(void)
{
    /* Do Nothing */
}

/*!
 * \brief atu unit test: mod_atu_init(), empty config data.
 *
 *  \details Handle case in mod_atu_init() where empty config data is passed.
 */
void test_mod_atu_init_fail(void)
{
    int status;

    /* Test init by passing NULL pointer instead of data */
    status = mod_atu_init(FWK_ID_MODULE(FWK_MODULE_IDX_ATU), 0, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

/*!
 * \brief atu unit test: mod_atu_init(), valid config data.
 *
 *  \details Handle case in mod_atu_init() where valid config data is passed.
 */
void test_mod_atu_init_success(void)
{
    int status = 0;

    status = mod_atu_init(
        FWK_ID_MODULE(FWK_MODULE_IDX_ATU), 1, (const void *)config_atu.data);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/*!
 * \brief atu unit test: mod_atu_device_init(), invalid config data.
 *
 *  \details Handle case in mod_atu_device_init() where invalid config data is
 *      passed.
 */
void test_mod_atu_device_init_fail(void)
{
    int status = 0;

    status =
        mod_atu_device_init(FWK_ID_ELEMENT(FWK_MODULE_IDX_ATU, 0), 1, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_DATA);
}

/*!
 * \brief atu unit test: mod_atu_device_init(), valid config data.
 *
 *  \details Handle case in mod_atu_device_init() where valid config data is
 *      passed.
 */
void test_mod_atu_device_init_success(void)
{
    int status = 0;
    struct __fwk_id_fmt mock_value;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    __fwk_id_str_ExpectAnyArgsAndReturn(mock_value);

    status = mod_atu_device_init(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_ATU, 0),
        1,
        (const void *)element_table[0].data);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/*!
 * \brief atu unit test: mod_atu_process_bind_request(), invalid bind.
 *
 *  \details Handle case in mod_atu_process_bind_request() where binding is
 *      requested for an element instead of the module.
 */
void test_mod_atu_process_bind_request_fail(void)
{
    int status;
    const void *api;

    fwk_id_t source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FAKE);

    fwk_id_t api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_ATU, MOD_ATU_API_IDX_ATU);

    fwk_id_t target_element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_ATU, 0);

    /*
     * Bind to element instead of the module.
     * FWK_E_PARAM should be returned by mod_atu_process_bind_request().
     */
    fwk_id_is_type_ExpectAnyArgsAndReturn(false);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_ATU_API_IDX_ATU);

    status = mod_atu_process_bind_request(
        source_id, target_element_id, api_id, &api);
    TEST_ASSERT_EQUAL(status, FWK_E_ACCESS);
}

/*!
 * \brief atu unit test: mod_atu_process_bind_request(), valid bind.
 *
 *  \details Handle case in mod_atu_process_bind_request() where binding is
 *      requested for the module.
 */
void test_mod_atu_process_bind_request_success(void)
{
    int status;
    const void *api;

    fwk_id_t source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FAKE);

    fwk_id_t api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_ATU, MOD_ATU_API_IDX_ATU);

    fwk_id_t target_module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_ATU);

    /*
     * Bind to module with correct API id.
     * FWK_SUCCESS should be returned by mod_atu_process_bind_request().
     */
    fwk_id_is_type_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    status =
        mod_atu_process_bind_request(source_id, target_module_id, api_id, &api);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL_PTR(api, &atu_manage_api);
}

/*!
 * \brief atu unit test: atu_add_region(), valid region.
 *
 *  \details Handle case in atu_add_region() where a valid translation region is
 *      requested to be mapped in the ATU.
 */
void test_atu_add_region_success(void)
{
    int status;
    uint8_t region_idx;
    fwk_id_t atu_device_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_ATU, 0);
    struct atu_device_ctx *device_ctx;
    struct __fwk_id_fmt mock_value;

    struct atu_region_map region = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_ATU),
        .log_addr_base = 0xA0000000,
        .phy_addr_base = 0x100000000,
        .region_size = (1 * FWK_GIB),
        .attributes = ATU_ENCODE_ATTRIBUTES_ROOT_PAS,
    };

    device_ctx = &atu_ctx.device_ctx_table[0];

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    __fwk_id_str_ExpectAnyArgsAndReturn(mock_value);

    /* Map the address translation region */
    status = atu_add_region(&region, atu_device_id, &region_idx);

    /*
     * Ensure that the ATU region has been enabled and the details of the region
     * have been stored in the context data.
     */
    TEST_ASSERT_EQUAL(region_idx, 1);
    TEST_ASSERT_EQUAL(((device_ctx->atu->ATUC >> region_idx) & 0x1), 1);
    TEST_ASSERT_EQUAL(
        device_ctx->active_regions[region_idx].log_addr_base,
        region.log_addr_base);
    TEST_ASSERT_EQUAL(
        device_ctx->active_regions[region_idx].phy_addr_base,
        region.phy_addr_base);
    TEST_ASSERT_EQUAL(
        device_ctx->active_regions[region_idx].region_size, region.region_size);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/*!
 * \brief atu unit test: atu_add_region(), invalid region.
 *
 *  \details Handle case in atu_add_region() where a partially overlapping
 *      translation region is requested to be mapped in the ATU.
 */
void test_atu_add_region_overlap(void)
{
    int status;
    uint8_t region_idx;
    fwk_id_t atu_device_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_ATU, 0);

    /* A region that partially overlaps an existing ATU region */
    struct atu_region_map region = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_ATU),
        .log_addr_base = 0x60000000,
        .phy_addr_base = 0x80000000,
        .region_size = 0x40000000,
        .attributes = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
    };

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    /* Map the address translation region */
    status = atu_add_region(&region, atu_device_id, &region_idx);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

/*!
 * \brief atu unit test: atu_add_region(), invalid attributes config.
 *
 *  \details Handle case in atu_add_region() where an invalid config data is
 *      passed.
 */
void test_atu_add_region_invalid_attributes_config(void)
{
    int status;
    uint8_t region_idx;
    fwk_id_t atu_device_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_ATU, 0);

    struct atu_region_map region = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_ATU),
        .log_addr_base = 0x70000000,
        .phy_addr_base = 0x80000000,
        .region_size = 0x10000000,
        /* Invalid attributes */
        .attributes = 0,
    };

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    /* Map the address translation region */
    status = atu_add_region(&region, atu_device_id, &region_idx);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

/*!
 * \brief atu unit test: atu_remove_region(), invalid region.
 *
 *  \details Handle case in atu_remove_region() where an invalid translation
 *       region is requested to be removed.
 */
void test_atu_remove_region_fail(void)
{
    int status;
    fwk_id_t atu_device_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_ATU, 0);
    fwk_id_t region_owner_id = FWK_ID_NONE_INIT;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    status = atu_remove_region(35, atu_device_id, region_owner_id);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

/*!
 * \brief atu unit test: atu_remove_region(), invalid permission.
 *
 *  \details Handle case in atu_remove_region() where an entity that does not
 *       own the region tries to remove an ATU region.
 */
void test_atu_remove_region_permission_fail(void)
{
    int status;
    fwk_id_t atu_device_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_ATU, 0);
    fwk_id_t region_owner_id = FWK_ID_NONE_INIT;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_module_is_valid_entity_id_ExpectAnyArgsAndReturn(true);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(false);

    status = atu_remove_region(0, atu_device_id, region_owner_id);
    TEST_ASSERT_EQUAL(status, FWK_E_ACCESS);
}

/*!
 * \brief atu unit test: atu_remove_region(), valid region.
 *
 *  \details Handle case in atu_remove_region() where a valid translation
 *       region is requested to be removed.
 */
void test_atu_remove_region_success(void)
{
    int status;
    fwk_id_t atu_device_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_ATU, 0);
    fwk_id_t region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_ATU);
    struct atu_device_ctx *device_ctx;
    uint8_t region_idx = 0;
    struct __fwk_id_fmt mock_value;

    device_ctx = &atu_ctx.device_ctx_table[0];

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_module_is_valid_entity_id_ExpectAnyArgsAndReturn(true);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);
    __fwk_id_str_ExpectAnyArgsAndReturn(mock_value);

    status = atu_remove_region(region_idx, atu_device_id, region_owner_id);
    /*
     * Ensure that the ATU region has been disabled and the context data
     * has been cleared.
     */
    TEST_ASSERT_EQUAL(((device_ctx->atu->ATUC >> region_idx) & 0x1), 0);
    TEST_ASSERT_EQUAL(device_ctx->active_regions[region_idx].log_addr_base, 0);
    TEST_ASSERT_EQUAL(device_ctx->active_regions[region_idx].phy_addr_base, 0);
    TEST_ASSERT_EQUAL(device_ctx->active_regions[region_idx].region_size, 0);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/*!
 * \brief atu unit test: atu_get_available_region_idx(), invalid parameter.
 *
 *  \details Handle case in atu_get_available_region_idx() where an invalid
 *       parameter is passed.
 */
void test_atu_get_available_region_idx_fail(void)
{
    int status;
    struct atu_device_ctx *device_ctx;

    device_ctx = &atu_ctx.device_ctx_table[0];

    status = atu_get_available_region_idx((void *)device_ctx, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

int atu_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_mod_atu_init_fail);
    RUN_TEST(test_mod_atu_init_success);
    RUN_TEST(test_mod_atu_device_init_fail);
    RUN_TEST(test_mod_atu_device_init_success);
    RUN_TEST(test_mod_atu_process_bind_request_fail);
    RUN_TEST(test_mod_atu_process_bind_request_success);
    RUN_TEST(test_atu_add_region_success);
    RUN_TEST(test_atu_add_region_overlap);
    RUN_TEST(test_atu_add_region_invalid_attributes_config);
    RUN_TEST(test_atu_remove_region_fail);
    RUN_TEST(test_atu_remove_region_permission_fail);
    RUN_TEST(test_atu_remove_region_success);
    RUN_TEST(test_atu_get_available_region_idx_fail);

    return UNITY_END();
}

int main(void)
{
    return atu_test_main();
}
